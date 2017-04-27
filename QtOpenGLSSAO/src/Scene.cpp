////////////////////////////////////////////////////////////////////////////////
/// @file Scene.cpp
/// @author Ramon Blanquer
/// @version 0.0.1
////////////////////////////////////////////////////////////////////////////////

// Third Party
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Qt
#include <QKeyEvent>
#include <QVector3D>
#include <QOpenGLTexture>

// Standard
#include <random>

// Project
#include "Scene.h"

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

Scene::Scene(Window *_window) : AbstractScene(_window)
{
}

Scene::~Scene()
{
  delete m_quad_program;
  delete m_geom_program;
  delete m_gbuffer_fbo;
}

void Scene::initialize()
{
  AbstractScene::initialize();

  //////////////////////////////////////////////////////////////////////////////
  // Import mesh data //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile("../assets/models/spheres.obj", 0);

  for (size_t i = 0; i < scene->mNumMeshes; i++)
  {
    // Populate vertices
    for (size_t j = 0; j < scene->mMeshes[i]->mNumVertices; j++)
    {
      m_geom_vertices.push_back(scene->mMeshes[i]->mVertices[j].x);
      m_geom_vertices.push_back(scene->mMeshes[i]->mVertices[j].y);
      m_geom_vertices.push_back(scene->mMeshes[i]->mVertices[j].z);
      m_geom_vertices.push_back(scene->mMeshes[i]->mNormals[j].x);
      m_geom_vertices.push_back(scene->mMeshes[i]->mNormals[j].y);
      m_geom_vertices.push_back(scene->mMeshes[i]->mNormals[j].z);
    }

    // Populate indices
    for (size_t j = 0; j < scene->mMeshes[i]->mNumFaces; j++)
    {
      m_geom_indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[0]);
      m_geom_indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[1]);
      m_geom_indices.push_back(scene->mMeshes[i]->mFaces[j].mIndices[2]);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Matrix initialization//////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_M.setToIdentity();
  m_V.setToIdentity();
  m_P.setToIdentity();
  m_P.perspective(45.0f, 1.0f, 0.01f, 30.0f);
  m_V.translate(0,0,-5);

  //////////////////////////////////////////////////////////////////////////////
  // Shader compilation and linking ////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_quad_program = new QOpenGLShaderProgram();
  m_quad_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/quad.vert");
  m_quad_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/quad.frag");
  m_quad_program->link();

  m_geom_program = new QOpenGLShaderProgram();
  m_geom_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/geom.vert");
  m_geom_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/geom.frag");
  m_geom_program->link();

  //////////////////////////////////////////////////////////////////////////////
  // Quad preparation //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  static const float quad[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f
  };
  m_quad_vao.create();
  m_quad_vbo.create();
  m_quad_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
  m_quad_program->bind();
  m_quad_vao.bind();
    m_quad_vbo.bind();
    m_quad_program->enableAttributeArray("position");
    m_quad_program->enableAttributeArray("uv");
    m_quad_vbo.allocate(quad, (3 + 2) * 6 * sizeof(GLfloat));
    m_quad_program->setAttributeBuffer("position", GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_quad_program->setAttributeBuffer("uv", GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
    m_quad_program->setUniformValue("tPosition", 0);
    m_quad_program->setUniformValue("tNormal"  , 1);
    m_quad_program->setUniformValue("pass", 0);
  m_quad_vao.release();
  m_quad_program->release();

  //////////////////////////////////////////////////////////////////////////////
  // Geometry preparation //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_geom_ebo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
  m_geom_ebo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_geom_vao.create();
  m_geom_ebo.create();
  m_geom_vbo.create();
  m_geom_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_geom_program->bind();
  m_geom_vao.bind();
    m_geom_vbo.bind();
    m_geom_vbo.allocate(&m_geom_vertices[0], (int)m_geom_vertices.size() * sizeof(GLfloat));
    m_geom_ebo.bind();
    m_geom_ebo.allocate(&m_geom_indices[0], (int)m_geom_indices.size() * sizeof(GLuint));
    m_geom_program->setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    m_geom_program->enableAttributeArray("position");
    m_geom_program->setAttributeBuffer("normal", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));
    m_geom_program->enableAttributeArray("normal");
    m_geom_program->setUniformValue("M", m_M);
    m_geom_program->setUniformValue("V", m_V);
    m_geom_program->setUniformValue("P", m_P);
  m_geom_vao.release();
  m_geom_program->release();

  //////////////////////////////////////////////////////////////////////////////
  // gBuffer FBO preparation ///////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_gbuffer_fbo = new QOpenGLFramebufferObject(720, 720);
  m_gbuffer_fbo->addColorAttachment(720, 720, GL_RGB);

  m_gbuffer_fbo->bind();

  const GLenum attachments[] = {
   GL_COLOR_ATTACHMENT0,
   GL_COLOR_ATTACHMENT1,
  };

  // Drawing multiple buffers.
  glDrawBuffers(2, attachments);

  // Create and attach depth buffer (renderbuffer) ===========================
  GLuint rbo_depth;

  // Generate renderbuffer object names
  glGenRenderbuffers(1, &rbo_depth);

  // Bind a named renderbuffer object
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);

  // Establish data storage, format and dimensions of a renderbuffer object's image
  glRenderbufferStorage(
       GL_RENDERBUFFER,     // Target
       GL_DEPTH_COMPONENT,  // Internal Format
       720, 720);  // Size

  // Attach a renderbuffer object to a framebuffer object
  glFramebufferRenderbuffer(
       GL_FRAMEBUFFER,       // Target
       GL_DEPTH_ATTACHMENT,  // Attachment
       GL_RENDERBUFFER,      // Renderbuffer Target
       rbo_depth);           // Renderbuffer

  // Finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   qCritical("Framebuffer not complete!");

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glViewport(0, 0, 720, 720);
  m_gbuffer_fbo->release();

  //////////////////////////////////////////////////////////////////////////////
  // SSAO kernel preparation ///////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
  std::default_random_engine generator;
  std::vector<QVector3D> ssaoKernel;
  for (size_t i = 0; i < 64; ++i)
  {
    QVector3D sample(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator)
    );
    sample.normalize();
    sample *= randomFloats(generator);
    float scale = (float)i/64.0;
    scale = lerp(0.1f, 1.0f, scale*scale);
    sample *= scale;
    ssaoKernel.push_back(sample);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Noise texture preparation /////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  std::vector<QVector3D> ssaoNoise;
  for (size_t i = 0; i < 16; i++)
  {
    QVector3D noise(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      0.0f
    );
    ssaoNoise.push_back(noise);
  }

  QOpenGLTexture* noiseTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  noiseTexture->setMinificationFilter(QOpenGLTexture::Nearest);
  noiseTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
  noiseTexture->setWrapMode(QOpenGLTexture::Repeat);
  noiseTexture->setSize(4,4);
  noiseTexture->setFormat(QOpenGLTexture::RGB16F);
  noiseTexture->allocateStorage();
  noiseTexture->setData(0, QOpenGLTexture::RGB, QOpenGLTexture::Float16, &ssaoNoise[0]);
}

void Scene::paint()
{
  m_gbuffer_fbo->bind();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_geom_program->bind();
    m_geom_vao.bind();
      glDrawElements(GL_TRIANGLES, (int)m_geom_indices.size(), GL_UNSIGNED_INT, 0);
    m_geom_vao.release();
    m_geom_program->release();

  m_gbuffer_fbo->release();
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    GLuint textureID = m_gbuffer_fbo->texture();
    glActiveTexture(GL_TEXTURE0);  // Position (RGB)
    glBindTexture(GL_TEXTURE_2D, textureID); textureID += 1;
    glActiveTexture(GL_TEXTURE1);  // Normal (RGB)
    glBindTexture(GL_TEXTURE_2D, textureID); textureID += 1;

    m_quad_program->bind();
    m_quad_vao.bind();
      glDrawArrays(GL_TRIANGLES, 0, 6);
    m_quad_vao.release();
    m_quad_program->release();

    if (m_keepSpinning)
    {
      m_geom_program->bind();
      m_M.rotate(0.2, 0.0, 1.0, 0.0);
      m_geom_program->setUniformValue("M", m_M);
      m_geom_program->release();
    }

}

void Scene::keyPressEvent(QKeyEvent *ev)
{

  switch(ev->key())
  {
    case Qt::Key_Left:
      m_V.translate(0.1f, 0.0f, 0.0f);
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_Right:
      m_V.translate(-0.1f, 0.0f, 0.0f);
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_Up:
      m_V.translate(0.0f, 0.0f, 0.1f);
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_Down:
      m_V.translate(0.0f, 0.0f, -0.1f);
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_W:
      m_V.translate(0.0f, -0.1f, 0.0f);
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_S:
      m_V.translate(0.0f, 0.1f, 0.0f);
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_0:
      m_quad_program->bind();
      m_quad_program->setUniformValue("pass", 0);
      m_quad_program->release();
      break;
    case Qt::Key_1:
      m_quad_program->bind();
      m_quad_program->setUniformValue("pass", 1);
      m_quad_program->release();
      break;
    case Qt::Key_2:
      m_quad_program->bind();
      m_quad_program->setUniformValue("pass", 2);
      m_quad_program->release();
      break;
    case Qt::Key_R:
      m_keepSpinning = !m_keepSpinning;
    default:
      break;
  }


}
