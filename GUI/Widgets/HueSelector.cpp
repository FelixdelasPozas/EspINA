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
#include "HueSelector.h"

// Qt
#include <QMouseEvent>
#include <QPainter>
#include <QBrush>

using namespace ESPINA;

//------------------------------------------------------------------------
int HueSelector::x2val(int x)
{
  double value = (x * 360)/width();
  if(m_reserveInitialValue)
  {
    return static_cast<int>(value--);
  }
  else
  {
    return static_cast<int>(value);
  }
}

//------------------------------------------------------------------------
int HueSelector::val2x(int v)
{
  return (v*width()) / 360;
}

//------------------------------------------------------------------------
HueSelector::HueSelector(QWidget* parent)
: QWidget(parent)
, m_reserveInitialValue{true}
, m_enabled            {true}
, m_needsRepaint       {false}
{
  m_hue = 0;
  m_val = 255;
  m_sat = 255;
  m_pix = 0;
}

//------------------------------------------------------------------------
HueSelector::~HueSelector()
{
  delete m_pix;
}

//------------------------------------------------------------------------
void HueSelector::mouseMoveEvent(QMouseEvent *m)
{
  setVal(x2val(m->x()));
}

//------------------------------------------------------------------------
void HueSelector::mousePressEvent(QMouseEvent *m)
{
  setVal(x2val(m->x()));
}

//------------------------------------------------------------------------
void HueSelector::setVal(int v)
{
  if (m_hue == v) return;

  m_hue = std::max(0, std::min(v, m_reserveInitialValue ? 360 : 359));
  delete m_pix;
  m_pix = 0;
  repaint();
  emit newHsv(m_hue, m_sat, m_val);
}

//------------------------------------------------------------------------
void HueSelector::paintEvent(QPaintEvent *event)
{
  QRect rect(0, 7, width(), height());
  int wi = rect.width();
  int hi = rect.height();
  if (!m_pix || m_pix->height() != hi || m_pix->width() != wi || m_needsRepaint)
  {
    delete m_pix;
    QImage img(wi, hi, QImage::Format_RGB32);

    for (int X = 0; X < wi; X++)
    {
      int value = x2val(X);
      for (int Y = 0; Y < hi; Y++)
      {
        if ((0 == value) && m_reserveInitialValue)
        {
          img.setPixel(X, Y, QColor(0,0,0).rgb());
        }
        else
        {
          QColor col;
          col.setHsv(value, 255, 255);
          if(!m_enabled)
          {
            auto gray = qGray(col.red(), col.green(), col.blue());
            col = QColor(gray, gray, gray);
          }
          img.setPixel(X, Y, col.rgb());
        }
      }
    }
    m_pix = new QPixmap;
    m_pix->convertFromImage(img);
    m_needsRepaint = false;
  }

  QPainter p(this);
  p.drawPixmap(0, 10, *m_pix);
  QPoint arrow[3];
  int x = val2x(m_hue);
  arrow[0] = QPoint(x,10);
  arrow[1] = QPoint(x-10, 0);
  arrow[2] = QPoint(x+10, 0);
  p.eraseRect(x-5, 0, 10, 5);

  QBrush brush(Qt::black, Qt::SolidPattern);
  p.setBrush(brush);
  p.setBackground(Qt::black);
  p.drawPolygon(arrow,3, Qt::WindingFill);

  QWidget::paintEvent(event);
}

//------------------------------------------------------------------------
void HueSelector::setHueValue(int h)
{
  m_hue = std::max(0, std::min(h, m_reserveInitialValue ? 360 : 359));

  delete m_pix;
  m_pix = 0;
  repaint();
  emit newHsv(m_hue,m_sat, m_val);
}

//------------------------------------------------------------------------
void HueSelector::setEnabled(bool value)
{
  if(m_enabled != value)
  {
    m_enabled = value;
    m_needsRepaint = true; // false after a repaint.

    // NOTE: a disabled widget doesn't receive events from a repaint(), so we need to force a repaint.
    if(!value) repaint();

    QWidget::setEnabled(value);

    if(value) repaint();

    update();
  }
}
