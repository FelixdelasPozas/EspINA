/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "ImageResolutionDialog.h"

// Qt
#include <QDialog>
#include <QSpinBox>
#include <QDebug>

//-----------------------------------------------------------------------------
ESPINA::ImageResolutionDialog::ImageResolutionDialog(QWidget *parent, int width,
    int height)
{
  setupUi(this);

  m_width = width;
  m_height = height;
  m_initialWidth = width;
  m_initialHeight = height;
  m_ratio = float(width) / height;
  m_width_spinBox->setValue(width);
  m_height_spinBox->setValue(height);

  connect(m_width_spinBox, SIGNAL(valueChanged(int)), this,
      SLOT(onWidthChanged(int)));
  connect(m_height_spinBox, SIGNAL(valueChanged(int)), this,
      SLOT(onHeightChanged(int)));
}

//-----------------------------------------------------------------------------
int ESPINA::ImageResolutionDialog::getMagnifcation()
{
  return 4096.0 / m_width + 0.5;
}

//-----------------------------------------------------------------------------
void ESPINA::ImageResolutionDialog::onHeightChanged(int value)
{
  m_height = value; //m_height_spinBox->value();
  m_width = m_height * m_ratio;
  m_width_spinBox->blockSignals(true);
  m_width_spinBox->setValue(m_width);
  m_width_spinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void ESPINA::ImageResolutionDialog::onWidthChanged(int value)
{
  m_width = value; //m_weight_spinBox->value();
  m_height = m_width / m_ratio;
  m_height_spinBox->blockSignals(true);
  m_height_spinBox->setValue(m_height);
  m_height_spinBox->blockSignals(false);
}
