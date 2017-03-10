// Project
#include "Scene.h"

Scene::Scene(Window *_window) : AbstractScene(_window)
{
}

Scene::~Scene()
{
  delete m_program;
  delete m_vao;
}

void Scene::generateQuadData() {
  m_quadData = {
    // Positions     // Colors
    -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
    -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

    -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
     0.05f,  0.05f,  0.0f, 1.0f, 1.0f
  };
}

void Scene::generateInstancesData() {
  m_instancesData = {
    // Position       // Radius
    -0.9f, -0.9f,     1.0f,
    -0.6f, -0.6f,     0.5f,
    -0.3f, -0.3f,     0.2f,
     0.0f,  0.0f,     0.2f,
     0.3f,  0.3f,     0.2f,
     0.6f,  0.6f,     0.5f,
     0.9f,  0.9f,     1.0f
  };
}

void Scene::initialize()
{
  AbstractScene::initialize();
  m_program = new QOpenGLShaderProgram();
  m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/particles.vert");
  m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/particles.frag");
  m_program->link();

  generateQuadData();
  generateInstancesData();

  // Create VAO
  m_vao = new QOpenGLVertexArrayObject();
  m_vao->create();

  // Create Instance VBO (hols positions of each instance)
  m_quad_vbo.create();
  m_quad_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

  // Create Quad VBO (this quad will be instanced various times)
  m_instance_vbo.create();
  m_instance_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

  // Connect VAO, buffer data and set attribute pointers
  m_vao->bind();

  // Quad Data =================================================================
  m_quad_vbo.bind();
  m_quad_vbo.allocate(&m_quadData[0], (2 + 3) * 6 * sizeof(GLfloat));
  m_program->enableAttributeArray("position");
  m_program->setAttributeBuffer("position", GL_FLOAT, 0, 2, 5 * sizeof(GLfloat));
  m_program->enableAttributeArray("color");
  m_program->setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(GLfloat), 3, 5 * sizeof(GLfloat));

  // Buffering Instances= Data =================================================
  m_instance_vbo.bind();
  m_instance_vbo.allocate(&m_instancesData[0], (2 + 1) * 7 * sizeof(GLfloat));

  m_program->enableAttributeArray("instance");
  m_program->setAttributeBuffer("instance", GL_FLOAT, 0, 3);

  m_instance_vbo.release();
  glVertexAttribDivisor(m_program->attributeLocation("instance"), 1);
  m_vao->release();

  glViewport(0, 0, window()->width(), window()->height());
  glClearColor(0.0f,0.0f, 0.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Scene::paint()
{
  glClear(GL_COLOR_BUFFER_BIT);
  m_program->bind();
  m_vao->bind();
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 7);
  m_vao->release();
  m_program->release();

}
