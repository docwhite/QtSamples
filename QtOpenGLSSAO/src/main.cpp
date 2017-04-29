////////////////////////////////////////////////////////////////////////////////
/// @file main.cpp
/// @author Ramon Blanquer
/// @version 0.0.1
////////////////////////////////////////////////////////////////////////////////

// Qt
#include <QObject>
#include <QApplication>

// Project
#include "Scene.h"
#include "Window.h"
#include "Panel.h"

const std::pair<float, float> ssao_radius_range = std::make_pair(0.01f, 0.9f);
const std::pair<float, float> ssao_bias_range   = std::make_pair(0.000001f, 0.009f);

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  Panel panel;

  Window window;
  QSurfaceFormat fmt;
  fmt.setVersion(3, 3);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setSamples(16);
  fmt.setSwapInterval(1);
  window.setFormat(fmt);

  Scene scene(ssao_radius_range, ssao_bias_range, &window);
  window.setScene(&scene);

  QObject::connect(panel.m_radius_slider, &QSlider::valueChanged, &scene, &Scene::setSSAORadius);
  QObject::connect(panel.m_bias_slider, &QSlider::valueChanged, &scene, &Scene::setSSAOBias);
  QObject::connect(panel.m_kernel_size_slider, &QSlider::valueChanged, &scene, &Scene::setSSAOKernelSize);
  QObject::connect(panel.m_blur_amount_slider, &QSlider::valueChanged, &scene, &Scene::setSSAOBlurAmount);

  window.resize(720, 720);
  window.show();
  panel.show();

  return app.exec();
}
