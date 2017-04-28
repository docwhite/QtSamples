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
#include <QOpenGLTexture>

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
  QOpenGLFramebufferObject* m_gbuffer_fbo;
  QOpenGLFramebufferObject* m_ssao_fbo;
  QOpenGLFramebufferObject* m_blur_fbo;
  GLuint m_blur_fbo_id;

  QOpenGLTexture* m_position_texture;
  QOpenGLTexture* m_normal_texture;
  QOpenGLTexture* m_occlusion_texture;
  QOpenGLTexture* m_blurred_occlusion_texture;

  QOpenGLShaderProgram* m_geom_program;
  QOpenGLShaderProgram* m_ssao_program;
  QOpenGLShaderProgram* m_blur_program;
  QOpenGLShaderProgram* m_lighting_program;

  QOpenGLVertexArrayObject* m_quad_vao;
  QOpenGLBuffer m_quad_vbo;

  QOpenGLVertexArrayObject* m_geom_vao;
  QOpenGLBuffer m_geom_vbo;
  QOpenGLBuffer m_geom_ebo;
  std::vector<GLuint> m_geom_indices;
  std::vector<GLfloat> m_geom_vertices;

  float m_keepSpinning;

  QMatrix4x4 m_M;
  QMatrix4x4 m_V;
  QMatrix4x4 m_P;

  QOpenGLTexture* m_noiseTexture;

  std::vector<QVector3D> m_ssao_kernel;


public slots:
  void keyPressEvent(QKeyEvent* ev) override;
};

#endif // SCENE_H
