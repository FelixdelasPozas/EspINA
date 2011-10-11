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
  : window(14,14)
  , m_bestPixel(0)
  {}
  virtual ~BestPixelSelector(){}
  
  void setBestPixelValue(int value) {m_bestPixel = value;}
  
  virtual void onMouseDown(QPoint& pos, ISelectableView* view);
  
  QSize window;
private:
  int m_bestPixel;
};

#endif //PIXELSELECTOR_H_
