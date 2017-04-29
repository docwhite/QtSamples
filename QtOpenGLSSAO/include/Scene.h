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
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

// Project
#include "AbstractScene.h"
#include "Window.h"

class Scene : public AbstractScene, public QObject
{
public:
  Scene(const std::pair<float,float> _ssao_radius_range, const std::pair<float,float> _ssao_bias_range, Window *_window);
  ~Scene();

  void initialize() override;
  void paint() override;

  const std::pair<float, float> m_ssao_radius_range;
  const std::pair<float, float> m_ssao_bias_range;

private:
  GLfloat m_ssao_radius;
  GLfloat m_ssao_bias;

  QOpenGLFramebufferObject* m_gbuffer_fbo;
  QOpenGLFramebufferObject* m_ssao_fbo;
  QOpenGLFramebufferObject* m_blur_fbo;

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
  std::vector<GLfloat> m_geom_vertices;
  std::vector<GLuint> m_geom_indices;

  QMatrix4x4 m_M;
  QMatrix4x4 m_V;
  QMatrix4x4 m_P;

  float m_keepSpinning;

  QOpenGLTexture* m_noiseTexture;

  std::vector<QVector3D> m_ssao_kernel;

  QVector3D m_eye;
  QVector3D m_center;

public slots:
  void keyPressEvent(QKeyEvent* ev) override;
  void setSSAORadius(int _value);
  void setSSAOKernelSize(int _value);
  void setSSAOBias(int _value);
  void setSSAOBlurAmount(int _value);
};

#endif // SCENE_H
