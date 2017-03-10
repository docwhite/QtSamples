#ifndef WINDOW_H
#define WINDOW_H

// Qt
#include <QOpenGLWindow>
#include <QTimer>

// Project
#include "AbstractScene.h"

class Window : public QOpenGLWindow
{

public:
  Window(QWindow *parent = 0);
  AbstractScene* scene() const;
  void setScene(AbstractScene *_scene);

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int _w, int _h);

private:
  AbstractScene *m_scene = nullptr;
  QTimer m_timer;

};

#endif // WINDOW_H
