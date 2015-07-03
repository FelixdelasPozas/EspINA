/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Widgets/PixelValueSelector.h>
#include <QPainter>
#include <QMouseEvent>

using namespace ESPINA::GUI::Widgets;
  
//------------------------------------------------------------------------
PixelValueSelector::PixelValueSelector(QWidget *parent)
: QWidget {parent}
, m_value {0}
, m_pixmap{nullptr}
{
}

//------------------------------------------------------------------------
PixelValueSelector::~PixelValueSelector()
{
  if(m_pixmap)
  {
    delete m_pixmap;
  }
}

//------------------------------------------------------------------------
void PixelValueSelector::setValue(int value)
{
  if(value != m_value)
  {
    m_value = value;
    repaint();

    emit newValue(value);
  }
}

//------------------------------------------------------------------------
int PixelValueSelector::value() const
{
  return m_value;
}

//------------------------------------------------------------------------
void PixelValueSelector::paintEvent(QPaintEvent *unused)
{
  QRect rect(0, 7, width(), height());
  auto wwidth  = rect.width();
  auto wheight = rect.height();

  if (!m_pixmap || m_pixmap->height() != wheight || m_pixmap->width() != wwidth)
  {
    delete m_pixmap;
    QImage image(wwidth, wheight, QImage::Format_RGB32);

    for (int i = 0; i < wwidth; i++)
    {
      for (int j = 0; j < wheight; j++)
      {
         auto grayValue = (i * 254) / width();
         QColor color(grayValue, grayValue, grayValue);

         image.setPixel(i, j, color.rgb());
      }
    }
    m_pixmap = new QPixmap;
    m_pixmap->convertFromImage(image);
  }

  QPainter painter(this);
  painter.drawPixmap(0, 10, *m_pixmap);
  QPoint arrow[3];

  int arrow_x = (m_value*width()) / 254;

  arrow[0] = QPoint(arrow_x,10);
  arrow[1] = QPoint(arrow_x-10, 0);
  arrow[2] = QPoint(arrow_x+10, 0);
  painter.eraseRect(arrow_x-5, 0, 10, 5);

  QBrush brush(Qt::black, Qt::SolidPattern);
  painter.setBrush(brush);
  painter.setBackground(Qt::black);
  painter.drawPolygon(arrow,3, Qt::WindingFill);
}

//------------------------------------------------------------------------
void PixelValueSelector::mouseMoveEvent(QMouseEvent *e)
{
  auto xPos = e->x();

  if(xPos < 0)       xPos = 0;
  if(xPos > width()) xPos = width();

  auto value = (xPos * 254)/width();
  setValue(value);
}

//------------------------------------------------------------------------
void PixelValueSelector::mousePressEvent(QMouseEvent *e)
{
  mouseMoveEvent(e);
}
