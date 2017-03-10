#ifndef SCENE_H
#define SCENE_H

// Qt
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

// Project
#include "AbstractScene.h"
#include "Window.h"

class Scene : public AbstractScene
{
public:
  Scene(Window *_window);
  ~Scene();
  void initialize();
  void paint();
  void generateQuadData();
  void generateInstancesData();

private:
  std::vector<GLfloat> m_sphereData;
  std::vector<GLfloat> m_instancesData;

  QOpenGLBuffer m_quad_vbo;
  QOpenGLBuffer m_instance_vbo;
  QOpenGLVertexArrayObject* m_vao;
  std::vector<GLfloat>  m_shape;
  QOpenGLShaderProgram* m_program;
};

#endif // SCENE_H
