#ifndef PIXELSELECTOR_H_
#define PIXELSELECTOR_H_

#include "selectionManager.h"

class PixelSelector 
: public ISelectionHandler
{
  Q_OBJECT
  
public:
  PixelSelector(QObject* parent = 0) : ISelectionHandler(parent){}
  virtual ~PixelSelector(){}
  
  virtual void onMouseDown(QMouseEvent* event, pqTwoDRenderView* view);
  virtual void onMouseMove(QMouseEvent* event, pqTwoDRenderView* view);
  virtual void onMouseUp(QMouseEvent* event, pqTwoDRenderView* view);
  
  virtual void abortSelection(){}
  
signals:
  void pixelSelected(int /*x*/, int /*y*/, int/*z*/);
protected:
  int x, y, z;
};

class BestPixelSelector
: public PixelSelector
{
  Q_OBJECT
public:
  BestPixelSelector(QObject* parent = 0) : PixelSelector(parent) {}
  virtual ~BestPixelSelector() {};
  
  //virtual void onMouseDown(QMouseEvent* event, pqTwoDRenderView* view);
  //virtual void onMouseMove(QMouseEvent* event, pqTwoDRenderView* view);
  //virtual void onMouseUp(QMouseEvent* event, pqTwoDRenderView* view);
};

#endif //PIXELSELECTOR_H_
