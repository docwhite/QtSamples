#ifndef PANEL_H
#define PANEL_H

// Qt
#include <QSlider>
#include <QWidget>

class Panel : public QWidget
{
  Q_OBJECT
public:
  explicit Panel(QWidget *parent = 0);
  QSlider *m_radius_slider;
  QSlider *m_kernel_size_slider;
  QSlider *m_bias_slider;
  QSlider *m_blur_amount_slider;
};

#endif // PANEL_H
