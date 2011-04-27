#include "pixelSelector.h"

#include <QDebug>
#include <QMouseEvent>

#include <pqTwoDRenderView.h>
#include <vtkImageData.h>
#include <QWidget>
#include <QSize>

//-----------------------------------------------------------------------------
void PixelSelector::onMouseDown(QPoint &pos, ISelectableView* view)
{
  ViewRegions regions;
  QPolygon singlePixel;
  singlePixel << pos;
  regions << singlePixel;
  
  view->setSelection(filters, regions);
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseMove(QPoint &pos, ISelectableView* view)
{
  //Do nothing
  qDebug("EspINA::PixelSelector: Mouse Moving");
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseUp(QPoint &pos, ISelectableView* view)
{
  qDebug() << "EspINA::PixelSelector: Mouse released";
}

//! Coordinates:
//! TL   ^
//!      |
//!      |
//! <----P---->
//!      |
//!      |
//!      v    BR
//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(QPoint& pos, ISelectableView* view)
{
  pqTwoDRenderView *rw = dynamic_cast<pqTwoDRenderView *>(view->view());
  vtkImageData *img = rw->captureImage(1);
  
  int extent[6];
  img->GetExtent(extent);
  
  int leftPixel = pos.x() - window.width()/2;
  if (leftPixel < extent[0])
    leftPixel = extent[0];
  
  int rightPixel = pos.y() + window.width()/2;
  if (rightPixel > extent[1]) 
    rightPixel = extent[1];
  
  int topPixel = pos.y() - window.height()/2;
  if (topPixel < extent[2])
    rightPixel = extent[2];
  
  int bottomPixel = pos.y() + window.height()/2;
  if (bottomPixel > extent[3]) 
    rightPixel = extent[3];
  
  QPoint bestPixel = pos;
  unsigned char * pixel;
  double bestValue;
  
  pixel = ((unsigned char *)img->GetScalarPointer(pos.x(),pos.y(),0));
  bestValue = pixel[0];
  
  //qDebug() << "EspINA::BestPixelSelector: Scalar componets:" <<img->GetNumberOfScalarComponents();
  
  for (int x = leftPixel; x <= rightPixel; x++)
    for (int y = topPixel; y <= bottomPixel; y++)
    {
      pixel = ((unsigned char *)img->GetScalarPointer(x,y,0));
      if (pixel[0] < bestValue)
      {
	bestValue = pixel[0];
	bestPixel = QPoint(x,y);
      }
      //qDebug() << "Pixel(" << x << "," << y<< ") value :" << pixel[0] << pixel[1] << pixel[2];
    }
    qDebug() << "EspINA::BestPixelSelector: Best Pixel(" << bestPixel.x() << "," << bestPixel.y() 
	     << ") value :" << bestValue;
    
    img->Delete();
    
    ViewRegions regions;
    QPolygon singlePixel;
    singlePixel << bestPixel;
    regions << singlePixel;
    
    view->setSelection(filters, regions);
}
