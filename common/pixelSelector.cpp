#include "pixelSelector.h"

#include <QDebug>

void PixelSelector::onMouseDown(QMouseEvent* event, ISelectableView* view)
{
  //TODO: Copy view's existing method
  ViewRegions regions;
  QPolygon singlePixel;
  singlePixel << QPoint(5,5);
  regions << singlePixel;
  
  qDebug() << "EspINA::PixelSelector: Click on";
  view->setSelection(&regions);
}

void PixelSelector::onMouseMove(QMouseEvent* event, ISelectableView* view)
{
  //Do nothing
  qDebug("Moving");
}

void PixelSelector::onMouseUp(QMouseEvent* event, ISelectableView* view)
{
  qDebug() << "EspINA::PixelSelector: Mouse released";
}