/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BrushSelector.h"

// EspINA
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Spatial.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/Volumetric/StreamedVolume.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/CategoryAdapter.h>

#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>
#include <Support/ViewManager.h>

// Qt
#include <QMouseEvent>
#include <QPainter>

// VTK
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkCoordinate.h>
#include <vtkSphere.h>
#include <vtkImageMapToColors.h>
#include <vtkImageData.h>
#include <vtkInformation.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
BrushSelector::BrushSelector(ViewManagerSPtr vm)
: m_viewManager(vm)
, m_referenceItem(nullptr)
, m_displayRadius(-1)
, m_borderColor(Qt::blue)
, m_brushColor(Qt::blue)
, m_brushImage(nullptr)
, m_tracking(false)
, m_stroke(vtkSmartPointer<vtkPoints>::New())
, m_plane(Plane::XY)
, m_radius(-1)
, m_lut(nullptr)
, m_preview(nullptr)
, m_actor(nullptr)
, m_drawing(true)
, m_segmentation(nullptr)
{
  memset(m_viewSize, 0, 2*sizeof(int));
  memset(m_LL, 0, 3*sizeof(double));
  memset(m_UR, 0, 3*sizeof(double));
  memset(m_worldSize, 0, 2*sizeof(double));

  buildCursor();
}

//-----------------------------------------------------------------------------
BrushSelector::~BrushSelector()
{
  if (m_brushImage)
  {
    delete m_brushImage;
    m_brushImage = nullptr;
  }
}

//-----------------------------------------------------------------------------
bool BrushSelector::filterEvent(QEvent* e, RenderView* view)
{
  if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && m_tracking)
    {
      startStroke(m_lastDot, view);
      return true;
    }
  }

  if (!m_tracking && QEvent::MouseButtonPress == e->type())
  {
    QMouseEvent *me = static_cast<QMouseEvent *>(e);
    if (me->button() == Qt::LeftButton)
    {
      m_tracking = true;
      startStroke(me->pos(), view);
      return true;
    }
  }
  else
    if (m_tracking && QEvent::MouseMove == e->type())
    {
      QMouseEvent *me = static_cast<QMouseEvent*>(e);
      updateStroke(me->pos(), view);
      return true;
    }
    else
      if (m_tracking && QEvent::MouseButtonRelease == e->type())
      {
        m_tracking = false;
        stopStroke(view);
        return true;
      }

  return false;
}

//-----------------------------------------------------------------------------
void BrushSelector::setRadius(int radius)
{
  if (radius <= 0)
    m_displayRadius = 1;
  else if (radius > MAX_RADIUS)
    m_displayRadius = MAX_RADIUS;
  else
    m_displayRadius = radius;

  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setBorderColor(QColor color)
{
  m_borderColor = color;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setBrushColor(QColor color)
{
  m_brushColor = color;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setReferenceItem(ViewItemAdapterPtr item)
{
  m_referenceItem = item;
  NmVector3 spacing;

  DefaultVolumetricDataSPtr volume;
  if (ItemAdapter::Type::SEGMENTATION == item->type())
  {
    volume = std::dynamic_pointer_cast<SparseVolume<itkVolumeType>>(item->output()->data(VolumetricData<itkVolumeType>::TYPE));
  }
  else
    if (ItemAdapter::Type::CHANNEL == item->type())
    {
      volume = std::dynamic_pointer_cast<StreamedVolume<itkVolumeType>>(item->output()->data(VolumetricData<itkVolumeType>::TYPE));
    }

  Q_ASSERT(volume);
  spacing = volume->spacing();

  m_spacing[0] = spacing[0];
  m_spacing[1] = spacing[1];
  m_spacing[2] = spacing[2];
}

//-----------------------------------------------------------------------------
itkVolumeType::SpacingType BrushSelector::referenceSpacing() const
{
  return m_spacing;
}


//-----------------------------------------------------------------------------
void BrushSelector::setBrushImage(QImage &image)
{
  if (m_brushImage)
  {
    delete m_brushImage;
    m_brushImage = nullptr;
  }

  if (!image.isNull())
  {
    m_brushImage = new QImage(image);
  }

  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::buildCursor()
{
  if (m_displayRadius == -1)
  {
    // Selector cursor radius not initialized. Using default value 20.
    m_displayRadius = 20;
  }

  int width = 2*m_displayRadius;

  QPixmap pix(width, width);
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setBrush(QBrush(m_brushColor));
  p.setPen(QPen(m_borderColor));
  p.drawEllipse(0, 0, width-1, width-1);

  if (m_brushImage)
    p.drawImage(QPoint(m_displayRadius/2,m_displayRadius/2), m_brushImage->scaledToWidth(m_displayRadius));

  Q_ASSERT(pix.hasAlpha());

  m_cursor = QCursor(pix);
}

//-----------------------------------------------------------------------------
void BrushSelector::createBrush(NmVector3 &center, QPoint pos)
{
  int H = (Plane::YZ == m_plane) ? 2 : 0;
  int V = (Plane::XZ == m_plane) ? 2 : 1;

  double wPos[3];
  int planeIndex = normalCoordinateIndex(m_plane);
  wPos[planeIndex] = m_pBounds[2*planeIndex];
  wPos[H] = m_LL[H] + pos.x()*m_worldSize[0]/m_viewSize[0];
  wPos[V] = m_UR[V] + pos.y()*m_worldSize[1]/m_viewSize[1];

  for(int i=0; i < 3; i++)
    center[i] = int(wPos[i]/m_spacing[i]+0.5)*m_spacing[i];
}


//-----------------------------------------------------------------------------
bool BrushSelector::validStroke(NmVector3 &center)
{
  Bounds brushBounds;

  switch(m_plane)
  {
    case Plane::XY:
      brushBounds[0] = center[0] - m_radius;
      brushBounds[1] = center[0] + m_radius;
      brushBounds[2] = center[1] - m_radius;
      brushBounds[3] = center[1] + m_radius;
      brushBounds[4] = m_pBounds[4];
      brushBounds[5] = m_pBounds[5];
      break;
    case Plane::XZ:
      brushBounds[0] = center[0] - m_radius;
      brushBounds[1] = center[0] + m_radius;
      brushBounds[2] = m_pBounds[2];
      brushBounds[3] = m_pBounds[3];
      brushBounds[4] = center[2] - m_radius;
      brushBounds[5] = center[2] + m_radius;
      break;
    case Plane::YZ:
      brushBounds[0] = m_pBounds[0];
      brushBounds[1] = m_pBounds[1];
      brushBounds[2] = center[1] - m_radius;
      brushBounds[3] = center[1] + m_radius;
      brushBounds[4] = center[2] - m_radius;
      brushBounds[5] = center[2] + m_radius;
      break;
    default:
      break;
  }
  if (!brushBounds.areValid())
    return false;

  if (!m_drawing)
  {
    DefaultVolumetricDataSPtr volume = std::dynamic_pointer_cast<SparseVolume<itkVolumeType>>(m_referenceItem->output()->data(VolumetricData<itkVolumeType>::TYPE));
    return intersect(brushBounds, volume->bounds());
  }

  return intersect(m_pBounds, brushBounds);
}

//-----------------------------------------------------------------------------
void BrushSelector::startStroke(QPoint pos, RenderView* view)
{
  m_pBounds = view->previewBounds(false);

  if (m_pBounds[0] == m_pBounds[1])
    m_plane = Plane::YZ;
  else if (m_pBounds[2] == m_pBounds[3])
    m_plane = Plane::XZ;
  else if (m_pBounds[4] == m_pBounds[5])
    m_plane = Plane::XY;
  else
    return; // Current implementation only works in 2D views

  memcpy(m_viewSize, view->renderWindow()->GetSize(), 2*sizeof(int));

  // Display bounds in world coordinates
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  vtkRenderer *renderer = view->mainRenderer();
  coords->SetViewport(renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  coords->SetValue(0, 0); //LL
  memcpy(m_LL,coords->GetComputedWorldValue(renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(m_UR,coords->GetComputedWorldValue(renderer),3*sizeof(double));

  int H = (Plane::YZ == m_plane) ? 2 : 0;
  int V = (Plane::XZ == m_plane) ? 2 : 1;

  m_worldSize[0] = fabs(m_UR[H] - m_LL[H]);
  m_worldSize[1] = fabs(m_UR[V] - m_LL[V]);

  m_radius = m_displayRadius*m_worldSize[0]/m_viewSize[0];

  NmVector3 center;
  createBrush(center, pos);

  startPreview(view);

  if (validStroke(center))
  {
    double brush[3]{center[0], center[1], center[2]};
    m_lastDot = pos;
    m_stroke->InsertNextPoint(brush);
    updatePreview(center, view);
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::updateStroke(QPoint pos, RenderView* view)
{
  if (m_stroke->GetNumberOfPoints() > 0 && QLineF(m_lastDot, pos).length() < m_displayRadius/2.0)
    return;

  NmVector3 center;
  createBrush(center, pos);

  if (validStroke(center))
  {
    double brush[3]{center[0], center[1], center[2]};
    m_lastDot = pos;
    m_stroke->InsertNextPoint(brush);
    updatePreview(center, view);
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::stopStroke(RenderView* view)
{
  stopPreview(view);

  if (m_stroke->GetNumberOfPoints() > 0)
    emit stroke(m_referenceItem, m_stroke, m_radius, m_plane);

  m_stroke->Reset();

  view->updateView();
}

//-----------------------------------------------------------------------------
void BrushSelector::startPreview(RenderView* view)
{
  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetNumberOfTableValues(2);
  m_lut->Build();
  m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_lut->SetTableValue(1, m_brushColor.redF(), m_brushColor.greenF(), m_brushColor.blueF(), 0.8);
  m_lut->Modified();

  int extent[6];
  for (int i = 0; i < 3; ++i)
  {
    extent[2 * i]       = m_pBounds[2 * i]       / m_spacing[i];
    extent[(2 * i) + 1] = m_pBounds[(2 * i) + 1] / m_spacing[i];
  }

  m_preview = vtkSmartPointer<vtkImageData>::New();
  m_preview->SetOrigin(0, 0, 0);
  vtkInformation *info = m_preview->GetInformation();
  m_preview->SetExtent(extent);
  m_preview->SetSpacing(m_spacing[0], m_spacing[1], m_spacing[2]);
  vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
  vtkImageData::SetNumberOfScalarComponents(1, info);
  m_preview->SetInformation(info);
  m_preview->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  m_preview->Modified();
  memset(m_preview->GetScalarPointer(), 0, m_preview->GetNumberOfPoints());

  // if erasing hide seg and copy contents of slice to preview actor
  if (!m_drawing)
  {
    SparseVolumeSPtr volume = std::dynamic_pointer_cast<SparseVolume<itkVolumeType>>(m_referenceItem->output()->data(VolumetricData<itkVolumeType>::TYPE));

    for(auto prototype: m_segmentation->representations())
      prototype->setActive(false, view);

    Bounds bounds{volume->bounds()};
    int segExtent[6]{ static_cast<int>(bounds[0]/m_spacing[0]), static_cast<int>(bounds[1]/m_spacing[0]),
                      static_cast<int>(bounds[2]/m_spacing[1]), static_cast<int>(bounds[3]/m_spacing[1]),
                      static_cast<int>(bounds[4]/m_spacing[2]), static_cast<int>(bounds[5]/m_spacing[2])};

    // minimize voxel copy, only fill the part of the preview that has
    // segmentation voxels.
    if (m_plane != Plane::YZ)
    {
      segExtent[0] = (extent[0] > segExtent[0]) ? extent[0] : segExtent[0];
      segExtent[1] = (extent[1] < segExtent[1]) ? extent[1] : segExtent[1];
    }
    else
      segExtent[0] = segExtent[1] = extent[0];

    if (m_plane != Plane::XZ)
    {
      segExtent[2] = (extent[2] > segExtent[2]) ? extent[2] : segExtent[2];
      segExtent[3] = (extent[3] < segExtent[3]) ? extent[3] : segExtent[3];
    }
    else
      segExtent[2] = segExtent[3] = extent[2];

    if (m_plane != Plane::XY)
    {
      segExtent[4] = (extent[4] > segExtent[4]) ? extent[4] : segExtent[4];
      segExtent[5] = (extent[5] < segExtent[5]) ? extent[5] : segExtent[5];
    }
    else
      segExtent[4] = segExtent[5] = extent[4];

    itkVolumeType::IndexType index;
    for (int x = segExtent[0]; x <= segExtent[1]; ++x)
      for (int y = segExtent[2]; y <= segExtent[3]; ++y)
        for (int z = segExtent[4]; z <= segExtent[5]; ++z)
        {
          index[0] = x;
          index[1] = y;
          index[2] = z;

          Bounds pixelBounds{x*m_spacing[0],x*m_spacing[0],y*m_spacing[1],y*m_spacing[1],z*m_spacing[2],z*m_spacing[2]};
          unsigned char *previewPixel = reinterpret_cast<unsigned char *>(m_preview->GetScalarPointer(x,y,z));
          *previewPixel = volume->itkImage(pixelBounds)->GetPixel(index);
        }
  }
  m_preview->Modified();

  vtkSmartPointer<vtkImageMapToColors> mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  mapToColors->SetInputData(m_preview);
  mapToColors->SetLookupTable(m_lut);
  mapToColors->SetNumberOfThreads(1);
  mapToColors->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetPickable(false);
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(mapToColors->GetOutputPort());
  m_actor->Update();

  view->addActor(m_actor);
  view->updateView();
}

//-----------------------------------------------------------------------------
void BrushSelector::updatePreview(NmVector3 center, RenderView* view)
{
  // fixes crashes when the user releases or presses the control key
  // in the middle of a stroke (that is, without unpressing the mouse
  // button).
  if (!m_preview)
    startPreview(view);

  Bounds brushBounds;

  switch(m_plane)
  {
    case Plane::XY:
      brushBounds[0] = center[0] - m_radius;
      brushBounds[1] = center[0] + m_radius;
      brushBounds[2] = center[1] - m_radius;
      brushBounds[3] = center[1] + m_radius;
      brushBounds[4] = brushBounds[5] = m_pBounds[4];
      break;
    case Plane::XZ:
      brushBounds[0] = center[0] - m_radius;
      brushBounds[1] = center[0] + m_radius;
      brushBounds[2] = brushBounds[3] = m_pBounds[2];
      brushBounds[4] = center[2] - m_radius;
      brushBounds[5] = center[2] + m_radius;
      break;
    case Plane::YZ:
      brushBounds[0] = brushBounds[1] = m_pBounds[0];
      brushBounds[2] = center[1] - m_radius;
      brushBounds[3] = center[1] + m_radius;
      brushBounds[4] = center[2] - m_radius;
      brushBounds[5] = center[2] + m_radius;
      break;
    default:
      break;
  }

  if (intersect(brushBounds, m_pBounds))
  {
    Bounds updateBounds = intersection(m_pBounds, brushBounds);
    double brush[3]{center[0], center[1], center[2]};

    double r2 = m_radius*m_radius;
    switch(m_plane)
    {
      case Plane::XY:
        for (int x = updateBounds[0]/m_spacing[0]; x <= updateBounds[1]/m_spacing[0]; x++)
          for (int y = updateBounds[2]/m_spacing[1]; y <= updateBounds[3]/m_spacing[1]; y++)
        {
          double pixel[3] = {x*m_spacing[0], y*m_spacing[1], m_pBounds[4]};
          if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,y,m_pBounds[4]/m_spacing[2]));
            *pixel = (m_drawing ? 1 : 0);
          }
        }
        break;
      case Plane::XZ:
        for (int x = updateBounds[0]/m_spacing[0]; x <= updateBounds[1]/m_spacing[0]; x++)
          for (int z = updateBounds[4]/m_spacing[2]; z <= updateBounds[5]/m_spacing[2]; z++)
        {
          double pixel[3] = {x*m_spacing[0], m_pBounds[2], z*m_spacing[2]};
          if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,m_pBounds[2]/m_spacing[1],z));
            *pixel = (m_drawing ? 1 : 0);
          }
        }
        break;
      case Plane::YZ:
        for (int y = updateBounds[2]/m_spacing[1]; y <= updateBounds[3]/m_spacing[1]; y++)
          for (int z = updateBounds[4]/m_spacing[2]; z <= updateBounds[5]/m_spacing[2]; z++)
        {
          double pixel[3] = {m_pBounds[0], y*m_spacing[1], z*m_spacing[2]};
          if (vtkMath::Distance2BetweenPoints(brush,pixel) < r2)
          {
            unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(m_pBounds[0]/m_spacing[0],y,z));
            *pixel = (m_drawing ? 1 : 0);
          }
        }
        break;
      default:
        break;
    }

    m_preview->Modified();
    view->updateView();
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::stopPreview(RenderView* view)
{
  view->removeActor(m_actor);
  m_lut = nullptr;
  m_preview = nullptr;
  m_actor = nullptr;

  if (!m_drawing && m_segmentation != nullptr)
    for(auto prototype: m_segmentation->representations())
      prototype->setActive(true, view);
}

//-----------------------------------------------------------------------------
void BrushSelector::DrawingOn(RenderView *view)
{
  if ((m_stroke->GetNumberOfPoints() > 0) && view != nullptr)
    stopStroke(view);

  m_drawing = true;
  m_segmentation = nullptr;
}

//-----------------------------------------------------------------------------
void BrushSelector::DrawingOff(RenderView *view, SegmentationAdapterPtr segmentation)
{
  if (m_stroke->GetNumberOfPoints() > 0)
    stopStroke(view);

  m_drawing = false;
  m_segmentation = segmentation;
}

//-----------------------------------------------------------------------------
QColor BrushSelector::getBrushColor()
{
  return m_brushColor;
}

//-----------------------------------------------------------------------------
BinaryMaskSPtr<unsigned char> BrushSelector::voxelSelectionMask() const
{
  Bounds strokeBounds;
  for (auto brush : m_brushes)
  {
    if (!strokeBounds.areValid())
      strokeBounds = brush.second;
    else
      strokeBounds = boundingBox(strokeBounds, brush.second);
  }

  NmVector3 spacing{ m_spacing[0], m_spacing[1], m_spacing[2] };
  BinaryMaskPtr<unsigned char> mask = new BinaryMask<unsigned char>(strokeBounds, spacing);
  for (auto brush : m_brushes)
  {
    BinaryMask<unsigned char>::region_iterator it(mask, brush.second);
    while (!it.isAtEnd())
    {
      auto index = it.getIndex();
      if (brush.first->FunctionValue(index.x * m_spacing[0], index.y * m_spacing[1], index.z * m_spacing[2]) <= 0)
        it.Set();
      ++it;
    }
  }

  return BinaryMaskSPtr<unsigned char>(mask);
}

//-----------------------------------------------------------------------------
void BrushSelector::initBrush()
{
  QImage noSeg = QImage(":/espina/add.svg");
  QImage hasSeg = QImage();
  QImage image;
  QColor color, borderColor;
  ViewItemAdapterPtr item = nullptr;

  SelectionSPtr selection = m_viewManager->selection();
  SegmentationAdapterList segs = selection->segmentations();
  if (segs.size() == 1)
  {
    item = segs.first();
    color = segs.first()->category()->color();
    image = hasSeg;
    borderColor = QColor(Qt::green);
  }
  else
  {
    item = m_viewManager->activeChannel();
    color = m_viewManager->activeCategory()->color();
    image = noSeg;
    borderColor = QColor(Qt::blue);
  }

  setBrushColor(color);
  setBrushImage(image);
  setBorderColor(borderColor);
  setReferenceItem(item);

  m_drawing = true;
  DrawingOn(nullptr);
}
