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
  return static_cast<int>(value--);
}

//------------------------------------------------------------------------
int HueSelector::val2x(int v)
{
  return (v*width()) / 360;
}

//------------------------------------------------------------------------
HueSelector::HueSelector(QWidget* parent)
    : QWidget(parent)
{
  hue = 359;
  val = 255;
  sat = 255;
  pix = 0;
}

//------------------------------------------------------------------------
HueSelector::~HueSelector()
{
  delete pix;
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
  if (this->hue == v)
    return;

  this->hue = qMax(0, qMin(v, 360));
  delete pix;
  pix = 0;
  repaint();
  emit newHsv(this->hue, this->sat, this->val);
}

//------------------------------------------------------------------------
void HueSelector::paintEvent(QPaintEvent *)
{
  QRect rect(0, 7, width(), height());
  int wi = rect.width();
  int hi = rect.height();
  if (!pix || pix->height() != hi || pix->width() != wi)
  {
    delete pix;
    QImage img(wi, hi, QImage::Format_RGB32);

    for (int X = 0; X < wi; X++)
    {
      int value = x2val(X);
      for (int Y = 0; Y < hi; Y++)
      {
        if (0 == value)
          img.setPixel(X, Y, QColor(0,0,0).rgb());
        else
        {
          QColor col;
          col.setHsv(value, 255, 255);
          img.setPixel(X, Y, col.rgb());
        }
      }
    }
    pix = new QPixmap;
    pix->convertFromImage(img);
  }

  QPainter p(this);
  p.drawPixmap(0, 10, *pix);
  QPoint arrow[3];
  int x = val2x(hue);
  arrow[0] = QPoint(x,10);
  arrow[1] = QPoint(x-10, 0);
  arrow[2] = QPoint(x+10, 0);
  p.eraseRect(x-5, 0, 10, 5);

  QBrush brush(Qt::black, Qt::SolidPattern);
  p.setBrush(brush);
  p.setBackground(Qt::black);
  p.drawPolygon(arrow,3, Qt::WindingFill);
}

//------------------------------------------------------------------------
void HueSelector::setHueValue(int h)
{
  h = qMax(0, qMin(h, 360));

  this->hue = h;
  delete pix;
  pix = 0;
  repaint();
  emit newHsv(h,this->sat, this->val);
}
