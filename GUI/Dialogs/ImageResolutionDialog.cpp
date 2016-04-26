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

using namespace ESPINA;
using namespace ESPINA::GUI::Dialogs;

//-----------------------------------------------------------------------------
ImageResolutionDialog::ImageResolutionDialog(const int     width,
                                             const int     height,
                                             const QImage& image,
                                             QWidget      *parent)
: QDialog      {parent}
, m_ratio      {double(width) / height}
, m_initialSize{width, height}
, m_size       {width, height}
{
  setupUi(this);

  m_width_spinBox->setValue(width);
  m_height_spinBox->setValue(height);

  auto thumb = image.scaled(m_image_label->maximumSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_image_label->setFixedSize(thumb.size());
  m_image_label->setPixmap(QPixmap::fromImage(thumb));

  // minimal resolution is 100, maintain ratio but do not allow values under that
  if(width <= height)
  {
    m_width_spinBox->setMinimum(100);
    m_height_spinBox->setMinimum(100/m_ratio);
  }
  else
  {
    m_width_spinBox->setMinimum(100*m_ratio);
    m_height_spinBox->setMinimum(100);
  }

  connect(m_width_spinBox,  SIGNAL(valueChanged(int)),
          this,             SLOT(onWidthChanged(int)));
  connect(m_height_spinBox, SIGNAL(valueChanged(int)),
          this,             SLOT(onHeightChanged(int)));
}

//-----------------------------------------------------------------------------
int ImageResolutionDialog::getMagnifcation() const
{
  double re = m_size.width() / m_initialSize.width() + 0.5;
  return (re < 1) ? 1 : re;
}

//-----------------------------------------------------------------------------
void ImageResolutionDialog::onHeightChanged(int value)
{
  m_size.setHeight(value);
  m_size.setWidth(value * m_ratio);

  m_width_spinBox->blockSignals(true);
  m_width_spinBox->setValue(m_size.width());
  m_width_spinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void ImageResolutionDialog::onWidthChanged(int value)
{
  m_size.setWidth(value);
  m_size.setHeight(value / m_ratio);
  m_height_spinBox->blockSignals(true);
  m_height_spinBox->setValue(m_size.height());
  m_height_spinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
const double ImageResolutionDialog::getRatio() const
{
  return m_ratio;
}

//-----------------------------------------------------------------------------
const int ImageResolutionDialog::getInitialHeight() const
{
  return m_initialSize.height();
}

//-----------------------------------------------------------------------------
const int ImageResolutionDialog::getInitialWidth() const
{
  return m_initialSize.width();
}

//-----------------------------------------------------------------------------
int ImageResolutionDialog::getHeight() const
{
  return m_size.height();
}

//-----------------------------------------------------------------------------
int ImageResolutionDialog::getWidth() const
{
  return m_size.width();
}

//-----------------------------------------------------------------------------
const QSize ImageResolutionDialog::getInitialSize() const
{
  return m_initialSize;
}

//-----------------------------------------------------------------------------
QSize ImageResolutionDialog::getSize() const
{
  return m_size;
}
