// EspINA
#include "PixelSelector.h"
#include <Core/EspinaTypes.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/PickableItem.h>
#include <GUI/QtWidget/EspinaRenderView.h>

// Qt
#include <QMouseEvent>
#include <QWidget>
#include <QSize>

// VTK
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkMath.h>

// ITK
#include <itkImageRegionConstIterator.h>
#include <itkExtractImageFilter.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
ISelector::PickList PixelSelector::generatePickList(EspinaRenderView* view)
{
  DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  return view->pick(m_filters, regions);
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseDown(const QPoint &pos, EspinaRenderView* view)
{
  PickList pickList = generatePickList(view);
  if (pickList.empty() || 0 == pickList.first().first->GetNumberOfPoints())
    return;

  emit itemsPicked(pickList);
}

//-----------------------------------------------------------------------------
double *PixelSelector::getPickPoint(EspinaRenderView *view)
{
  PickList pickList = generatePickList(view);
  if (pickList.empty() || (pickList.first().second->type() != EspINA::CHANNEL) || 0 == pickList.first().first->GetNumberOfPoints())
    return NULL;

  double *point = new double[3];
  pickList.first().first->GetPoint(0, point);
  return point;
}

//-----------------------------------------------------------------------------
bool PixelSelector::filterEvent(QEvent* e, EspinaRenderView* view)
{
  // If successor didn't abort the filtering, apply its own filtering
  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      onMouseDown(me->pos(), view);
      PickList pickList = generatePickList(view);
      if (pickList.empty())
        return false;

      foreach(PickedItem item, pickList)
        if (0 == item.first->GetNumberOfPoints())
          return false;

      return true;
    }
  }

  return ISelector::filterEvent(e,view);
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
BestPixelSelector::~BestPixelSelector()
{
  delete m_window;
}

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(const QPoint& pos, EspinaRenderView* view)
{
  PickList pickList = generatePickList(view);
  if (pickList.empty() || (pickList.first().second->type() != EspINA::CHANNEL)  || 0 == pickList.first().first->GetNumberOfPoints())
    return;

  double *point = getPickPoint(view);

  pickList.first().first->SetPoint(0, point[0], point[1], point[2]);
  delete point;
  emit itemsPicked(pickList);
}

//-----------------------------------------------------------------------------
double *BestPixelSelector::getPickPoint(EspinaRenderView *view)
{
  PickList pickList = generatePickList(view);
  if (pickList.empty() || (pickList.first().second->type() != EspINA::CHANNEL) || 0 == pickList.first().first->GetNumberOfPoints())
    return NULL;

  PickableItemPtr pickedItem = pickList.first().second;
  ChannelPtr channel = channelPtr(pickedItem);

  itkVolumeType::Pointer channelVol = channel->volume()->toITK();
  double pickedPoint[3];
  pickList.first().first->GetPoint(0, pickedPoint);

  itkVolumeType::SpacingType spacing = channelVol->GetSpacing();
  double bounds[6];
  view->previewBounds(bounds);

  int extent[6];
  for (int i = 0; i < 6; i++)
    extent[i] = bounds[i]/spacing[i/2];

  // limit extent to defined QSize
  PickList tempPickList;
  DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

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

  itkVolumeType::SizeType regionSize;
  regionSize[0] = extent[1]-extent[0]+1;
  regionSize[1] = extent[3]-extent[2]+1;
  regionSize[2] = extent[5]-extent[4]+1;

  itkVolumeType::IndexType regionIndex;
  regionIndex[0] = extent[0];
  regionIndex[1] = extent[2];
  regionIndex[2] = extent[4];

  itkVolumeType::RegionType region;
  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  itk::ImageRegionConstIterator<itkVolumeType> it(channelVol,region);
  it.GoToBegin();

  unsigned char bestValue = abs(it.Get() - m_bestPixel);
  itkVolumeType::IndexType bestPixelIndex = it.GetIndex();
  double bestPoint[3] = { bestPixelIndex[0]*spacing[0], bestPixelIndex[1]*spacing[1], bestPixelIndex[2]*spacing[2] };
  double point[3];

  while (!it.IsAtEnd())
  {
    unsigned char pixelValue = abs(it.Get()-m_bestPixel);
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

  // NOTE: check that point is deleted later
  double *requestedPoint = new double[3];
  requestedPoint[0] = bestPoint[0];
  requestedPoint[1] = bestPoint[1];
  requestedPoint[2] = bestPoint[2];
  return requestedPoint;
}