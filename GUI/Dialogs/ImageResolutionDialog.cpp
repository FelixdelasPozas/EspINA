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
    : QDialog
    { parent }, m_ratio
    { float(width) / height }, m_initialHeight
    { height }, m_initialWidth
    { width }, m_height
    { height }, m_width
    { width }
{
  setupUi(this);

  m_width_spinBox->setValue(width);
  m_height_spinBox->setValue(height);

  connect(m_width_spinBox, SIGNAL(valueChanged(int)), this,
      SLOT(onWidthChanged(int)));
  connect(m_height_spinBox, SIGNAL(valueChanged(int)), this,
      SLOT(onHeightChanged(int)));
}

//-----------------------------------------------------------------------------
ESPINA::ImageResolutionDialog::ImageResolutionDialog(QWidget* parent, int width,
    int height, QImage& image)
    : ImageResolutionDialog
    { parent, width, height }
{
  auto thumb = image.scaled(m_image_label->maximumSize(), Qt::KeepAspectRatio,
      Qt::SmoothTransformation);
  m_image_label->setPixmap(QPixmap::fromImage(thumb));
}

//-----------------------------------------------------------------------------
int ESPINA::ImageResolutionDialog::getMagnifcation() const
{
  double re = m_width / m_initialWidth + 0.5;
  return (re < 1) ? 1 : re;
}

//-----------------------------------------------------------------------------
void ESPINA::ImageResolutionDialog::onHeightChanged(int value)
{
  m_height = value;
  m_width = m_height * m_ratio;
  m_width_spinBox->blockSignals(true);
  m_width_spinBox->setValue(m_width);
  m_width_spinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void ESPINA::ImageResolutionDialog::onWidthChanged(int value)
{
  m_width = value;
  m_height = m_width / m_ratio;
  m_height_spinBox->blockSignals(true);
  m_height_spinBox->setValue(m_height);
  m_height_spinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
const double ESPINA::ImageResolutionDialog::getRatio() const
{
  return m_ratio;
}

//-----------------------------------------------------------------------------
const int ESPINA::ImageResolutionDialog::getInitialHeight() const
{
  return m_initialHeight;
}

//-----------------------------------------------------------------------------
const int ESPINA::ImageResolutionDialog::getInitialWidth() const
{
  return m_initialWidth;
}

//-----------------------------------------------------------------------------
int ESPINA::ImageResolutionDialog::getHeight() const
{
  return m_height;
}

//-----------------------------------------------------------------------------
int ESPINA::ImageResolutionDialog::getWidth() const
{
  return m_width;
}

//-----------------------------------------------------------------------------
const QSize ESPINA::ImageResolutionDialog::getInitialSize() const
{
  return QSize(m_initialWidth, m_initialHeight);
}

//-----------------------------------------------------------------------------
QSize ESPINA::ImageResolutionDialog::getSize() const
{
  return QSize(m_width, m_height);
}
