#ifndef IPIXELSELECTOR_H_
#define IPIXELSELECTOR_H_

//TODO: Change to implementations
#include "interfaces.h"

class IPixelSelector
{
public:
  virtual ~IPixelSelector(){}
  virtual ImagePixel pickPixel(const Selection &sel) = 0;
};


class PixelSelector : public IPixelSelector
{
public:
  PixelSelector();
  virtual ~PixelSelector();
  
  virtual ImagePixel pickPixel(const Selection& sel);
};

#endif//IPIXELSELECTOR_H_
