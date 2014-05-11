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
void PixelSelector::onMouseDown(const QPoint &pos, RenderView* view)
{
  Selection selection = generateSelection(view);

  if (selection.empty() || (ItemAdapter::Type::CHANNEL != selection.first().second->type()))
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

      return true;
    }
  }

  return EventHandler::filterEvent(e,view);
}

//-----------------------------------------------------------------------------
NmVector3 PixelSelector::getPickPoint(RenderView *view)
{
  Selection selection = generateSelection(view);

  if (selection.empty() || (selection.first().second->type() != ItemAdapter::Type::CHANNEL))
    Q_ASSERT(false);

  auto voxelBounds = selection.first().first->bounds();
  return NmVector3{(voxelBounds[0]+voxelBounds[1])/2, (voxelBounds[2]+voxelBounds[3])/2, (voxelBounds[4]+voxelBounds[5])/2};
}

//-----------------------------------------------------------------------------
Selector::Selection PixelSelector::generateSelection(RenderView *view)
{
  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  // View3D cannot select with this method.
  View3D* view3d = dynamic_cast<View3D*>(view);
  View2D *view2d = dynamic_cast<View2D*>(view);
  if(view3d != nullptr || view2d == nullptr)
    return Selector::Selection();

  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetCoordinateSystemToDisplay();
  coords->SetValue(xPos, yPos);
  auto point = coords->GetComputedWorldValue(view->mainRenderer());
  auto index = normalCoordinateIndex(view2d->plane());
  point[index] = view2d->crosshairPoint()[index];

  Selector::SelectionFlags flags;
  flags.insert(Selector::CHANNEL);

  BinaryMaskSPtr<unsigned char> bm{ new BinaryMask<unsigned char>{Bounds(NmVector3{point[0], point[1], point[2]}), m_resolution}};
  BinaryMask<unsigned char>::iterator bmit(bm.get());
  bmit.goToBegin();
  bmit.Set();

  return view->select(flags, bm);
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
BestPixelSelector::BestPixelSelector(NmVector3 resolution)
: PixelSelector{resolution}
, m_window(new QSize(14,14))
, m_bestPixel  (0)
{}

//-----------------------------------------------------------------------------
BestPixelSelector::~BestPixelSelector()
{
  delete m_window;
}

//-----------------------------------------------------------------------------
NmVector3 BestPixelSelector::getPickPoint(RenderView *view)
{
  auto selection = generateSelection(view);
  if   (selection.empty() || (ItemAdapter::Type::CHANNEL != selection.first().second->type()))
    Q_ASSERT(false);

  auto selectedItem  = selection.first().second;
  auto channel       = channelPtr(selectedItem);

  auto itemBounds = selection.first().first->bounds();
  double pickedPoint[3]{(itemBounds[0]+itemBounds[1])/2, (itemBounds[2]+itemBounds[3])/2, (itemBounds[4]+itemBounds[5])/2};

  auto volume = volumetricData(channel->output());
  auto spacing = volume->spacing();

  auto bounds = view->previewBounds();

  int extent[6];
  for (int i = 0; i < 6; i++)
    extent[i] = bounds[i]/spacing[i/2];

  // limit extent to defined QSize
  Selector::Selection tmpSelectionList;
  Selector::SelectionFlags tmpFlags;
  tmpFlags.insert(Selector::CHANNEL);

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  //limiting left extent
  tmpSelectionList = view->select(tmpFlags, xPos-(m_window->width()/2), yPos);
  if (!tmpSelectionList.empty())
  {
    auto selectionBounds = tmpSelectionList.first().first->bounds();
    double L_point[3]{ (selectionBounds[0]+selectionBounds[1])/2, (selectionBounds[2]+selectionBounds[3])/2, (selectionBounds[4]+selectionBounds[5])/2};
    if (L_point[0] < pickedPoint[0])
      extent[0] = L_point[0]/spacing[0];

    if (L_point[1] < pickedPoint[1])
      extent[2] = L_point[1]/spacing[1];

    if (L_point[2] < pickedPoint[2])
      extent[4] = L_point[2]/spacing[2];
  }

  //limiting top extent
  tmpSelectionList = view->select(tmpFlags, xPos, yPos-(m_window->height()/2));
  if (!tmpSelectionList.empty())
  {
    auto selectionBounds = tmpSelectionList.first().first->bounds();
    double T_point[3]{ (selectionBounds[0]+selectionBounds[1])/2, (selectionBounds[2]+selectionBounds[3])/2, (selectionBounds[4]+selectionBounds[5])/2};
    if (T_point[0] > pickedPoint[0])
      extent[1] = T_point[0]/spacing[0];

    if (T_point[1] > pickedPoint[1])
      extent[3] = T_point[1]/spacing[1];

    if (T_point[2] > pickedPoint[2])
      extent[5] = T_point[2]/spacing[2];
  }

  //limiting right extent
  tmpSelectionList = view->select(tmpFlags, xPos+(m_window->width()/2), yPos);
  if (!tmpSelectionList.empty())
  {
    auto selectionBounds = tmpSelectionList.first().first->bounds();
    double R_point[3]{ (selectionBounds[0]+selectionBounds[1])/2, (selectionBounds[2]+selectionBounds[3])/2, (selectionBounds[4]+selectionBounds[5])/2};
    if (R_point[0] > pickedPoint[0])
      extent[1] = R_point[0]/spacing[0];

    if (R_point[1] > pickedPoint[1])
      extent[3] = R_point[1]/spacing[1];

    if (R_point[2] > pickedPoint[2])
      extent[5] = R_point[2]/spacing[2];
  }

  //limiting bottom extent
  tmpSelectionList = view->select(tmpFlags, xPos, yPos+(m_window->height()/2));
  if (!tmpSelectionList.empty())
  {
    auto selectionBounds = tmpSelectionList.first().first->bounds();
    double B_point[3]{ (selectionBounds[0]+selectionBounds[1])/2, (selectionBounds[2]+selectionBounds[3])/2, (selectionBounds[4]+selectionBounds[5])/2};
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

//-----------------------------------------------------------------------------
void BestPixelSelector::onMouseDown(const QPoint &pos, RenderView* view)
{
  auto selection = generateSelection(view);

  if(selection.empty() ||
     ItemAdapter::Type::CHANNEL != selection.first().second->type() ||
     selection.first().first->numberOfVoxels() != 1)
  {
    return;
  }

  auto point = getPickPoint(view);

  BinaryMaskSPtr<unsigned char> bm{ new BinaryMask<unsigned char>{Bounds(NmVector3{point[0], point[1], point[2]}), m_resolution}};
  BinaryMask<unsigned char>::iterator bmit(bm.get());
  bmit.goToBegin();
  bmit.Set();

  selection.first().first = bm;

  emit itemsSelected(selection);
}
