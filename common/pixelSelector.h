#ifndef PIXELSELECTOR_H_
#define PIXELSELECTOR_H_

#include "selectionManager.h"

class PixelSelector 
: public IViewSelector
{
public:
  PixelSelector() {}
  virtual ~PixelSelector(){}
  
    virtual void onMouseDown(QMouseEvent* event, ISelectableView* view);
    virtual void onMouseMove(QMouseEvent* event, ISelectableView* view);
    virtual void onMouseUp(QMouseEvent* event, ISelectableView* view);
};

#endif //PIXELSELECTOR_H_
