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

// Standard
#include <random>

// Project
#include "Scene.h"

const float DELTA = 0.5f;

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

Scene::Scene(Window *_window)
    : AbstractScene(_window)
  , m_eye(QVector3D(0,0,5))
  , m_center(QVector3D(0,0,0))
  , m_keepSpinning(true)
{
  m_ssao_radius_range = std::make_pair(0.01f, 0.9f);
  m_ssao_bias_range = std::make_pair(0.000001f, 0.009f);
  m_ssao_kernelSize = 64;
}

Scene::~Scene()
{
  delete m_geom_program;
  delete m_gbuffer_fbo;
  delete m_ssao_program;
  delete m_lighting_program;
}

void Scene::initialize()
{
  AbstractScene::initialize();

  //////////////////////////////////////////////////////////////////////////////
  // Import mesh data //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile("../assets/models/cells.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

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
  m_P.perspective(45.0f, 1.0f, 0.1f, 50.0f);
  m_M.scale(0.2);
  m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));

  //////////////////////////////////////////////////////////////////////////////
  // Shader compilation and linking ////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_geom_program = new QOpenGLShaderProgram();
  m_geom_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/geom.vert");
  m_geom_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/geom.frag");
  m_geom_program->link();

  m_ssao_program = new QOpenGLShaderProgram();
  m_ssao_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/ssao.vert");
  m_ssao_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/ssao.frag");
  m_ssao_program->link();

  m_blur_program = new QOpenGLShaderProgram();
  m_blur_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/ssao.vert");
  m_blur_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/blur.frag");
  m_blur_program->link();

  m_lighting_program = new QOpenGLShaderProgram();
  m_lighting_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/ssao.vert");
  m_lighting_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/lighting.frag");
  m_lighting_program->link();

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
  m_quad_vao = new QOpenGLVertexArrayObject(window());
  m_quad_vao->create();
  m_quad_vbo.create();
  m_quad_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_lighting_program->bind();
  m_quad_vao->bind();
    m_quad_vbo.bind();
    m_quad_vbo.allocate(quad, (3 + 2) * 6 * sizeof(GLfloat));
    m_lighting_program->setAttributeBuffer("position", GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    m_lighting_program->setAttributeBuffer("uv", GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
    m_lighting_program->enableAttributeArray("position");
    m_lighting_program->enableAttributeArray("uv");
    m_lighting_program->setUniformValue("pass", 0);
  m_quad_vao->release();
  m_lighting_program->release();

  //////////////////////////////////////////////////////////////////////////////
  // Geometry preparation //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_geom_vao = new QOpenGLVertexArrayObject(window());
  m_geom_ebo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
  m_geom_ebo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_geom_vao->create();
  m_geom_ebo.create();
  m_geom_vbo.create();
  m_geom_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  m_geom_program->bind();
  m_geom_vao->bind();
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
  m_geom_vao->release();
  m_geom_program->release();

  //////////////////////////////////////////////////////////////////////////////
  // Framebuffer textures initialization ///////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_position_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  m_position_texture->setSize(720, 720);
  m_position_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  m_position_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
  m_position_texture->setFormat(QOpenGLTexture::RGB16_UNorm);
  m_position_texture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::Float32);

  m_normal_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  m_normal_texture->setSize(720, 720);
  m_normal_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  m_normal_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
  m_normal_texture->setFormat(QOpenGLTexture::RGB16_UNorm);
  m_normal_texture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::Float32);

  m_occlusion_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  m_occlusion_texture->setSize(720, 720);
  m_occlusion_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  m_occlusion_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
  m_occlusion_texture->setFormat(QOpenGLTexture::RGB8_UNorm);
  m_occlusion_texture->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::Float32);

  m_blurred_occlusion_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  m_blurred_occlusion_texture->setSize(720, 720);
  m_blurred_occlusion_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  m_blurred_occlusion_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
  m_blurred_occlusion_texture->setFormat(QOpenGLTexture::RGB8_UNorm);
  m_blurred_occlusion_texture->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::Float32);


  //////////////////////////////////////////////////////////////////////////////
  // gBuffer FBO preparation ///////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_gbuffer_fbo = new QOpenGLFramebufferObject(720, 720);  // GL_COLOR_ATTACHMENT0
  m_gbuffer_fbo->bind();
  m_gbuffer_fbo->addColorAttachment(720, 720, GL_RGB);     // GL_COLOR_ATTACHMENT1

  qDebug("Position texture ID: %d", m_position_texture->textureId());
  qDebug("Normal texture ID: %d", m_normal_texture->textureId());
  glBindTexture(GL_TEXTURE_2D, m_position_texture->textureId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_position_texture->textureId(), 0);
  glBindTexture(GL_TEXTURE_2D, m_normal_texture->textureId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_normal_texture->textureId(), 0);

  const GLenum gbuffer_attachments[2] = {
   GL_COLOR_ATTACHMENT0,
   GL_COLOR_ATTACHMENT1,
  };

  // Drawing multiple buffers.
  glDrawBuffers(2, gbuffer_attachments);

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
   qCritical("gBuffer Framebuffer not complete!");
  m_gbuffer_fbo->release();

  //////////////////////////////////////////////////////////////////////////////
  // SSAO FBO preparation //////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_ssao_fbo = new QOpenGLFramebufferObject(720, 720);  // GL_COLOR_ATTACHMENT0
  m_ssao_fbo->bind();

  qDebug("Occlusion texture ID: %d", m_occlusion_texture->textureId());
  glBindTexture(GL_TEXTURE_2D, m_occlusion_texture->textureId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_occlusion_texture->textureId(), 0);

  const GLenum ssao_attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, ssao_attachments);

  // Finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   qCritical("SSAO Framebuffer not complete!");
  m_ssao_fbo->release();

  //////////////////////////////////////////////////////////////////////////////
  // Blur FBO preparation //////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  m_blur_fbo = new QOpenGLFramebufferObject(720, 720);
  m_blur_fbo->bind();

  qDebug("Blurred occlusion texture ID: %d", m_blurred_occlusion_texture->textureId());
  glBindTexture(GL_TEXTURE_2D, m_blurred_occlusion_texture->textureId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_blurred_occlusion_texture->textureId(), 0);

  const GLenum blur_attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, blur_attachments);

  // Finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
   qCritical("Blur Framebuffer not complete!");
  m_blur_fbo->release();

  //////////////////////////////////////////////////////////////////////////////
  // SSAO kernel preparation ///////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
  std::default_random_engine generator;
  m_ssao_kernel.clear();
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
    m_ssao_kernel.push_back(sample);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Noise texture generation //////////////////////////////////////////////////
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

  m_noiseTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  m_noiseTexture->setMinificationFilter(QOpenGLTexture::Nearest);
  m_noiseTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
  m_noiseTexture->setWrapMode(QOpenGLTexture::Repeat);
  m_noiseTexture->setSize(4,4);
  m_noiseTexture->setFormat(QOpenGLTexture::RGB16F);
  m_noiseTexture->allocateStorage();
  m_noiseTexture->setData(0, QOpenGLTexture::RGB, QOpenGLTexture::Float16, &ssaoNoise[0]);

  //////////////////////////////////////////////////////////////////////////////
  // Shader configuration /// //////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  m_ssao_program->bind();
  m_ssao_program->setUniformValue("tPosition", 0);
  m_ssao_program->setUniformValue("tNormal"  , 1);
  m_ssao_program->setUniformValue("tTexNoise"  , 2);
  m_ssao_program->release();
  m_blur_program->bind();
  m_blur_program->setUniformValue("tInputSSAO", 0);
  m_blur_program->release();
  m_lighting_program->bind();
  m_lighting_program->setUniformValue("tPosition", 0);
  m_lighting_program->setUniformValue("tNormal"  , 1);
  m_lighting_program->setUniformValue("tSSAO"  , 2);
  m_lighting_program->release();
  setSSAORadius(50);
  setSSAOBias(0);
  setSSAOKernelSize(64);
  setSSAOBlurAmount(2);

  //////////////////////////////////////////////////////////////////////////////
  // OpenGL Initialization /////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  glViewport(0, 0, 720, 720);
}

void Scene::paint()
{
  // ===========================================================================
  // Render scene geometry to gBuffer
  // ===========================================================================
  m_gbuffer_fbo->bind();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_geom_program->bind();
    m_geom_vao->bind();
      glDrawElements(GL_TRIANGLES, (int)m_geom_indices.size(), GL_UNSIGNED_INT, 0);
    m_geom_vao->release();
    m_geom_program->release();
  m_gbuffer_fbo->release();

  // ===========================================================================
  // Generate SSAO texture
  // ===========================================================================
  m_ssao_fbo->bind();
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    m_ssao_program->bind();
    for (uint i = 0; i < m_ssao_kernelSize; ++i)
    {
      char buffer [12];
      sprintf(buffer, "samples[%d]", i);
      m_ssao_program->setUniformValue(buffer, m_ssao_kernel[i]);
    }
    m_ssao_program->setUniformValue("P", m_P);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_position_texture->textureId());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normal_texture->textureId());
    m_quad_vao->bind();
      glDrawArrays(GL_TRIANGLES, 0, 6);
    m_quad_vao->release();
    m_ssao_program->release();
  m_ssao_fbo->release();

  // ===========================================================================
  // Blur SSAO texture to remove noise
  // ===========================================================================
  m_blur_fbo->bind();
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    m_blur_program->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_occlusion_texture->textureId());
    m_quad_vao->bind();
      glDrawArrays(GL_TRIANGLES, 0, 6);
    m_quad_vao->release();
    m_blur_program->release();
  m_blur_fbo->release();

  // ===========================================================================
  // Lighting pass
  // ===========================================================================
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  m_lighting_program->bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_position_texture->textureId());
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_normal_texture->textureId());
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_blurred_occlusion_texture->textureId());
  m_quad_vao->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
  m_quad_vao->release();
  m_lighting_program->release();

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
      m_eye.setX(m_eye.x() - DELTA);
      m_V.setToIdentity();
      m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_Right:
      m_eye.setX(m_eye.x() + DELTA);
      m_V.setToIdentity();
      m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_Up:
      m_eye.setZ(m_eye.z() - DELTA);
      m_V.setToIdentity();
      m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_Down:
      m_eye.setZ(m_eye.z() + DELTA);
      m_V.setToIdentity();
      m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_W:
    m_eye.setY(m_eye.y() + DELTA);
    m_V.setToIdentity();
    m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_S:
    m_eye.setY(m_eye.y() - DELTA);
    m_V.setToIdentity();
    m_V.lookAt(m_eye, m_center, QVector3D(0,1,0));
      m_geom_program->bind();
      m_geom_program->setUniformValue("V", m_V);
      m_geom_program->release();
      break;
    case Qt::Key_0:
      m_lighting_program->bind();
      m_lighting_program->setUniformValue("pass", 0);
      m_lighting_program->release();
      break;
    case Qt::Key_1:
      m_lighting_program->bind();
      m_lighting_program->setUniformValue("pass", 1);
      m_lighting_program->release();
      break;
    case Qt::Key_2:
      m_lighting_program->bind();
      m_lighting_program->setUniformValue("pass", 2);
      m_lighting_program->release();
      break;
    case Qt::Key_R:
      m_keepSpinning = !m_keepSpinning;
    default:
      break;
  }
}

void Scene::setSSAORadius(int _value)
{
  m_ssao_radius = lerp(m_ssao_radius_range.first, m_ssao_radius_range.second, (float)_value/(float)100);
  qDebug("Radius: %f", m_ssao_radius);
  m_ssao_program->bind();
  m_ssao_program->setUniformValue("radius", m_ssao_radius);
  m_ssao_program->release();
}

void Scene::setSSAOBias(int _value)
{
  m_ssao_bias = lerp(m_ssao_bias_range.first, m_ssao_bias_range.second, (float)_value/(float)100);
  qDebug("Bias: %f", m_ssao_bias);
  m_ssao_program->bind();
  m_ssao_program->setUniformValue("bias", m_ssao_bias);
  m_ssao_program->release();

}

void Scene::setSSAOKernelSize(int _value)
{
  qDebug("Kernel Size: %d", _value);
  m_ssao_kernelSize = _value;

  //////////////////////////////////////////////////////////////////////////////
  // SSAO kernel preparation ///////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
  std::default_random_engine generator;
  m_ssao_kernel.clear();
  for (size_t i = 0; i < m_ssao_kernelSize; ++i)
  {
    QVector3D sample(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator)
    );
    sample.normalize();
    sample *= randomFloats(generator);
    float scale = (float)i/float(m_ssao_kernelSize);
    scale = lerp(0.1f, 1.0f, scale*scale);
    sample *= scale;
    m_ssao_kernel.push_back(sample);
  }

  m_ssao_program->bind();
  m_ssao_program->setUniformValue("kernelSize", _value);
  m_ssao_program->release();
}

void Scene::setSSAOBlurAmount(int _value)
{
  m_blur_program->bind();
  m_blur_program->setUniformValue("blurAmount", _value);
  m_blur_program->release();
}
