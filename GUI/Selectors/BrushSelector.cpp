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

// ESPINA
#include "BrushSelector.h"
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Spatial.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>
#include <GUI/View/Selection.h>
#include <App/ToolGroups/View/Representations/SegmentationSlice/SegmentationSlicePipeline.h>
#include <Support/Representations/RepresentationUtils.h>

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
#include <vtkLookupTable.h>

using namespace ESPINA;

class BrushSelector::BrushPipeline
: RepresentationPipeline
{
public:
  BrushPipeline()
  : RepresentationPipeline("BrushPipeline")
  , m_transparency{1.0}
  , m_view        {nullptr}
  , m_slice       {Plane::XY, ColorEngineSPtr()}{}

  virtual RepresentationState representationState(const ViewItemAdapter *item, const RepresentationState &settings);

  virtual ActorList createActors(const ViewItemAdapter *item, const RepresentationState &state);

  virtual bool pick(ViewItemAdapter *item, const NmVector3 &point) const;

  VTKActor createTemporalActor(ViewItemAdapter *referenceItem);



  void setDrawMode();

  void setEraseMode();

  void setPreviewView(View2D* view)
  { m_view = view; Q_ASSERT(view); }

private:
  void setTransparency(double value);
  bool isTemporalActor(const RepresentationState &state);

  double    m_transparency;

  View2D*   m_view;

  SegmentationSlicePipeline m_slice;
};

//-----------------------------------------------------------------------------
RepresentationState BrushSelector::BrushPipeline::representationState(const ViewItemAdapter *item,
                                                                      const RepresentationState &settings)
{
  auto state = m_slice.representationState(item, settings);

  state.setValue<double>("Transparency", m_transparency);

  return state;
}

//-----------------------------------------------------------------------------
RepresentationPipeline::ActorList BrushSelector::BrushPipeline::createActors(const ViewItemAdapter *item,
                                                                             const RepresentationState &state)
{
  ActorList actors;

  if (isTemporalActor(state))
  {
    //actors << m_actor;
  }
  else
  {
    m_slice.setPlane(RepresentationUtils::plane(state));
    actors = m_slice.createActors(item, state);
  }

  return actors;
}

//-----------------------------------------------------------------------------
bool BrushSelector::BrushPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
{
  return m_slice.pick(item, point);
}

//-----------------------------------------------------------------------------
bool BrushSelector::BrushPipeline::isTemporalActor(const RepresentationState &state)
{
  return m_view->plane() == RepresentationUtils::plane(state)
      && m_view->crosshair() == crosshairPoint(state);
}

//-----------------------------------------------------------------------------
void BrushSelector::BrushPipeline::setDrawMode()
{
  setTransparency(1);
}

//-----------------------------------------------------------------------------
void BrushSelector::BrushPipeline::setEraseMode()
{
  setTransparency(0.5);
}

//-----------------------------------------------------------------------------
void BrushSelector::BrushPipeline::setTransparency(double value)
{
  m_transparency = value;
}

//-----------------------------------------------------------------------------
BrushSelector::BrushSelector()
: m_item            {nullptr}
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
, m_eraseMode       {false}
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
  {
    stopPreview(m_previewView);
  }
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

        m_drawing = !m_eraseMode;

        if (m_previewView != nullptr)
        {
          stopPreview(view);
        }

        emit drawingModeChanged(m_drawing);
      }
      break;
    case QEvent::Enter:
      {
        updateCurrentDrawingMode(view);
      }
      break;
    case QEvent::KeyPress:
      {
        ke = static_cast<QKeyEvent *>(e);
        if ((ke->key() == Qt::Key_Shift) && !m_tracking && m_item && (m_item->type() == ViewItemAdapter::Type::SEGMENTATION))
        {
          updateCurrentDrawingMode(view);

          return true;
        }
      }
      break;
    case QEvent::KeyRelease:
      {
        ke = static_cast<QKeyEvent *>(e);
        if ((ke->key() == Qt::Key_Shift) && !m_tracking)
        {
          updateCurrentDrawingMode(view);
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
void BrushSelector::setEraseMode(bool value)
{
  m_eraseMode = value;
  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setRadius(int radius)
{
  if (radius <= 0)
  {
    m_displayRadius = 1;
  }
  else if (radius > MAX_RADIUS)
  {
    m_displayRadius = MAX_RADIUS;
  }
  else
  {
    m_displayRadius = radius;
  }

  buildCursor();
}

//-----------------------------------------------------------------------------
void BrushSelector::setBorderPaintColor(QColor color)
{
  m_borderPaintColor = color;

  if(m_drawing)
  {
    buildCursor();
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::setBorderEraseColor(QColor color)
{
  m_borderEraseColor = color;

  if(!m_drawing)
  {
    buildCursor();
  }
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
    m_item       = nullptr;
    m_origin     = NmVector3();
    m_spacing[0] = m_spacing[1] = m_spacing[2] = 0;
  }
  else
  {
    m_item    = item;
    m_spacing = ItkSpacing<itkVolumeType>(m_item->output()->spacing());

    if(hasVolumetricData(m_item->output()))
    {
      m_origin = volumetricData(item->output())->origin();
    }
    else
    {
      m_origin = NmVector3{0,0,0};
    }
  }
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
  {
    painter.drawImage(QPoint(m_displayRadius/2,m_displayRadius/2), m_brushImage->scaledToWidth(m_displayRadius));
  }

  m_cursor = QCursor(pix);
}

//-----------------------------------------------------------------------------
void BrushSelector::getBrushPosition(NmVector3 &center, QPoint const pos)
{
  int H = (Plane::YZ == m_plane) ? 2 : 0;
  int V = (Plane::XZ == m_plane) ? 2 : 1;

  double wPos[3];
  int planeIndex = normalCoordinateIndex(m_plane);
  wPos[planeIndex] = m_previewBounds[2*planeIndex];
  wPos[H] = m_LL[H] + pos.x()*m_worldSize[0]/m_viewSize[0];
  wPos[V] = m_UR[V] + pos.y()*m_worldSize[1]/m_viewSize[1];

  for(int i=0; i < 3; i++)
  {
    center[i] = wPos[i];
  }
}

//-----------------------------------------------------------------------------
bool BrushSelector::validStroke(NmVector3 &center)
{
  Bounds brushBounds = buildBrushBounds(center);

  if (!brushBounds.areValid()) return false;

  if(hasVolumetricData(m_item->output()))
  {
    auto volume = volumetricData(m_item->output());
    if(!m_drawing && !intersect(m_previewBounds, volume->bounds()))
    {
      return false;
    }
  }

  return intersect(m_previewBounds, brushBounds);
}

//-----------------------------------------------------------------------------
void BrushSelector::startStroke(QPoint pos, RenderView* view)
{
  if (!m_item) return;

  auto previewView = view2D_cast(view);
  m_plane = previewView->plane();
  m_previewBounds = view->previewBounds(false);

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
    updatePreview(createBrushShape(m_item, center, m_radius, m_plane), view);
  }
}

//-----------------------------------------------------------------------------
void BrushSelector::updateStroke(QPoint pos, RenderView* view)
{
  if (!m_item) return;

  if (!m_brushes.empty() > 0 && QLineF(m_lastDot, pos).length() < m_displayRadius/2.0)
    return;

  NmVector3 center;
  getBrushPosition(center, pos);

  if (validStroke(center))
  {
    m_lastDot = pos;
    updatePreview(createBrushShape(m_item, center, m_radius, m_plane), view);
  }

  m_lastUdpdatePoint = center;
}

//-----------------------------------------------------------------------------
void BrushSelector::stopStroke(RenderView* view)
{
  if(!m_item) return;

  if (!m_brushes.empty())
  {
    auto mask = voxelSelectionMask();
    Selector::SelectionItem item{QPair<SelectionMask, NeuroItemAdapterPtr>{mask, m_item}};
    Selector::Selection selection;
    selection << item;

    emit itemsSelected(selection);
  }

  updateCurrentDrawingMode(view);

  m_brushes.clear();
}

//-----------------------------------------------------------------------------
void BrushSelector::startPreview(RenderView* view)
{
  if (m_preview) return;
  if (!m_item)   return;
  if (!hasVolumetricData(m_item->output())) return;

  m_previewView   = view;
  m_previewBounds = view->previewBounds(false);

  auto spacing       = ToNmVector3<itkVolumeType>(m_spacing);
  auto previewBounds = VolumeBounds{m_previewBounds, spacing, m_origin};

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetNumberOfTableValues(2);
  m_lut->Build();
  m_lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  m_lut->SetTableValue(1, m_brushColor.redF(), m_brushColor.greenF(), m_brushColor.blueF(), m_brushOpacity/100.);
  m_lut->Modified();

  int extent[6];

  auto view2d = view2D_cast(view);
  Q_ASSERT(view2d);

  m_brushPipeline = std::make_shared<BrushPipeline>();
  m_brushPipeline->setPreviewView(view2d);

  if (m_drawing)
  {
    for (int i = 0; i < 3; ++i)
    {
      extent[2 * i]       = m_previewBounds[2 * i]       / m_spacing[i];
      extent[(2 * i) + 1] = m_previewBounds[(2 * i) + 1] / m_spacing[i];
    }

    m_brushPipeline->setDrawMode();

    m_preview = vtkSmartPointer<vtkImageData>::New();
    m_preview->SetOrigin(0, 0, 0);
    m_preview->SetExtent(extent);
    m_preview->SetSpacing(m_spacing[0], m_spacing[1], m_spacing[2]);

    auto info = m_preview->GetInformation();
    vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
    vtkImageData::SetNumberOfScalarComponents(1, info);
    m_preview->SetInformation(info);
    m_preview->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    m_preview->Modified();
    unsigned char *imagePointer = reinterpret_cast<unsigned char *>(m_preview->GetScalarPointer());
    memset(imagePointer, 0, m_preview->GetNumberOfPoints());
  }
  else
  {
    m_brushPipeline->setEraseMode();

    auto volume = volumetricData(m_item->output());
    if (!intersect(previewBounds.bounds(), volume->bounds()))
    {
      m_lut = nullptr;
      m_previewView = nullptr;

      return;
    }

    m_preview = vtkImage<itkVolumeType>(volume, VolumeBounds(intersection(m_previewBounds, volume->bounds()), spacing, m_origin).bounds());
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
  double pos[3];
  m_actor->GetPosition(pos);
  pos[normalCoordinateIndex(view2d->plane())] += 2 * view2d->segmentationDepth();
  m_actor->SetPosition(pos);

  m_previewView->addActor(m_actor);
  m_previewView->updateView();
}

//-----------------------------------------------------------------------------
void BrushSelector::updatePreview(BrushShape shape, RenderView* view)
{
  NmVector3 nmSpacing{m_spacing[0], m_spacing[1], m_spacing[2]};

  if (!m_item || !hasVolumetricData(m_item->output())) return;

  if (m_previewView == nullptr)
  {
    startPreview(view);

    if (!intersect(VolumeBounds(m_previewBounds, nmSpacing, m_origin).bounds(), m_item->output()->bounds())) return;
  }

  Bounds brushBounds = shape.second;
  NmVector3 center{(brushBounds[0]+brushBounds[1])/2, (brushBounds[2]+brushBounds[3])/2, (brushBounds[4]+brushBounds[5])/2};

  auto r2 = m_radius * m_radius;

  if (intersect(brushBounds, m_previewBounds))
  {
    double point1[3] = { static_cast<double>(m_lastUdpdatePoint[0]), static_cast<double>(m_lastUdpdatePoint[1]), static_cast<double>(m_lastUdpdatePoint[2])};
    double point2[3] = { center[0], center[1], center[2] };
    double distance = vtkMath::Distance2BetweenPoints(point1,point2);

    BrushShapeList brushes;
    brushes << shape;

    // apply stroke interpolation
    if ((distance >= r2) && m_lastUpdateBounds.areValid())
    {
      brushes.clear(); // we are going to replace it with a list of brushes.

      double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
      int chunks = 2 * static_cast<int>(distance/r2);
      double delta[3] = { vector[0]/chunks, vector[1]/chunks, vector[2]/chunks };
      for(auto i = 0; i < chunks; ++i)
      {
        auto pointCenter = NmVector3{m_lastUdpdatePoint[0] + static_cast<int>(delta[0] * i),
                                     m_lastUdpdatePoint[1] + static_cast<int>(delta[1] * i),
                                     m_lastUdpdatePoint[2] + static_cast<int>(delta[2] * i)};

        brushes << createBrushShape(m_item, pointCenter, m_radius, m_plane);;
      }
    }

    int extent[6];
    m_preview->GetExtent(extent);
    for (auto brush: brushes)
    {
      if (!intersect(m_previewBounds, brush.second))
      {
        brushes.removeOne(brush);
        continue;
      }

      Bounds pointBounds = intersection(m_previewBounds, brush.second);
      auto region = equivalentRegion<itkVolumeType>(m_origin, nmSpacing, pointBounds);
      auto tempImage = create_itkImage<itkVolumeType>(pointBounds, SEG_BG_VALUE, nmSpacing, m_origin);

      itk::ImageRegionIteratorWithIndex<itkVolumeType> it(tempImage, region);
      it.GoToBegin();
      while(!it.IsAtEnd())
      {
        auto index = it.GetIndex();

        if (!(index[0] < extent[0] || index[0] > extent[1] || index[1] < extent[2] || index[1] > extent[3] || index[2] < extent[4] || index[2] > extent[5])
           && (brush.first->FunctionValue(index[0] * m_spacing[0], index[1] * m_spacing[1], index[2] * m_spacing[2]) <= 0))
        {
          unsigned char *pixel = static_cast<unsigned char*>(m_preview->GetScalarPointer(index[0],index[1], index[2]));
          *pixel = (m_drawing ? 1 : 0);
        }

        ++it;
      }
    }

    m_brushes << brushes;
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
  if (m_previewView == nullptr || m_previewView != view || !m_item || !hasVolumetricData(m_item->output())) return;

  disconnect(m_previewView, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(updateSliceChange()));
  m_previewView->removeActor(m_actor);
  m_lut = nullptr;
  m_preview = nullptr;
  m_mapToColors = nullptr;
  m_actor = nullptr;
  m_previewBounds = m_lastUpdateBounds = Bounds();

  if (isSegmentation(m_item))
  {
    // TODO: Restore representation
    Q_ASSERT(false);
//     for(auto prototype: m_item->representations())
//     {
//       prototype->setActive(true, m_previewView);
//     }
  }

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
  bounds[2*index] = m_previewBounds[2*index];
  bounds[2*index+1] = m_previewBounds[2*index+1];
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
  Q_ASSERT(!m_drawing);

  if(m_actor != nullptr)
  {
    m_previewView->removeActor(m_actor);
  }

  m_actor = nullptr;
  m_mapToColors = nullptr;
  m_preview = nullptr;

  if(!hasVolumetricData(m_item->output()))
  {
    return;
  }

  NmVector3 nmSpacing { m_spacing[0], m_spacing[1], m_spacing[2] };
  auto volume = volumetricData(m_item->output());
  m_previewBounds = m_previewView->previewBounds(false);
  if(!intersect(volume->bounds(), m_previewBounds))
    return;

  m_preview = vtkImage<itkVolumeType>(volume, VolumeBounds(intersection(m_previewBounds, volume->bounds()), nmSpacing, m_origin).bounds());
  m_preview->Modified();
  int extent[6];
  m_preview->GetExtent(extent);

  m_mapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapToColors->SetInputData(m_preview);
  m_mapToColors->DebugOn();
  m_mapToColors->GlobalWarningDisplayOn();
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
  auto view2d = qobject_cast<View2D *>(m_previewView);
  double pos[3];
  m_actor->GetPosition(pos);
  pos[normalCoordinateIndex(view2d->plane())] += 2 * view2d->segmentationDepth();
  m_actor->SetPosition(pos);
  m_previewView->addActor(m_actor);
}

//-----------------------------------------------------------------------------
inline bool BrushSelector::ShiftKeyIsDown()
{
  // true if Shift button is down.
  return QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
}

//-----------------------------------------------------------------------------
void BrushSelector::updateCurrentDrawingMode(RenderView* view)
{
  if (ShiftKeyIsDown())
  {
    m_drawing = m_eraseMode;
  }
  else
  {
    m_drawing = !m_eraseMode;
  }

  buildCursor();
  view->setCursor(m_cursor);

  if (m_drawing)
  {
    stopPreview(view);
  }
  else
  {
    startPreview(view);
  }

  emit drawingModeChanged(m_drawing);
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

  updateCurrentDrawingMode(m_previewView);
}
