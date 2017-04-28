// Qt
#include <QVBoxLayout>
#include <QLabel>

// Project
#include "Panel.h"

Panel::Panel(QWidget *parent) : QWidget(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout;

  QLabel *m_radius_label = new QLabel("Radius");
  m_radius_slider = new QSlider(Qt::Horizontal);
  m_radius_slider->setValue(50);

  QLabel *m_kernel_size_label = new QLabel("Kernel Size");
  m_kernel_size_slider = new QSlider(Qt::Horizontal);
  m_kernel_size_slider->setRange(32, 96);
  m_kernel_size_slider->setValue(64);

  QLabel *m_bias_label = new QLabel("Bias");
  m_bias_slider = new QSlider(Qt::Horizontal);

  QLabel *m_blur_amount_label = new QLabel("Blur Amount");
  m_blur_amount_slider = new QSlider(Qt::Horizontal);
  m_blur_amount_slider->setRange(2, 8);
  m_blur_amount_slider->setValue(2);

  mainLayout->addWidget(m_radius_label);
  mainLayout->addWidget(m_radius_slider);
  mainLayout->addWidget(m_kernel_size_label);
  mainLayout->addWidget(m_kernel_size_slider);
  mainLayout->addWidget(m_bias_label);
  mainLayout->addWidget(m_bias_slider);
  mainLayout->addWidget(m_blur_amount_label);
  mainLayout->addWidget(m_blur_amount_slider);

  setLayout(mainLayout);
}
