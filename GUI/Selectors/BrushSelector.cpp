/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/CategoryAdapter.h>

#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>
#include <GUI/View/Selection.h>
#include <Support/ViewManager.h>

// Qt
#include <QApplication>
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
#include <vtkPoints.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
BrushSelector::BrushSelector()
: m_item   {nullptr}
, m_displayRadius   {15}
, m_borderPaintColor{Qt::blue}
, m_borderEraseColor{Qt::red}
, m_brushColor      {Qt::blue}
, m_brushOpacity    {50}
, m_brushImage      {nullptr}
, m_plane           {Plane::UNDEFINED}
, m_radius          {-1}
, m_lut             {nullptr}
, m_preview         {nullptr}
, m_mapToColors     {nullptr}
, m_actor           {nullptr}
, m_previewBounds   {Bounds()}
, m_drawing         {true}
, m_lastUpdateBounds{Bounds()}
, m_tracking        {false}
, m_previewView     {nullptr}
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

  if (m_previewView)
    stopPreview(m_previewView);
}

//-----------------------------------------------------------------------------
bool BrushSelector::filterEvent(QEvent* e, RenderView* view)
{
  QKeyEvent *ke = nullptr;
  QMouseEvent *me = nullptr;

  switch(e->type())
  {
    case QEvent::Leave:
      {
        if (m_tracking)
        {
          stopStroke(view);
          m_tracking = false;
        }
        m_drawing = true;

        if (m_previewView != nullptr)
          stopPreview(view);

        emit drawingModeChanged(m_drawing);
      }
      break;
    case QEvent::Enter:
      {
        m_drawing = !ShiftKeyIsDown();
        buildCursor();
        view->setCursor(m_cursor);

        if (!m_drawing)
          startPreview(view);

        emit drawingModeChanged(m_drawing);
      }
      break;
    case QEvent::KeyPress:
      {
        ke = static_cast<QKeyEvent *>(e);
        if ((ke->key() == Qt::Key_Shift) && !m_tracking && m_item && (m_item->type() == ViewItemAdapter::Type::SEGMENTATION))
        {
          m_drawing = false;
          buildCursor();
          view->setCursor(m_cursor);
          startPreview(view);

          emit drawingModeChanged(m_drawing);
          return true;
        }
      }
      break;
    case QEvent::KeyRelease:
      {
        ke = static_cast<QKeyEvent *>(e);
        if ((ke->key() == Qt::Key_Shift) && !m_tracking && !m_drawing)
        {
          stopPreview(view);
          m_drawing = true;
          buildCursor();
          view->setCursor(m_cursor);

          emit drawingModeChanged(m_drawing);
          return true;
        }
      }
      break;
    case QEvent::MouseButtonPress:
      {
        // the crtl check is to avoid interference with View2D ctrl+click
        if (!m_tracking && !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
        {
          me = static_cast<QMouseEvent *>(e);
          if (me->button() == Qt::LeftButton)
          {
            m_tracking = true;
            startStroke(me->pos(), view);
            return true;
          }
        }
      }
      break;
    case QEvent::MouseMove:
      {
        if (m_tracking)
        {
          me = static_cast<QMouseEvent*>(e);
          updateStroke(me->pos(), view);
          return true;
        }
      }
      break;
    case QEvent::MouseButtonRelease:
      {
        if (m_tracking)
        {
          m_tracking = false;
          stopStroke(view);
          return true;
        }
      }
      break;
    case QEvent::Wheel:
      {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
        {
          QWheelEvent *we = static_cast<QWheelEvent *>(e);
          int numSteps = we->delta() / 8 / 15;  //Refer to QWheelEvent doc.
          m_displayRadius -= numSteps;
          setRadius(m_displayRadius);
          emit radiusChanged(m_displayRadius);
          view->setCursor(m_cursor);
          return true;
        }
      }
      break;
    default:
      break;
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
void BrushSelector::setBorderPaintColor(QColor color)
{
  m_borderPaintColor = color;

  if(m_drawing)
    buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setBorderEraseColor(QColor color)
{
  m_borderEraseColor = color;

  if(!m_drawing)
    buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setBrushColor(QColor color)
{
  m_brushColor = color;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setBrushOpacity(int value)
{
  m_brushOpacity = value;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setReferenceItem(ViewItemAdapterPtr item)
{
  if (!item)
  {
    m_item = nullptr;
    m_origin = NmVector3();
    m_spacing[0] = m_spacing[1] = m_spacing[2] = 0;
    return;
  }

  m_item = item;
  NmVector3 spacing;

  auto volume = volumetricData(item->output());
  Q_ASSERT(volume);
  spacing = volume->spacing();
  m_spacing[0] = spacing[0];
  m_spacing[1] = spacing[1];
  m_spacing[2] = spacing[2];
  m_origin = volume->origin();
}

//-----------------------------------------------------------------------------
itkVolumeType::SpacingType BrushSelector::referenceSpacing() const
{
  return m_spacing;
}

//-----------------------------------------------------------------------------
void BrushSelector::setBrushImage(const QImage& image)
{
  if (m_brushImage != nullptr)
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
  int width = 2*m_displayRadius;

  m_brushColor.setAlphaF(m_brushOpacity/100.);

  QPixmap pix(width, width);
  pix.fill(Qt::transparent);
  QPainter painter(&pix);
  painter.setBrush(QBrush(m_brushColor));
  painter.setPen(m_drawing ? QPen(m_borderPaintColor) : QPen(m_borderEraseColor));
  painter.drawEllipse(0, 0, width-1, width-1);

  if (m_brushImage)
    painter.drawImage(QPoint(m_displayRadius/2,m_displayRadius/2), m_brushImage->scaledToWidth(m_displayRadius));

  m_cursor = QCursor(pix);
}

//-----------------------------------------------------------------------------
void BrushSelector::getBrushPosition(NmVector3 &center, QPoint pos)
{
  int H = (Plane::YZ == m_plane) ? 2 : 0;
  int V = (Plane::XZ == m_plane) ? 2 : 1;

  double wPos[3];
  int planeIndex = normalCoordinateIndex(m_plane);
  wPos[planeIndex] = m_pBounds[2*planeIndex];
  wPos[H] = m_LL[H] + pos.x()*m_worldSize[0]/m_viewSize[0];
  wPos[V] = m_UR[V] + pos.y()*m_worldSize[1]/m_viewSize[1];

  for(int i=0; i < 3; i++)
    center[i] = wPos[i];
}


//-----------------------------------------------------------------------------
bool BrushSelector::validStroke(NmVector3 &center)
{
  Bounds brushBounds = buildBrushBounds(center);

  if (!brushBounds.areValid())
    return false;

  return intersect(m_previewBounds, brushBounds);
}

//-----------------------------------------------------------------------------
void BrushSelector::startStroke(QPoint pos, RenderView* view)
{
  if (!m_item)
    return;

  m_pBounds = view->previewBounds(false);
  View2D *previewView = static_cast<View2D*>(view);
  m_plane = previewView->plane();

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
  getBrushPosition(center, pos);
  startPreview(view);
  m_lastUdpdatePoint = center;

  if (validStroke(center))
  {
    m_lastDot = pos;
    m_brushes << createBrushShape(m_item, center, m_radius, m_plane);
    updatePreview(center, view);
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::updateStroke(QPoint pos, RenderView* view)
{
  if (!m_item)
    return;

  if (!m_brushes.empty() > 0 && QLineF(m_lastDot, pos).length() < m_displayRadius/2.0)
    return;

  NmVector3 center;
  getBrushPosition(center, pos);

  if (validStroke(center))
  {
    m_lastDot = pos;
    m_brushes << createBrushShape(m_item, center, m_radius, m_plane);
    updatePreview(center, view);
  }

  m_lastUdpdatePoint = center;
}

//-----------------------------------------------------------------------------
void BrushSelector::stopStroke(RenderView* view)
{
  if(!m_item)
    return;

  if (!m_brushes.empty())
  {
    auto mask = voxelSelectionMask();
    Selector::SelectionItem item{QPair<SelectionMask, NeuroItemAdapterPtr>{mask, m_item}};
    Selector::Selection selection;
    selection << item;

    emit itemsSelected(selection);
  }

  m_drawing = !ShiftKeyIsDown();
  if (m_drawing)
  {
    stopPreview(view);
    buildCursor();
    view->setCursor(m_cursor);
    emit drawingModeChanged(m_drawing);
    view->updateView();
  }

  m_brushes.clear();
}

//-----------------------------------------------------------------------------
void BrushSelector::startPreview(RenderView* view)
{
  if (m_previewView != nullptr)
    return;

  NmVector3 spacing{m_spacing[0], m_spacing[1], m_spacing[2]};
  VolumeBounds previewBounds{ view->previewBounds(false), spacing, m_origin};
  auto volume = volumetricData(m_item->output());
  m_previewView = view;

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetNumberOfTableValues(2);
  m_lut->Build();
  m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_lut->SetTableValue(1, m_brushColor.redF(), m_brushColor.greenF(), m_brushColor.blueF(), m_brushOpacity/100.);
  m_lut->Modified();

  int extent[6];
  if (m_drawing)
  {
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
    unsigned char *imagePointer = reinterpret_cast<unsigned char *>(m_preview->GetScalarPointer());
    memset(imagePointer, 0, m_preview->GetNumberOfPoints());
    m_previewBounds = previewBounds.bounds();
  }
  else
  {
    if (!intersect(previewBounds.bounds(), volume->bounds()))
    {
      m_lut = nullptr;
      m_previewView = nullptr;

      return;
    }
    View2D* view2d = qobject_cast<View2D *>(m_previewView);
    Q_ASSERT(view2d);
    connect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(updateSliceChange()));

    for(auto prototype: m_item->representations())
      prototype->setActive(false, m_previewView);

    m_previewBounds = VolumeBounds(intersection(previewBounds.bounds(), volume->bounds()), spacing, m_origin).bounds();
    m_preview = vtkImage<itkVolumeType>(volume, m_previewBounds);
  }
  m_preview->Modified();
  m_preview->GetExtent(extent);

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputData(m_preview);
  m_mapToColors->SetUpdateExtent(extent);
  m_mapToColors->SetLookupTable(m_lut);
  m_mapToColors->SetNumberOfThreads(1);
  m_mapToColors->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetPickable(false);
  m_actor->SetDisplayExtent(extent);
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->SetNumberOfThreads(1);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapToColors->GetOutputPort());
  m_actor->GetMapper()->SetUpdateExtent(extent);
  m_actor->Update();

  // preview actor must be above others or it will be occluded
  View2D* view2d = qobject_cast<View2D *>(m_previewView);
  double pos[3];
  m_actor->GetPosition(pos);
  int index = normalCoordinateIndex(view2d->plane());
  pos[index] = pos[index] + ((index == 2) ? -View2D::SEGMENTATION_SHIFT : View2D::SEGMENTATION_SHIFT);
  m_actor->SetPosition(pos);

  m_previewView->addActor(m_actor);
  m_previewView->updateView();
}

//-----------------------------------------------------------------------------
void BrushSelector::updatePreview(NmVector3 center, RenderView* view)
{
  if (m_previewView == nullptr)
  {
    startPreview(view);

    m_pBounds = m_previewView->previewBounds(false);
    auto volume = volumetricData(m_item->output());
    if (!intersect(VolumeBounds(m_pBounds, NmVector3{m_spacing[0], m_spacing[1], m_spacing[2]}, m_origin).bounds(), volume->bounds()))
        return;
  }

  Bounds brushBounds = buildBrushBounds(center);
  QList<NmVector3> points;
  double r2 = m_radius*m_radius;

  if (intersect(brushBounds, m_previewBounds))
  {
    double point1[3] = { static_cast<double>(m_lastUdpdatePoint[0]), static_cast<double>(m_lastUdpdatePoint[1]), static_cast<double>(m_lastUdpdatePoint[2])};
    double point2[3] = { static_cast<double>(center[0]),static_cast<double>(center[1]),static_cast<double>(center[2]) };
    double distance = vtkMath::Distance2BetweenPoints(point1,point2);

    // apply stroke interpolation
    if ((distance >= r2) && m_lastUpdateBounds.areValid())
    {
      m_brushes.pop_back(); // we must delete the last one because we are going to replace it;

      double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
      int chunks = 2* static_cast<int>(distance/r2);
      double delta[3] = { vector[0]/chunks, vector[1]/chunks, vector[2]/chunks };
      for(auto i = 0; i < chunks; ++i)
      {
        points << NmVector3{m_lastUdpdatePoint[0] + static_cast<int>(delta[0] * i),
                            m_lastUdpdatePoint[1] + static_cast<int>(delta[1] * i),
                            m_lastUdpdatePoint[2] + static_cast<int>(delta[2] * i)};

        m_brushes << createBrushShape(m_item, points.last(), m_radius, m_plane);
      }
    }
    else
      points << center;

    int extent[6];
    m_preview->GetExtent(extent);
    for (auto point: points)
    {
      Bounds brushBounds = buildBrushBounds(point);
      if (!intersect(m_previewBounds, brushBounds))
        continue;

      Bounds pointBounds = intersection(m_previewBounds,brushBounds);
      double pointCenter[3]{ point[0], point[1], point[2] };
      int depth;

      switch(m_plane)
      {
        case Plane::XY:
          depth = vtkMath::Round(((m_previewBounds[4]+m_previewBounds[5])/2)/m_spacing[2]);
          for (int x = vtkMath::Round((pointBounds[0]+m_spacing[0]/2)/m_spacing[0]); x < vtkMath::Round((pointBounds[1]+m_spacing[0]/2)/m_spacing[0]); x++)
            for (int y = vtkMath::Round((pointBounds[2]+m_spacing[1]/2)/m_spacing[1]); y < vtkMath::Round((pointBounds[3]+m_spacing[1]/2)/m_spacing[1]); y++)
          {
            if (x < extent[0] || x > extent[1])
              continue;
            if (y < extent[2] || y > extent[3])
              continue;

            double pixel[3]{x*m_spacing[0], y*m_spacing[1], m_previewBounds[4]};
            if (vtkMath::Distance2BetweenPoints(pointCenter,pixel) < r2)
            {
              unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,y,depth));
              *pixel = (m_drawing ? 1 : 0);
            }
          }
          break;
        case Plane::XZ:
          depth = vtkMath::Round(((m_previewBounds[2]+m_previewBounds[3])/2)/m_spacing[1]);
          for (int x = vtkMath::Round((pointBounds[0]+m_spacing[0]/2)/m_spacing[0]); x < vtkMath::Round((pointBounds[1]+m_spacing[0]/2)/m_spacing[0]); x++)
            for (int z = vtkMath::Round((pointBounds[4]+m_spacing[2]/2)/m_spacing[2]); z < vtkMath::Round((pointBounds[5]+m_spacing[2]/2)/m_spacing[2]); z++)
          {
            if (x < extent[0] || x > extent[1])
              continue;
            if (z < extent[4] || z > extent[5])
              continue;

            double pixel[3] = {x*m_spacing[0], m_previewBounds[2], z*m_spacing[2]};
            if (vtkMath::Distance2BetweenPoints(pointCenter,pixel) < r2)
            {
              unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(x,depth,z));
              *pixel = (m_drawing ? 1 : 0);
            }
          }
          break;
        case Plane::YZ:
          depth = vtkMath::Round(((m_previewBounds[0]+m_previewBounds[1])/2)/m_spacing[0]);
          for (int y = vtkMath::Round((pointBounds[2]+m_spacing[1]/2)/m_spacing[1]); y < vtkMath::Round((pointBounds[3]+m_spacing[1]/2)/m_spacing[1]); y++)
            for (int z = vtkMath::Round((pointBounds[4]+m_spacing[2]/2)/m_spacing[2]); z < vtkMath::Round((pointBounds[5]+m_spacing[2]/2)/m_spacing[2]); z++)
          {
            if (y < extent[2] || y > extent[3])
              continue;
            if (z < extent[4] || z > extent[5])
              continue;

            double pixel[3] = {m_previewBounds[0], y*m_spacing[1], z*m_spacing[2]};
            if (vtkMath::Distance2BetweenPoints(pointCenter,pixel) < r2)
            {
              unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(depth,y,z));
              *pixel = (m_drawing ? 1 : 0);
            }
          }
          break;
        default:
          break;
      }
    }
    m_lastUpdateBounds = brushBounds;
    m_preview->Modified();
    m_mapToColors->Update();
    m_actor->Update();
    m_previewView->updateView();
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::stopPreview(RenderView* view)
{
  if (m_previewView == nullptr || m_previewView != view)
    return;

  disconnect(m_previewView, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(updateSliceChange()));
  m_previewView->removeActor(m_actor);
  m_lut = nullptr;
  m_preview = nullptr;
  m_mapToColors = nullptr;
  m_actor = nullptr;
  m_previewBounds = m_lastUpdateBounds = Bounds();

  if (m_item->type() == ViewItemAdapter::Type::SEGMENTATION)
    for(auto prototype: m_item->representations())
      prototype->setActive(true, m_previewView);

  m_previewView->updateView();
  m_previewView = nullptr;
}

//-----------------------------------------------------------------------------
QColor BrushSelector::getBrushColor()
{
  return m_brushColor;
}

//-----------------------------------------------------------------------------
BinaryMaskSPtr<unsigned char> BrushSelector::voxelSelectionMask() const
{
  Q_ASSERT(!m_brushes.empty());

  const NmVector3 spacing{ m_spacing[0], m_spacing[1], m_spacing[2] };
  auto image = define_itkImage<itkVolumeType>(m_origin, spacing);

  Bounds strokeBounds;
  for (auto brush : m_brushes)
  {
    VolumeBounds brushBounds = volumeBounds<itkVolumeType>(image, brush.second);

    if (!strokeBounds.areValid())
      strokeBounds = brushBounds.bounds();
    else
      strokeBounds = boundingBox(strokeBounds, brushBounds.bounds());
  }

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

  auto value = (m_drawing ? SEG_VOXEL_VALUE : SEG_BG_VALUE);
  mask->setForegroundValue(value);
  return BinaryMaskSPtr<unsigned char>(mask);
}


//-----------------------------------------------------------------------------
Bounds BrushSelector::buildBrushBounds(NmVector3 center)
{
  Bounds bounds = { '[', center[0] - m_radius, center[0] + m_radius, ')' ,
                    '[', center[1] - m_radius, center[1] + m_radius, ')' ,
                    '[', center[2] - m_radius, center[2] + m_radius, ')' };

  int index = normalCoordinateIndex(m_plane);
  bounds[2*index] = m_pBounds[2*index];
  bounds[2*index+1] = m_pBounds[2*index+1];
  bounds.setUpperInclusion(toAxis(index), true);

  NmVector3 spacing{m_spacing[0], m_spacing[1], m_spacing[2]};
  VolumeBounds adjustedBounds(bounds, spacing, m_origin);

  return adjustedBounds.bounds();
}

//-----------------------------------------------------------------------------
void BrushSelector::updateSliceChange()
{
  View2D* view = qobject_cast<View2D *>(sender());
  Q_ASSERT(view && view == m_previewView);

  if (m_tracking)
  {
    stopStroke(view);
    m_tracking = false;
  }
  m_drawing = true;
  stopPreview(view);
  buildCursor();
  view->setCursor(m_cursor);
  emit drawingModeChanged(m_drawing);
}

//-----------------------------------------------------------------------------
bool BrushSelector::ShiftKeyIsDown()
{
  // true if Shift button is down.
  return QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
}

//-----------------------------------------------------------------------------
void BrushSelector::abortOperation()
{
  if (m_previewView == nullptr)
    return;

  if (!m_brushes.empty())
  {
    m_brushes.clear();
    stopStroke(m_previewView);
  }

  RenderView *view = m_previewView;
  stopPreview(view);
  m_drawing = true;
  buildCursor();
  view->setCursor(m_cursor);
  emit drawingModeChanged(m_drawing);
}
