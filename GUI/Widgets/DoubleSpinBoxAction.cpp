/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "DoubleSpinBoxAction.h"

// Qt
#include <QHBoxLayout>

namespace ESPINA
{
  //------------------------------------------------------------------------
  DoubleSpinBoxAction::DoubleSpinBoxAction(QObject *parent)
  : QWidgetAction {parent}
  , m_label       {nullptr}
  , m_spinBox     {nullptr}
  , m_value       {0}
  , m_enabled     {true}
  , m_maximumValue{30}
  , m_minimumValue{1}
  , m_step        {1}
  {
  }

  //------------------------------------------------------------------------
  DoubleSpinBoxAction::~DoubleSpinBoxAction()
  {
    if (m_label)
    {
      delete m_label;
    }

    if (m_spinBox)
    {
      delete m_spinBox;
    }
  }

  //------------------------------------------------------------------------
  QWidget* DoubleSpinBoxAction::createWidget(QWidget* parent)
  {
    QWidget*     widget = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(widget);

    widget->setLayout(layout);

    m_label   = new QLabel(m_text, widget);
    m_spinBox = new QDoubleSpinBox(widget);

    m_spinBox->setValue(m_value);
    m_spinBox->setMinimum(m_minimumValue);
    m_spinBox->setMaximum(m_maximumValue);
    m_spinBox->setSuffix(m_suffix);
    m_spinBox->setSingleStep(m_step);

    connect(m_spinBox, SIGNAL(destroyed(QObject*)),
            this,      SLOT(destroySignalEmmited()));
    connect(m_spinBox, SIGNAL(valueChanged(double)),
            this,      SLOT(setValue(double)));

    layout->addWidget(m_label);
    layout->addWidget(m_spinBox);

    m_label  ->setEnabled(m_enabled);
    m_spinBox->setEnabled(m_enabled);

    return widget;
  }

  //------------------------------------------------------------------------
  void DoubleSpinBoxAction::setValue(double value)
  {
    m_value = value;
    if (m_spinBox)
    {
      m_spinBox->setValue(value);
    }

    emit valueChanged(value);
  }

  //------------------------------------------------------------------------
  void DoubleSpinBoxAction::setSpinBoxMinimum(double value)
  {
    m_minimumValue = value;

    if (m_spinBox)
    {
      m_spinBox->setMinimum(value);
      if (m_value < value)
      {
        m_value = value;
        m_spinBox->setValue(m_value);
      }
    }
  }

  //------------------------------------------------------------------------
  void DoubleSpinBoxAction::setSpinBoxMaximum(double value)
  {
    m_maximumValue = value;

    if (m_spinBox)
    {
      m_spinBox->setMaximum(value);
      if (m_value > value)
      {
        m_value = value;
        m_spinBox->setValue(m_value);
      }
    }
  }

  //------------------------------------------------------------------------
  void DoubleSpinBoxAction::setLabelText(const QString &label)
  {
    m_text = label;

    if (m_label)
    {
      m_label->setText(m_text);
    }
  }

  //------------------------------------------------------------------------
  void DoubleSpinBoxAction::setSuffix(const QString &suffix)
  {
    m_suffix = suffix;

    if (m_spinBox)
    {
      m_spinBox->setSuffix(suffix);
    }
  }

  //------------------------------------------------------------------------
  void DoubleSpinBoxAction::setStepping(double value)
  {
    m_step = value;

    if(m_spinBox)
    {
      m_spinBox->setSingleStep(m_step);
    }
  }

} /* namespace ESPINA */
