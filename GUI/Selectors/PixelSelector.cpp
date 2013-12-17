// EspINA
#include "PixelSelector.h"
#include <GUI/View/RenderView.h>
#include <Core/Analysis/Data/VolumetricData.h>

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
void PixelSelector::onMouseDown(const QPoint &pos, RenderView* view)
{
  SelectionList selection = generateSelection(view);

  if (selection.empty() || 0 == selection.first().first->GetNumberOfPoints())
    return;

  emit itemsSelected(selection);
}

//-----------------------------------------------------------------------------
bool PixelSelector::filterEvent(QEvent *e, RenderView *view)
{
  // If successor didn't abort the filtering, apply its own filtering
  if (QEvent::MouseButtonPress == e->type())
  {
    auto me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      onMouseDown(me->pos(), view);
      auto selection = generateSelection(view);
      if (selection.empty())
        return false;

      for(auto item : selection)
      {
        if (0 == item.first->GetNumberOfPoints()) return false;
      }

      return true;
    }
  }

  return EventHandler::filterEvent(e,view);
}

//-----------------------------------------------------------------------------
NmVector3 PixelSelector::getPickPoint(RenderView *view)
{
  SelectionList selection = generateSelection(view);

  if (selection.empty()
  || (selection.first().second->type() != ItemAdapter::Type::CHANNEL)
  || 0 == selection.first().first->GetNumberOfPoints())
    Q_ASSERT(false);

  double point[3];
  selection.first().first->GetPoint(0, point);

  return NmVector3{point[0], point[1], point[2]};
}

//-----------------------------------------------------------------------------
Selector::SelectionList PixelSelector::generateSelection(RenderView *view)
{
  DisplayRegionList regions;
  QPolygon          singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  return view->pick(m_flags, regions);
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
void BestPixelSelector::onMouseDown(const QPoint& pos, RenderView* view)
{
  auto selection = generateSelection(view);
  if   (selection.empty()
    || (ItemAdapter::Type::CHANNEL != selection.first().second->type())
    || 0 == selection.first().first->GetNumberOfPoints())
    return;

  auto point = getPickPoint(view);

  selection.first().first->SetPoint(0, point[0], point[1], point[2]);

  emit itemsSelected(selection);
}

//-----------------------------------------------------------------------------
NmVector3 BestPixelSelector::getPickPoint(RenderView *view)
{
  auto selection = generateSelection(view);
  if   (selection.empty()
    || (ItemAdapter::Type::CHANNEL != selection.first().second->type())
    || 0 == selection.first().first->GetNumberOfPoints())
    Q_ASSERT(false);

  auto selectedItem  = selection.first().second;
  auto channel       = channelPtr(selectedItem);

  double pickedPoint[3];
  selection.first().first->GetPoint(0, pickedPoint);

  auto volume = volumetricData(channel->output());
  auto spacing = volume->spacing();

  Bounds bounds = view->previewBounds();

  int extent[6];
  for (int i = 0; i < 6; i++)
    extent[i] = bounds[i]/spacing[i/2];

  // limit extent to defined QSize
  Selector::SelectionList tmpSelectionList;
  DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  //limiting left extent
  singlePixel.clear();
  singlePixel << QPoint(xPos-(m_window->width()/2), yPos);
  regions.clear();
  regions << singlePixel;
  tmpSelectionList = view->pick(m_flags, regions);
  if (!tmpSelectionList.empty())
  {
    double L_point[3];
    tmpSelectionList.first().first->GetPoint(0, L_point);
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
  tmpSelectionList = view->pick(m_flags, regions);
  if (!tmpSelectionList.empty())
  {
    double T_point[3];
    tmpSelectionList.first().first->GetPoint(0, T_point);
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
  tmpSelectionList = view->pick(m_flags, regions);
  if (!tmpSelectionList.empty())
  {
    double R_point[3];
    tmpSelectionList.first().first->GetPoint(0, R_point);
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
  tmpSelectionList = view->pick(m_flags, regions);
  if (!tmpSelectionList.empty())
  {
    double B_point[3];
    tmpSelectionList.first().first->GetPoint(0, B_point);
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

  Bounds finalBounds = intersection(bounds, channel->bounds());

  Q_ASSERT(finalBounds.areValid());

  itkVolumeType::Pointer preview = volume->itkImage(finalBounds);
  itk::ImageRegionConstIterator<itkVolumeType> it(preview, region);
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

  NmVector3 requestedPoint;

  requestedPoint[0] = bestPoint[0];
  requestedPoint[1] = bestPoint[1];
  requestedPoint[2] = bestPoint[2];

  return requestedPoint;
}
