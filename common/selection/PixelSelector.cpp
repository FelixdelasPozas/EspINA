#include "PixelSelector.h"

#include <selection/SelectableView.h>

#include <QDebug>

#include <QMouseEvent>
#include <QWidget>
#include <QSize>

#include <pqRenderView.h>
#include <vtkImageData.h>
#include <../Views/vtkSMSliceViewProxy.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>

//-----------------------------------------------------------------------------
void PixelSelector::onMouseDown(const QPoint &pos, SelectableView* view)
{
  ViewRegions regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  MultiSelection msel = view->select(m_filters, regions);

  emit selectionChanged(msel);
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseMove(const QPoint &pos, SelectableView* view)
{
  //Do nothing
  qDebug() << "EspINA::PixelSelector: Mouse Moving: " << pos.x() << pos.y();
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseUp(const QPoint &pos, SelectableView* view)
{
  qDebug() << "EspINA::PixelSelector: Mouse released";
}

//-----------------------------------------------------------------------------
bool PixelSelector::filterEvent(QEvent* e, SelectableView* view)
{
  // If succesor didn't abort the filtering, apply its own filtering
  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      onMouseDown(me->pos(), view);
      return true; // Prevent other elements to filter the event
    }
  }

  return SelectionHandler::filterEvent(e,view);
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
BestPixelSelector::BestPixelSelector(SelectionHandler* succesor)
: PixelSelector(succesor)
, m_window     (new QSize(14,14))
, m_bestPixel  (0)
{}

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(const QPoint& pos, SelectableView* view)
{
  pqRenderViewBase *rw = dynamic_cast<pqRenderViewBase *>(view->view());
  Q_ASSERT(rw);
  vtkImageData *img = rw->captureImage(1);

  int extent[6];
  img->GetExtent(extent);

  int leftPixel = pos.x() - m_window->width()/2;
  if (leftPixel < extent[0])
    leftPixel = extent[0];

  int rightPixel = pos.x() + m_window->width()/2;
  if (rightPixel > extent[1])
    rightPixel = extent[1];

  int topPixel = pos.y() - m_window->height()/2;
  if (topPixel < extent[2])
    rightPixel = extent[2];

  int bottomPixel = pos.y() + m_window->height()/2;
  if (bottomPixel > extent[3])
    rightPixel = extent[3];

  QPoint bestPixel = pos;
  unsigned char * pixel;
  unsigned char pixelValue;
  unsigned char bestValue;

  pixel = ((unsigned char *)img->GetScalarPointer(pos.x(),pos.y(),0));
  bestValue = abs(pixel[0]-m_bestPixel);

  //qDebug() << "EspINA::BestPixelSelector: Scalar componets:" <<img->GetNumberOfScalarComponents();

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
      } else if (pixelValue == bestValue &&
	quadDist(pos.x(),pos.y(),x,y) < quadDist(pos.x(),pos.y(),bestPixel.x(),bestPixel.y()))
      {
	bestValue = pixelValue;
	bestPixel = QPoint(x,y);
      }
      //qDebug() << "Pixel(" << x << "," << y<< ") value :" << pixel[0] << pixel[1] << pixel[2];
    }
  }

  qDebug() << "EspINA::BestPixelSelector: Best Pixel(" << bestPixel.x() << "," << bestPixel.y()
  << ") value :" << bestValue;

  img->Delete();

  ViewRegions regions;
  QPolygon singlePixel;
  singlePixel << bestPixel;
  regions << singlePixel;

//   view->setSelection(m_filters, regions);
}
