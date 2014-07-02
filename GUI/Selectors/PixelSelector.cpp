// EspINA
#include "PixelSelector.h"
#include <GUI/View/View3D.h>
#include <GUI/View/View2D.h>
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
#include <vtkCoordinate.h>

// ITK
#include <itkImageRegionConstIterator.h>
#include <itkExtractImageFilter.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
bool PixelSelector::validSelection(Selector::Selection selectedItems)
{
  for(auto item: selectedItems)
  {
    if((item.second->type() == ItemAdapter::Type::SEGMENTATION) && (!m_flags.contains(Selector::SEGMENTATION)))
      return false;

    if((item.second->type() == ItemAdapter::Type::CHANNEL) && (!m_flags.contains(Selector::CHANNEL)))
      return false;

    if((item.second->type() == ItemAdapter::Type::SAMPLE) && (!m_flags.contains(Selector::SAMPLE)))
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
void PixelSelector::onMouseDown(const QPoint &pos, RenderView* view)
{
  Selection selectedItems = generateSelection(view);

  if (selectedItems.empty())
    return;

  emit itemsSelected(selectedItems);
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
      auto selectedItems = generateSelection(view);
      if (selectedItems.empty())
        return false;

      return true;
    }
  }

  return EventHandler::filterEvent(e,view);
}

//-----------------------------------------------------------------------------
NmVector3 PixelSelector::getPickPoint(RenderView *view)
{
  Selection selectedItems = generateSelection(view);

  if (selectedItems.empty())
    Q_ASSERT(false);

  auto voxelBounds = selectedItems.first().first->bounds();
  return NmVector3{(voxelBounds[0]+voxelBounds[1])/2, (voxelBounds[2]+voxelBounds[3])/2, (voxelBounds[4]+voxelBounds[5])/2};
}

//-----------------------------------------------------------------------------
void PixelSelector::transformDisplayToWorld(int x, int y, RenderView *view, NmVector3 &point, bool inSlice) const
{
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToDisplay();
  coords->SetValue(x, y);
  auto displayCoords = coords->GetComputedWorldValue(view->mainRenderer());
  point[0] = displayCoords[0];
  point[1] = displayCoords[1];
  point[2] = displayCoords[2];

  if(inSlice)
  {
    auto view2d = dynamic_cast<View2D *>(view);
    if(view2d == nullptr)
      Q_ASSERT(false);

    auto index = normalCoordinateIndex(view2d->plane());
    point[index] = view2d->crosshairPoint()[index];
  }
}

//-----------------------------------------------------------------------------
Selector::Selection PixelSelector::generateSelection(RenderView *view)
{
  // View3D cannot select with this method.
  View3D* view3d = dynamic_cast<View3D*>(view);
  View2D *view2d = dynamic_cast<View2D*>(view);
  if(view3d != nullptr || view2d == nullptr)
    return Selector::Selection();

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  auto selectedItems = view->select(m_flags, xPos, yPos, m_multiSelection);

  if(!validSelection(selectedItems))
    return Selector::Selection();

  return selectedItems;
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
: PixelSelector()
, m_window     {new QSize(14,14)}
, m_bestPixel  {0}
{}

//-----------------------------------------------------------------------------
BestPixelSelector::~BestPixelSelector()
{
  delete m_window;
}

//-----------------------------------------------------------------------------
NmVector3 BestPixelSelector::getPickPoint(RenderView *view)
{
  auto selectedItems = generateSelection(view);
  if(selectedItems.empty())
    Q_ASSERT(false);

  auto selectedItem  = selectedItems.first().second;
  auto channel       = channelPtr(selectedItem);
  auto channelBounds = channel->bounds();
  auto channelSpacing = channel->output()->spacing();
  auto channelOrigin = channel->position();

  auto pickedBounds = selectedItems.first().first->bounds();
  double pickedPoint[3]{(pickedBounds[0]+pickedBounds[1])/2, (pickedBounds[2]+pickedBounds[3])/2, (pickedBounds[4]+pickedBounds[5])/2};

  // create proportional bounds around the picked point
  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  NmVector3 point;
  transformDisplayToWorld(xPos-(m_window->width()/2), yPos, view, point, true);
  auto boxBounds = boundingBox(pickedBounds, VolumeBounds{Bounds{point}, channelSpacing, channelOrigin});

  transformDisplayToWorld(xPos+(m_window->width()/2), yPos, view, point, true);
  boxBounds = boundingBox(boxBounds, VolumeBounds{Bounds{point}, channelSpacing, channelOrigin});

  transformDisplayToWorld(xPos, yPos-(m_window->height()/2), view, point, true);
  boxBounds = boundingBox(boxBounds, VolumeBounds{Bounds{point}, channelSpacing, channelOrigin});

  transformDisplayToWorld(xPos, yPos+(m_window->height()/2), view, point, true);
  boxBounds = boundingBox(boxBounds, VolumeBounds{Bounds{point}, channelSpacing, channelOrigin});

  auto intersectionBounds = intersection(channelBounds, boxBounds.bounds(), channelSpacing);
  Q_ASSERT(intersectionBounds.areValid());
  auto region = equivalentRegion<itkVolumeType>(channelOrigin, channelSpacing, intersectionBounds);

  itkVolumeType::Pointer preview = volumetricData(channel->output())->itkImage(intersectionBounds);
  itk::ImageRegionConstIterator<itkVolumeType> it(preview, region);
  it.GoToBegin();

  unsigned char bestValue = abs(it.Get() - m_bestPixel);
  itkVolumeType::IndexType bestPixelIndex = it.GetIndex();
  double bestPoint[3] = { bestPixelIndex[0]*channelSpacing[0], bestPixelIndex[1]*channelSpacing[1], bestPixelIndex[2]*channelSpacing[2] };
  double dpoint[3];

  while (!it.IsAtEnd())
  {
    unsigned char pixelValue = abs(it.Get()-m_bestPixel);
    if (pixelValue < bestValue)
    {
      bestValue = pixelValue;
      bestPixelIndex = it.GetIndex();
      bestPoint[0] = bestPixelIndex[0]*channelSpacing[0];
      bestPoint[1] = bestPixelIndex[1]*channelSpacing[1];
      bestPoint[2] = bestPixelIndex[2]*channelSpacing[2];
    }
    else
    {
      if (pixelValue == bestValue)
      {
        dpoint[0] = it.GetIndex()[0]*channelSpacing[0];
        dpoint[1] = it.GetIndex()[1]*channelSpacing[1];
        dpoint[2] = it.GetIndex()[2]*channelSpacing[2];

        if (vtkMath::Distance2BetweenPoints(dpoint, pickedPoint) < vtkMath::Distance2BetweenPoints(bestPoint, pickedPoint))
        {
          bestPixelIndex = it.GetIndex();
          bestPoint[0] = bestPixelIndex[0]*channelSpacing[0];
          bestPoint[1] = bestPixelIndex[1]*channelSpacing[1];
          bestPoint[2] = bestPixelIndex[2]*channelSpacing[2];
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

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(const QPoint &pos, RenderView* view)
{
  auto selectedItems = generateSelection(view);

  if(selectedItems.empty())
    return;

  auto point = getPickPoint(view);
  auto channelAdapter = channelPtr(selectedItems.first().second);
  Q_ASSERT(channelAdapter);
  auto spacing = channelAdapter->output()->spacing();

  BinaryMaskSPtr<unsigned char> bm{ new BinaryMask<unsigned char>{Bounds(NmVector3{point[0], point[1], point[2]}), spacing}};
  BinaryMask<unsigned char>::iterator bmit(bm.get());
  bmit.goToBegin();
  bmit.Set();

  selectedItems.first().first = bm;

  emit itemsSelected(selectedItems);
}
