#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include <QSlider>


class Panel : public QWidget
{
  Q_OBJECT
public:
  explicit Panel(QWidget *parent = 0);
  QSlider *m_radius_slider;
  QSlider *m_kernel_size_slider;
  QSlider *m_bias_slider;
  QSlider *m_blur_amount_slider;

signals:

public slots:

};

#endif // PANEL_H
