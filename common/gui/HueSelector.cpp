/*
 * HueSelector.cpp
 *
 *  Created on: August 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Qt includes
#include <QWidget>
#include <QBrush>

// espina includes
#include "HueSelector.h"

#include <iostream>

int QtHuePicker::x2val(int x)
{
  double value = (x * 360)/width();
  return static_cast<int>(value--);
}

int QtHuePicker::val2x(int v)
{
  return (v*width()) / 360;
}

QtHuePicker::QtHuePicker(QWidget* parent)
    : QWidget(parent)
{
  hue = 359;
  val = 255;
  sat = 255;
  pix = 0;
}

QtHuePicker::~QtHuePicker()
{
  delete pix;
}

void QtHuePicker::mouseMoveEvent(QMouseEvent *m)
{
  setVal(x2val(m->x()));
}

void QtHuePicker::mousePressEvent(QMouseEvent *m)
{
  setVal(x2val(m->x()));
}

void QtHuePicker::setVal(int v)
{
  if (this->hue == v)
    return;
  
  this->hue = qMax(0, qMin(v, 360));
  delete pix;
  pix = 0;
  repaint();
  emit newHsv(this->hue, this->sat, this->val);
}

void QtHuePicker::paintEvent(QPaintEvent *)
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

void QtHuePicker::setHueValue(int h)
{
  h = qMax(0, qMin(h, 360));;

  this->hue = h;
  delete pix;
  pix = 0;
  repaint();
  emit newHsv(h,this->sat, this->val);
}

HueSelector::HueSelector(double stain, QWidget *parent_Widget, Qt::WindowFlags f)
    : QDialog(parent_Widget, f)
{
  setupUi(this); // this sets up GUI
  this->modified = false;
  if (-1 == stain)
    this->hue = 0;
  else
    this->hue = (stain* 359) +1;

  hp = new QtHuePicker(this);
  hp->setFixedHeight(40);
  hp->setHueValue(hue);
  this->hueLayout->addWidget(hp);

  // spinbox goes the whole range
  hueBox->setValue(this->hue-1);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(AcceptedData()));
  connect(hueBox, SIGNAL(valueChanged(int)), this, SLOT(spinboxChanged(int)));
  connect(hp, SIGNAL(newHsv(int,int,int)), this, SLOT(newHsv(int,int,int)));
}

HueSelector::~HueSelector()
{
  delete this->hp;
}

bool HueSelector::ModifiedData()
{
  return this->modified;
}

void HueSelector::AcceptedData()
{
  this->modified = true;
}

int HueSelector::GetHueValue()
{
  return this->hue-1;
}

void HueSelector::SetHueValue(int h)
{
  this->hueBox->setValue(h-1);
  hp->setHueValue(h);
}

void HueSelector::newHsv(int h, int s, int v)
{
  this->hueBox->setValue(h-1);
  this->hue = h;
}

void HueSelector::spinboxChanged(int value)
{
  hp->setHueValue(value+1);
}
