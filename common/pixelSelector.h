#ifndef PIXELSELECTOR_H_
#define PIXELSELECTOR_H_

#include "selectionManager.h"

class PixelSelector 
: public ISelectionHandler
{
public:
  PixelSelector() {}
  virtual ~PixelSelector(){}
  
    virtual void onMouseDown(QPoint &pos, ISelectableView* view);
    virtual void onMouseMove(QPoint &pos, ISelectableView* view);
    virtual void onMouseUp(QPoint &pos, ISelectableView* view);
};

class BestPixelSelector
: public PixelSelector
{
public:
  BestPixelSelector() 
  : window(20,20)
  {}
  virtual ~BestPixelSelector(){}
  
  virtual void onMouseDown(QPoint& pos, ISelectableView* view);
  
  QSize window;
};

#endif //PIXELSELECTOR_H_
