#include "PixelSelector.h"

#include <EspinaRenderView.h>

#include <QDebug>
#include <QMouseEvent>
#include <QWidget>
#include <QSize>

#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>

//-----------------------------------------------------------------------------
void PixelSelector::onMouseDown(const QPoint &pos, EspinaRenderView* view)
{
  DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  PickList pickList = view->pick(m_filters, regions);

  emit itemsPicked(pickList);
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseMove(const QPoint &pos, EspinaRenderView* view)
{
  //Do nothing
  //qDebug() << "EspINA::PixelSelector: Mouse Moving: " << pos.x() << pos.y();
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseUp(const QPoint &pos, EspinaRenderView* view)
{
  //qDebug() << "EspINA::PixelSelector: Mouse released";
}

//-----------------------------------------------------------------------------
bool PixelSelector::filterEvent(QEvent* e, EspinaRenderView* view)
{
  // If succesor didn't abort the filtering, apply its own filtering
  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      onMouseDown(me->pos(), view);
      // If handled, prevent other elements to filter the event
      return true;
    }
  }

  return IPicker::filterEvent(e,view);
}


int quadDist(int cx, int cy, int x, int y)
{
  return ((x-cx)*(x-cx)+(y-cy)*(y-cy));
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
BestPixelSelector::BestPixelSelector(IPicker* succesor)
: PixelSelector(succesor)
, m_window     (new QSize(14,14))
, m_bestPixel  (0)
{}

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(const QPoint& pos, EspinaRenderView* view)
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
  vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(view->renderWindow());
  //windowToImageFilter->SetMagnification(3); //set the resolution of the output image (3 times the current resolution of vtk render window)
  windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
  windowToImageFilter->Update();

  vtkImageData *img = windowToImageFilter->GetOutput();

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  int extent[6];
  img->GetExtent(extent);
  //qDebug() << extent[0] << extent[1] << extent[2] << extent[3] << extent[4] << extent[5];

  int leftPixel = xPos - m_window->width()/2;
  if (leftPixel < extent[0])
    leftPixel = extent[0];

  int rightPixel = xPos + m_window->width()/2;
  if (rightPixel > extent[1])
    rightPixel = extent[1];

  int topPixel = yPos - m_window->height()/2;
  if (topPixel < extent[2])
    rightPixel = extent[2];

  int bottomPixel = yPos + m_window->height()/2;
  if (bottomPixel > extent[3])
    rightPixel = extent[3];

  QPoint bestPixel = QPoint(xPos, yPos);
  unsigned char * pixel;
  unsigned char pixelValue;
  unsigned char bestValue;

  pixel = ((unsigned char *)img->GetScalarPointer(xPos, yPos,0));
  bestValue = abs(pixel[0]-m_bestPixel);

  //qDebug() << "EspINA::BestPixelSelector: Scalar componets:" <<img->GetNumberOfScalarComponents();
  //TODO: Use iterators
  for (int x = leftPixel; x <= rightPixel; x++)
  {
    for (int y = topPixel; y <= bottomPixel; y++)
    {
      pixel = ((unsigned char *)img->GetScalarPointer(x,y,0));
      pixelValue = abs(pixel[0] - m_bestPixel);
      if (pixelValue < bestValue)
      {
        bestValue = pixelValue;
        bestPixel = QPoint(x,y);
      }
      else
        if (pixelValue == bestValue &&	quadDist(xPos,yPos,x,y) < quadDist(xPos,yPos,bestPixel.x(),bestPixel.y()))
        {
          bestValue = pixelValue;
          bestPixel = QPoint(x,y);
        }
      //qDebug() << "Pixel(" << x << "," << y<< ") value :" << pixel[0] << pixel[1] << pixel[2];
    }
  }

  //qDebug() << "EspINA::BestPixelSelector: Best Pixel(" << bestPixel.x() << "," << bestPixel.y()
  //<< ") value :" << bestValue;

  DisplayRegionList regions;
  QPolygon singlePixel;
  singlePixel << bestPixel;
  regions << singlePixel;

  PickList pickList = view->pick(m_filters, regions);

  emit itemsPicked(pickList);
}
