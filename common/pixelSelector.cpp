#include "pixelSelector.h"

#include <QDebug>

void PixelSelector::onMouseDown(QMouseEvent* event, pqTwoDRenderView* view)
{
  //TODO: Copy view's existing method
  x = y = z = 5;
  
  qDebug() << "EspINA::PixelSelector: Click on" << x << y << z;
  emit pixelSelected(x,y,z);
}

void PixelSelector::onMouseMove(QMouseEvent* event, pqTwoDRenderView* view)
{
  //Do nothing
  qDebug("Moving");
}

void PixelSelector::onMouseUp(QMouseEvent* event, pqTwoDRenderView* view)
{
  qDebug() << "EspINA::PixelSelector: Mouse released";
  emit pixelSelected(x,y,z);
}




