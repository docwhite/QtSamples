////////////////////////////////////////////////////////////////////////////////
/// @file main.cpp
/// @author Ramon Blanquer
/// @version 0.0.1
////////////////////////////////////////////////////////////////////////////////

// Qt
#include <QGuiApplication>
#include <QPropertyAnimation>

// Project
#include "Scene.h"
#include "Window.h"

int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv);

  Window window;
  QSurfaceFormat fmt;
  fmt.setVersion(3, 3);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setSamples(16);
  fmt.setSwapInterval(1);
  window.setFormat(fmt);

  Scene scene(&window);
  window.setScene(&scene);

  window.resize(720, 720);
  window.show();

  return app.exec();
}
