/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SliderAction.h"

#include <QHBoxLayout>

namespace EspINA
{
  //------------------------------------------------------------------------
  SliderAction::SliderAction(QObject *parent)
  : QWidgetAction(parent)
  , m_label(nullptr)
  , m_slider(nullptr)
  , m_value(0)
  , m_text(QString())
  , m_enabled(true)
  , m_maximumValue(30)
  , m_minimumValue(1)
  {
  }
  
  //------------------------------------------------------------------------
  SliderAction::~SliderAction()
  {
    if (m_label)
      delete m_label;

    if (m_slider)
      delete m_slider;
  }

  //------------------------------------------------------------------------
  QWidget* SliderAction::createWidget(QWidget* parent)
  {
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout();
    widget->setLayout(layout);

    m_label = new QLabel(m_text);
    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setFixedWidth(100);

    // only catching one of them will suffice
    connect(m_slider, SIGNAL(destroyed(QObject*)), this, SLOT(destroySignalEmmited()));

    m_slider->setValue(m_value);
    m_slider->setMinimum(m_minimumValue);
    m_slider->setMaximum(m_maximumValue);
    m_slider->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    connect(m_slider,SIGNAL(valueChanged(int)),
            this, SLOT(setValue(int)));

    layout->addWidget(m_label);
    layout->addWidget(m_slider);

    m_label->setEnabled(m_enabled);
    m_slider->setEnabled(m_enabled);

    return widget;
  }

  //------------------------------------------------------------------------
  void SliderAction::setValue(int value)
  {
    m_value = value;
    if (m_slider != nullptr)
      m_slider->setValue(value);
    emit valueChanged(value);
  }

  //------------------------------------------------------------------------
  void SliderAction::setSliderMinimum(int value)
  {
    m_minimumValue = value;

    if (m_slider != nullptr)
    {
      m_slider->setMinimum(value);
      if (m_value < value)
      {
        m_value = value;
        m_slider->setValue(m_value);
      }
    }
  }

  //------------------------------------------------------------------------
  void SliderAction::setSliderMaximum(int value)
  {
    m_maximumValue = value;

    if (m_slider != nullptr)
    {
      m_slider->setMaximum(value);
      if (m_value > value)
      {
        m_value = value;
        m_slider->setValue(m_value);
      }
    }
  }

  //------------------------------------------------------------------------
  void SliderAction::setLabelText(const QString &label)
  {
    m_text = label;

    if (m_label != nullptr)
      m_label->setText(m_text);
  }

} /* namespace EspINA */
