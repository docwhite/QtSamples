////////////////////////////////////////////////////////////////////////////////
/// @file Scene.h
/// @author Ramon Blanquer
/// @version 0.0.1
////////////////////////////////////////////////////////////////////////////////

#ifndef SCENE_H
#define SCENE_H

// Qt
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>

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

private:
  float m_keepSpinning;

  QOpenGLFramebufferObject* m_gbuffer_fbo;
  QOpenGLFramebufferObject* m_ssao_fbo;
  QOpenGLFramebufferObject* m_blur_fbo;

  QMatrix4x4 m_M;
  QMatrix4x4 m_V;
  QMatrix4x4 m_P;

  QOpenGLShaderProgram* m_quad_program;
  QOpenGLVertexArrayObject m_quad_vao;
  QOpenGLBuffer m_quad_vbo;

  QOpenGLShaderProgram* m_geom_program;
  QOpenGLVertexArrayObject m_geom_vao;
  QOpenGLBuffer m_geom_vbo;
  QOpenGLBuffer m_geom_ebo;
  std::vector<GLuint> m_geom_indices;
  std::vector<GLfloat> m_geom_vertices;

public slots:
  void keyPressEvent(QKeyEvent* ev) override;
};

#endif // SCENE_H
