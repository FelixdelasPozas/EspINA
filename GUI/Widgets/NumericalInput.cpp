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

// ESPINA
#include "NumericalInput.h"

// Qt
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;

//------------------------------------------------------------------------
NumericalInput::NumericalInput(QWidget *parent)
: QWidget  (parent)
, m_label  (new QLabel(this))
, m_slider (new QSlider(this))
, m_spinBox(new QSpinBox(this))
{
  m_slider->setOrientation(Qt::Horizontal);
  m_slider->setFixedWidth(80);

  m_slider->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  m_spinBox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

  connect(m_spinBox, SIGNAL(valueChanged(int)),
          m_slider, SLOT(setValue(int)));

  connect(m_slider, SIGNAL(valueChanged(int)),
          this,     SLOT(setValue(int)));


  auto layout = new QHBoxLayout();

  layout->addWidget(m_label);
  layout->addWidget(m_slider);
  layout->addWidget(m_spinBox);

  setLayout(layout);
}

//------------------------------------------------------------------------
NumericalInput::~NumericalInput()
{
}

//------------------------------------------------------------------------
void NumericalInput::setValue(int value)
{
  //m_slider ->setValue(value);
  m_spinBox->setValue(value);

  emit valueChanged(value);
}

//------------------------------------------------------------------------
int NumericalInput::value() const
{
  return m_slider->value();
}

//------------------------------------------------------------------------
void NumericalInput::setMinimum(int value)
{
  m_slider->setMinimum(value);
  m_spinBox->setMinimum(value);
}

//------------------------------------------------------------------------
void NumericalInput::setMaximum(int value)
{
  m_slider->setMaximum(value);
  m_spinBox->setMaximum(value);
}

//------------------------------------------------------------------------
void NumericalInput::setLabelText(const QString &label)
{
  m_label->setText(label);
}

//------------------------------------------------------------------------
void NumericalInput::setLabelVisibility(bool value)
{
  m_label->setVisible(value);
}

//------------------------------------------------------------------------
void NumericalInput::setSliderVisibility(bool value)
{
  m_slider->setVisible(value);
}

//------------------------------------------------------------------------
void NumericalInput::setSpinBoxVisibility(bool value)
{
  m_spinBox->setVisible(value);
}
