/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "SpinBoxAction.h"

#include <QHBoxLayout>

namespace EspINA
{
  //------------------------------------------------------------------------
  SpinBoxAction::SpinBoxAction(QObject *parent)
  : QWidgetAction(parent)
  , m_radiusLabel(nullptr)
  , m_radiusSpinBox(nullptr)
  , m_radius(0)
  , m_label(QString())
  , m_enabled(true)
  , m_maximumValue(30)
  , m_minimumValue(1)
  {
  }
  
  //------------------------------------------------------------------------
  SpinBoxAction::~SpinBoxAction()
  {
    if (m_radiusLabel)
      delete m_radiusLabel;

    if (m_radiusSpinBox)
      delete m_radiusSpinBox;
  }

  //------------------------------------------------------------------------
  QWidget* SpinBoxAction::createWidget(QWidget* parent)
  {
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout();
    widget->setLayout(layout);

    m_radiusLabel = new QLabel(m_label);
    m_radiusSpinBox = new QSpinBox();

    // only catching one of them will suffice
    connect(m_radiusSpinBox, SIGNAL(destroyed(QObject*)), this, SLOT(destroySignalEmmited()));


    m_radiusSpinBox->setValue(m_radius);
    m_radiusSpinBox->setMinimum(m_minimumValue);
    m_radiusSpinBox->setMaximum(m_maximumValue);

    connect(m_radiusSpinBox,SIGNAL(valueChanged(int)),
            this, SLOT(setRadius(int)));

    layout->addWidget(m_radiusLabel);
    layout->addWidget(m_radiusSpinBox);

    m_radiusLabel->setEnabled(m_enabled);
    m_radiusSpinBox->setEnabled(m_enabled);

    return widget;
  }

  //------------------------------------------------------------------------
  void SpinBoxAction::setRadius(int value)
  {
    m_radius = value;
    emit radiusChanged(value);
  }

  //------------------------------------------------------------------------
  void SpinBoxAction::setSpinBoxMinimum(int value)
  {
    m_minimumValue = value;

    if (m_radiusSpinBox != nullptr)
    {
      m_radiusSpinBox->setMinimum(value);
      if (m_radius < value)
      {
        m_radius = value;
        m_radiusSpinBox->setValue(m_radius);
      }
    }
  }

  //------------------------------------------------------------------------
  void SpinBoxAction::setSpinBoxMaximum(int value)
  {
    m_maximumValue = value;

    if (m_radiusSpinBox != nullptr)
    {
      m_radiusSpinBox->setMaximum(value);
      if (m_radius > value)
      {
        m_radius = value;
        m_radiusSpinBox->setValue(m_radius);
      }
    }
  }

  //------------------------------------------------------------------------
  void SpinBoxAction::setLabelText(const QString &label)
  {
    m_label = label;

    if (m_radiusLabel != nullptr)
      m_radiusLabel->setText(m_label);
  }


} /* namespace EspINA */
