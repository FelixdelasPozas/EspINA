#include "pixelSelector.h"

#include <QDebug>
#include <QMouseEvent>

void PixelSelector::onMouseDown(QPointF &pos, ISelectableView* view)
{
  //TODO: Copy view's existing method
  ViewRegions regions;
  QPolygonF singlePixel;
  singlePixel << pos;
  regions << singlePixel;
  
  view->setSelection(filters, regions);
}

void PixelSelector::onMouseMove(QPointF &pos, ISelectableView* view)
{
  //Do nothing
  qDebug("EspINA::PixelSelector: Mouse Moving");
}

void PixelSelector::onMouseUp(QPointF &pos, ISelectableView* view)
{
  qDebug() << "EspINA::PixelSelector: Mouse released";
}