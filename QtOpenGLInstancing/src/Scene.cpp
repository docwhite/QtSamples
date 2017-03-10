// Project
#include "Scene.h"
#include <cmath>

// Recursion subdivision algorithm from http://www.opengl.org.ru/docs/pg/0208.html
#define X .525731112119133606
#define Z .850650808352039932

void pushTriangle(float *v1, float *v2, float *v3, std::vector<GLfloat>& _data) {
  _data.push_back(v1[0]); _data.push_back(v1[1]); _data.push_back(v1[2]); // v1
  _data.push_back(v2[0]); _data.push_back(v2[1]); _data.push_back(v2[2]); // v2
  _data.push_back(v3[0]); _data.push_back(v3[1]); _data.push_back(v3[2]); //v3
}

void subdivide(float *v1, float *v2, float *v3, long depth, std::vector<GLfloat>& _data) {
  GLfloat v12[3], v23[3], v31[3]; // midpoints

  if (depth == 0) {
     pushTriangle(v1, v2, v3, _data);
     return;
  }

  for (size_t i = 0; i < 3; i++) {
    v12[i] = v1[i] + v2[i];
    v23[i] = v2[i] + v3[i];
    v31[i] = v3[i] + v1[i];
  }

  float v12_len = std::sqrt(v12[0]*v12[0] + v12[1] * v12[1] + v12[2] * v12[2]);
  float v23_len = std::sqrt(v23[0]*v23[0] + v23[1] * v23[1] + v23[2] * v23[2]);
  float v31_len = std::sqrt(v31[0]*v31[0] + v31[1] * v31[1] + v31[2] * v31[2]);

  // Normalize (push so match sphere)
  for (size_t i = 0; i < 3; i++)
  {
    v12[i] /= v12_len;
    v23[i] /= v23_len;
    v31[i] /= v31_len;
  }

  subdivide(v1, v12, v31,  depth-1, _data);
  subdivide(v2, v23, v12,  depth-1, _data);
  subdivide(v3, v31, v23,  depth-1, _data);
  subdivide(v12, v23, v31, depth-1, _data);
}

Scene::Scene(Window *_window) : AbstractScene(_window)
{
}

Scene::~Scene()
{
  delete m_program;
  delete m_vao;
}

void Scene::generateQuadData() {

  m_sphereData.clear();

  GLfloat vdata[12][3] = {
     {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
     {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
     {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
  };

  GLuint tindices[20][3] = {
     {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
     {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
     {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
     {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

  for (size_t i = 0; i < 20; i++) {
    subdivide(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], 2, m_sphereData);
  }
}

void Scene::generateInstancesData() {
  m_instancesData = {
    // Position             // Radius
    -0.9f, -0.9f, 0.0f,     0.1f,
    -0.6f, -0.6f, 0.0f,     0.2f,
    -0.3f, -0.3f, 0.0f,     0.3f,
     0.0f,  0.0f, 0.0f,     0.2f,
     0.3f,  0.3f, 0.0f,     0.1f,
     0.6f,  0.6f, 0.0f,     0.2f,
     0.9f,  0.9f, 0.0f,     0.1f
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
  m_quad_vbo.allocate(&m_sphereData[0], m_sphereData.size() * sizeof(GLfloat));
  m_program->enableAttributeArray("position");
  m_program->setAttributeBuffer("position", GL_FLOAT, 0, 3);

  // Buffering Instances= Data =================================================
  m_instance_vbo.bind();
  m_instance_vbo.allocate(&m_instancesData[0], (3 + 1) * 7 * sizeof(GLfloat));

  m_program->enableAttributeArray("instance");
  m_program->setAttributeBuffer("instance", GL_FLOAT, 0, 4);

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

  unsigned int pointsToDraw = m_sphereData.size() / 3.0;

  glDrawArraysInstanced(GL_TRIANGLES, 0, pointsToDraw, 7);
  m_vao->release();
  m_program->release();

}
