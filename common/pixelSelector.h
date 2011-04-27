#ifndef PIXELSELECTOR_H_
#define PIXELSELECTOR_H_

#include "selectionManager.h"

class PixelSelector 
: public ISelectionHandler
{
public:
  PixelSelector() {}
  virtual ~PixelSelector(){}
  
    virtual void onMouseDown(QPointF &pos, ISelectableView* view);
    virtual void onMouseMove(QPointF &pos, ISelectableView* view);
    virtual void onMouseUp(QPointF &pos, ISelectableView* view);
};

#endif //PIXELSELECTOR_H_
