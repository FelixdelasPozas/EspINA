/*
 * Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>
 * This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ESPINA
#include <GUI/View/Widgets/ROI/ROIWidget.h>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/vtkVoxelContour2D.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <GUI/View/Utils.h>
#include <GUI/Representations/Frame.h>

// VTK
#include <vtkImageCanvasSource2D.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTransformTextureCoords.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets::ROI;
using namespace ESPINA::GUI::View::Utils;

//-----------------------------------------------------------------------------
ROIWidget::ROIWidget(ROISPtr roi)
: m_ROI         {roi}
, m_color       {Qt::yellow}
, m_depth       {0}
, m_reslicePoint{VTK_DOUBLE_MAX}
, m_index       {-1}
, m_view        {nullptr}
{
}

//-----------------------------------------------------------------------------
void ROIWidget::setPlane(Plane plane)
{
  // intentionally empty.
}

//-----------------------------------------------------------------------------
void ROIWidget::setRepresentationDepth(Nm depth)
{
  // intentionally empty.
}

//-----------------------------------------------------------------------------
TemporalRepresentation2DSPtr ROIWidget::clone()
{
  auto representation = std::make_shared<ROIWidget>(m_ROI);

  representation->setColor(m_color);

  return representation;
}

//-----------------------------------------------------------------------------
bool ROIWidget::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return m_view && (m_reslicePoint != crosshair[m_index]);
}

//-----------------------------------------------------------------------------
bool ROIWidget::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return false;
}

//-----------------------------------------------------------------------------
bool ROIWidget::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return false;
}

//-----------------------------------------------------------------------------
bool ROIWidget::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//-----------------------------------------------------------------------------
void ROIWidget::initialize(RenderView *view)
{
  if(m_view || !isView2D(view)) return;

  m_view         = view;
  m_index        = normalCoordinateIndex(view2D_cast(view)->plane());
  m_reslicePoint = m_view->crosshair()[m_index];

  m_contour = vtkSmartPointer<vtkVoxelContour2D>::New();

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetColorModeToDefault();
  m_mapper->ScalarVisibilityOff();
  m_mapper->StaticOff();

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetColor(m_color.redF(), m_color.greenF(), m_color.blueF());
  m_actor->GetProperty()->SetOpacity(1);
  m_actor->GetProperty()->Modified();
  m_actor->SetVisibility(false);
  m_actor->SetDragable(false);

  repositionActor(m_actor, view2D_cast(view)->widgetDepth(), m_index);

  updateCurrentSlice();

  m_view->addActor(m_actor);

  connect(m_ROI.get(), SIGNAL(dataChanged()),
          this,        SLOT(onROIChanged()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void ROIWidget::uninitialize()
{
  disconnect(m_ROI.get(), SIGNAL(dataChanged()),
             this,        SLOT(onROIChanged()));

  m_view->removeActor(m_actor);
  m_view = nullptr;
}

//-----------------------------------------------------------------------------
void ROIWidget::show()
{
  m_actor->SetVisibility(true);
  m_actor->Modified();
}

//-----------------------------------------------------------------------------
void ROIWidget::hide()
{
  m_actor->SetVisibility(false);
  m_actor->Modified();
}

//-----------------------------------------------------------------------------
void ROIWidget::setColor(const QColor& color)
{
  if (m_color != color && m_actor.Get())
  {
    auto actorProp = m_actor->GetProperty();
    actorProp->SetColor(color.redF(), color.greenF(), color.blueF());

    m_color = color;
  }
}

//-----------------------------------------------------------------------------
void ROIWidget::display(const GUI::Representations::FrameCSPtr& frame)
{
  if(m_view)
  {
    m_reslicePoint = frame->crosshair[m_index];

    updateCurrentSlice();
  }
}

//----------------------------------------------------------------------------
void ROIWidget::onROIChanged()
{
  updateCurrentSlice();
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> ROIWidget::currentSlice() const
{
  Bounds bounds = m_ROI->bounds();
  bounds[2*m_index] = bounds[(2*m_index) + 1] = m_reslicePoint;
  bounds.setUpperInclusion(toAxis(m_index), true);

  vtkSmartPointer<vtkImageData> slice = nullptr;

  if (intersect(m_ROI->bounds(), bounds))
  {
    slice = vtkImage<itkVolumeType>(m_ROI->itkImage(bounds), bounds);
  }

  return slice;
}

//----------------------------------------------------------------------------
void ROIWidget::updateCurrentSlice()
{
  auto image = currentSlice();

  bool hasSlice = image.Get() != nullptr;

  if (hasSlice)
  {
    m_contour->SetInputData(image);
    m_contour->SetUpdateExtent(image->GetExtent());
    m_contour->UpdateWholeExtent();
    m_contour->Update();

    m_mapper->SetInputConnection(m_contour->GetOutputPort());
    m_mapper->SetUpdateExtent(image->GetExtent());
    m_mapper->UpdateWholeExtent();
    m_mapper->Update();
  }

  m_actor->Modified();
}
