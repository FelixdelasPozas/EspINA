/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include <vtkImplicitFunction.h>
#include "CustomROIWidget.h"

// Qt
#include <QCheckBox>
#include <QHBoxLayout>

// C++
#include <cstring>

using namespace ESPINA;

const unsigned int DEFAULT_ROI_VALUE = 500;

//------------------------------------------------------------------------
CustomROIWidget::CustomROIWidget(QWidget* parent)
: QWidget {parent}
, m_useROI{true}
{
  // default values
  std::memset(m_values, DEFAULT_ROI_VALUE, sizeof(unsigned int)*3);

  auto roiCheckBox = new QCheckBox(tr("Apply ROI"), this);
  roiCheckBox->setCheckState(m_useROI?Qt::Checked:Qt::Unchecked);

  QString labels[3] = {"X:", "Y:", "Z:"};
  for(int i = 0; i < 3; ++i)
  {
    m_labelROI[i] = new QLabel(this);
    m_labelROI[i]->setText(labels[i]);
    m_labelROI[i]->setVisible(m_useROI);

    m_spinBoxROI[i] = new QSpinBox(this);
    m_spinBoxROI[i]->setVisible(m_useROI);
    m_spinBoxROI[i]->setAlignment(Qt::AlignRight);
    m_spinBoxROI[i]->setMinimum(0);
    m_spinBoxROI[i]->setMaximum(100000);
    m_spinBoxROI[i]->setValue(m_values[i]);
    m_spinBoxROI[i]->setSuffix(" nm");
  }

  connect(m_spinBoxROI[0], SIGNAL(valueChanged(int)),
          this,            SLOT(onXSizeChanged(int)));
  connect(m_spinBoxROI[1], SIGNAL(valueChanged(int)),
          this,            SLOT(onYSizeChanged(int)));
  connect(m_spinBoxROI[2], SIGNAL(valueChanged(int)),
          this,            SLOT(onZSizeChanged(int)));

  auto mainLaout = new QHBoxLayout(this);
  mainLaout->addWidget(roiCheckBox);
  for (int i = 0; i < 3; ++i)
  {
    mainLaout->addWidget(m_labelROI[i]);
    mainLaout->addWidget(m_spinBoxROI[i]);
  }

  setLayout(mainLaout);

  connect(roiCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(onApplyROIChanged(bool)));
}

//------------------------------------------------------------------------
void CustomROIWidget::setValue(Axis axis, unsigned int value)
{
  int i = idx(axis);

  m_values[i] = value;

  m_spinBoxROI[i]->setValue(value);
}

//------------------------------------------------------------------------
void CustomROIWidget::setApplyROI(bool enabled)
{
  m_useROI = enabled;
}

//------------------------------------------------------------------------
void CustomROIWidget::onApplyROIChanged(bool value)
{
   m_useROI = value;

  for(int i = 0; i < 3; ++i)
  {
    if (m_labelROI[i] && m_spinBoxROI[i])
    {
      m_labelROI  [i]->setVisible(m_useROI);
      m_spinBoxROI[i]->setVisible(m_useROI);
    }
  }

   emit useROI(value);
}

//------------------------------------------------------------------------
void CustomROIWidget::onXSizeChanged(int value)
{
  m_values[0] = value;
}

//------------------------------------------------------------------------
void CustomROIWidget::onYSizeChanged(int value)
{
  m_values[1] = value;
}
//------------------------------------------------------------------------
void CustomROIWidget::onZSizeChanged(int value)
{
  m_values[2] = value;
}
