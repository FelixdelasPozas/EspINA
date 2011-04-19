#include "pixelSelector.h"


void PixelSelector::onMouseDown(QMouseEvent* event, pqTwoDRenderView* view)
{
  //TODO: Copy view's existing method
  x = y = z = 0;
}

void PixelSelector::onMouseMove(QMouseEvent* event, pqTwoDRenderView* view)
{
  //Do nothing
}

void PixelSelector::onMouseUp(QMouseEvent* event, pqTwoDRenderView* view)
{
  emit pixelSelected(x,y,z);
}



