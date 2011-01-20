#include "iPixelSelector.h"


PixelSelector::PixelSelector() {}

PixelSelector::~PixelSelector() {}

ImagePixel PixelSelector::pickPixel(const Selection& sel)
{
  return sel.coord;
}


