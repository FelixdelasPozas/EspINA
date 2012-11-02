#include "PixelSelector.h"

#include <EspinaRenderView.h>
#include <common/tools/PickableItem.h>
#include <common/EspinaTypes.h>

#include <QDebug>
#include <QMouseEvent>
#include <QWidget>
#include <QSize>

#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkMath.h>

#include <itkImageRegionConstIterator.h>

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
bool PixelSelector::filterEvent(QEvent* e, EspinaRenderView* view)
{
  // If succesor didn't abort the filtering, apply its own filtering
  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      onMouseDown(me->pos(), view);
      return true;
    }
  }
  else if (e->type() == QEvent::MouseMove)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    onMouseMove(me->pos(), view);
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
BestPixelSelector::BestPixelSelector()
: m_window(new QSize(14,14))
, m_bestPixel  (0)
{}

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(const QPoint& pos, EspinaRenderView* view)
{
  DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  PickList pickList = view->pick(m_filters, regions);
  if ((pickList.first().second->type() != ModelItem::CHANNEL) || pickList.empty())
    return;

  EspinaVolume *channel = pickList.first().second->itkVolume();
  double pickedPoint[3];
  pickList.first().first->GetPoint(0, pickedPoint);

  EspinaVolume::SpacingType spacing = channel->GetSpacing();
  double bounds[6];
  view->previewBounds(bounds);

  int extent[6] = { bounds[0]/spacing[0],
                    bounds[1]/spacing[0],
                    bounds[2]/spacing[1],
                    bounds[3]/spacing[1],
                    bounds[4]/spacing[2],
                    bounds[5]/spacing[2] };

  // limit extent to defined QSize
  PickList tempPickList;

  //limiting left extent
  singlePixel.clear();
  singlePixel << QPoint(xPos-(m_window->width()/2), yPos);
  regions.clear();
  regions << singlePixel;
  tempPickList = view->pick(m_filters, regions);
  if (!tempPickList.empty())
  {
    double L_point[3];
    tempPickList.first().first->GetPoint(0, L_point);
    if (L_point[0] < pickedPoint[0])
      extent[0] = L_point[0]/spacing[0];

    if (L_point[1] < pickedPoint[1])
      extent[2] = L_point[1]/spacing[1];

    if (L_point[2] < pickedPoint[2])
      extent[4] = L_point[2]/spacing[2];
  }

  //limiting top extent
  singlePixel.clear();
  singlePixel << QPoint(xPos, yPos-(m_window->height()/2));
  regions.clear();
  regions << singlePixel;
  tempPickList = view->pick(m_filters, regions);
  if (!tempPickList.empty())
  {
    double T_point[3];
    tempPickList.first().first->GetPoint(0, T_point);
    if (T_point[0] > pickedPoint[0])
      extent[1] = T_point[0]/spacing[0];

    if (T_point[1] > pickedPoint[1])
      extent[3] = T_point[1]/spacing[1];

    if (T_point[2] > pickedPoint[2])
      extent[5] = T_point[2]/spacing[2];
  }

  //limiting right extent
  singlePixel.clear();
  singlePixel << QPoint(xPos+(m_window->width()/2), yPos);
  regions.clear();
  regions << singlePixel;
  tempPickList = view->pick(m_filters, regions);
  if (!tempPickList.empty())
  {
    double R_point[3];
    tempPickList.first().first->GetPoint(0, R_point);
    if (R_point[0] > pickedPoint[0])
      extent[1] = R_point[0]/spacing[0];

    if (R_point[1] > pickedPoint[1])
      extent[3] = R_point[1]/spacing[1];

    if (R_point[2] > pickedPoint[2])
      extent[5] = R_point[2]/spacing[2];
  }

  //limiting bottom extent
  singlePixel.clear();
  singlePixel << QPoint(xPos, yPos+(m_window->height()/2));
  regions.clear();
  regions << singlePixel;
  tempPickList = view->pick(m_filters, regions);
  if (!tempPickList.empty())
  {
    double B_point[3];
    tempPickList.first().first->GetPoint(0, B_point);
    if (B_point[0] < pickedPoint[0])
      extent[0] = B_point[0]/spacing[0];

    if (B_point[1] < pickedPoint[1])
      extent[2] = B_point[1]/spacing[1];

    if (B_point[2] < pickedPoint[2])
      extent[4] = B_point[2]/spacing[2];
  }

  EspinaVolume::SizeType regionSize;
  regionSize[0] = extent[1]-extent[0]+1;
  regionSize[1] = extent[3]-extent[2]+1;
  regionSize[2] = extent[5]-extent[4]+1;

  EspinaVolume::IndexType regionIndex;
  regionIndex[0] = extent[0];
  regionIndex[1] = extent[2];
  regionIndex[2] = extent[4];

  EspinaVolume::RegionType region;
  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  itk::ImageRegionConstIterator<EspinaVolume> it(channel,region);
  it.GoToBegin();

  unsigned char pixelValue;
  unsigned char bestValue = abs(it.Get() - m_bestPixel);
  EspinaVolume::IndexType bestPixelIndex = it.GetIndex();
  double bestPoint[3] = { bestPixelIndex[0]*spacing[0], bestPixelIndex[1]*spacing[1], bestPixelIndex[2]*spacing[2] };
  double point[3];

  while (!it.IsAtEnd())
  {
    pixelValue = abs(it.Get()-m_bestPixel);
    if (pixelValue < bestValue)
    {
      bestValue = pixelValue;
      bestPixelIndex = it.GetIndex();
      bestPoint[0] = bestPixelIndex[0]*spacing[0];
      bestPoint[1] = bestPixelIndex[1]*spacing[1];
      bestPoint[2] = bestPixelIndex[2]*spacing[2];
    }
    else
    {
      if (pixelValue == bestValue)
      {
        point[0] = it.GetIndex()[0]*spacing[0];
        point[1] = it.GetIndex()[1]*spacing[1];
        point[2] = it.GetIndex()[2]*spacing[2];

        if (vtkMath::Distance2BetweenPoints(point, pickedPoint) < vtkMath::Distance2BetweenPoints(bestPoint, pickedPoint))
        {
          bestPixelIndex = it.GetIndex();
          bestPoint[0] = bestPixelIndex[0]*spacing[0];
          bestPoint[1] = bestPixelIndex[1]*spacing[1];
          bestPoint[2] = bestPixelIndex[2]*spacing[2];
        }
      }
    }
    ++it;
  }

  // omit getting the QPos of bestPixel, we won't use it later
  pickList.first().first->SetPoint(0, bestPoint[0], bestPoint[1], bestPoint[2]);
  emit itemsPicked(pickList);
}

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseMove(const QPoint &pos, EspinaRenderView *view)
{
  DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  PickList pickList = view->pick(m_filters, regions);

  if ((pickList.first().second->type() != ModelItem::CHANNEL) || pickList.empty())
    return;

  // TODO: continuar con la preview
}
