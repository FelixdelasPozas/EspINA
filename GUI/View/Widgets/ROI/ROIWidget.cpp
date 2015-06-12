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
#include <vtkImageCanvasSource2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTransformTextureCoords.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets::ROI;

//-----------------------------------------------------------------------------
ROIWidget::ROIWidget(ROISPtr roi)
: m_ROI{roi}
, m_color{Qt::yellow}
, m_depth{0.0}
{
}

//-----------------------------------------------------------------------------
void ROIWidget::setPlane(Plane plane)
{
  acceptChangesOnPlane(plane);
}

//-----------------------------------------------------------------------------
void ROIWidget::setRepresentationDepth(Nm depth)
{
  m_depth = depth;
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
  return acceptPlaneCrosshairChange(crosshair);
}

//-----------------------------------------------------------------------------
bool ROIWidget::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return true;
}

//-----------------------------------------------------------------------------
void ROIWidget::initialize(RenderView *view)
{
  m_contour = vtkSmartPointer<vtkVoxelContour2D>::New();
//   m_contour->UpdateWholeExtent();

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetColorModeToDefault();
  m_mapper->ScalarVisibilityOff();
  m_mapper->StaticOff();
//   m_mapper->UpdateWholeExtent();

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetColor(m_color.redF(), m_color.greenF(), m_color.blueF());
  m_actor->GetProperty()->SetOpacity(1);
  m_actor->GetProperty()->Modified();
  m_actor->SetVisibility(false);
  m_actor->SetDragable(false);
//   m_actor->Modified();

  m_view = view;

  m_view->addActor(m_actor);

  connect(m_ROI.get(), SIGNAL(dataChanged()),
          this,        SLOT(onROIChanged()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void ROIWidget::uninitialize()
{
  disconnect(m_ROI.get(), SIGNAL(dataChanged()));

  m_view->removeActor(m_actor);
}

//-----------------------------------------------------------------------------
void ROIWidget::show()
{
  m_actor->SetVisibility(true);
}

//-----------------------------------------------------------------------------
void ROIWidget::hide()
{
  m_actor->SetVisibility(false);
}

//-----------------------------------------------------------------------------
void ROIWidget::setColor(const QColor& color)
{
  if (m_color != color && m_actor.Get())
  {
    auto actorProp = m_actor->GetProperty();
    actorProp->SetColor(color.redF(), color.greenF(), color.blueF());
  }

  m_color = color;
}

//-----------------------------------------------------------------------------
void ROIWidget::setCrosshair(const NmVector3 &crosshair)
{
  changeReslicePosition(crosshair);

  updateCurrentSlice();
}


//----------------------------------------------------------------------------
void ROIWidget::onROIChanged()
{
  updateCurrentSlice();
}


//----------------------------------------------------------------------------
vtkSmartPointer<vtkImageData> ROIWidget::currentSlice() const
{
  auto bounds = m_ROI->bounds();

  bounds[2 * m_normalIndex] = bounds[(2 * m_normalIndex) + 1] = m_reslicePosition;

  bounds.setUpperInclusion(toAxis(m_normalIndex), true);

  vtkSmartPointer<vtkImageData> slice;

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

  bool isVisible = image.Get() != nullptr;

  if (isVisible)
  {
    m_contour->SetInputData(image);
    m_contour->SetUpdateExtent(image->GetExtent());
    m_contour->UpdateWholeExtent();
    m_contour->Update();

    m_mapper->SetInputConnection(m_contour->GetOutputPort());
    m_mapper->SetUpdateExtent(image->GetExtent());
    m_mapper->UpdateWholeExtent();
    m_mapper->Update();


    double position[3];
    m_actor->GetPosition(position);
    position[m_normalIndex] += m_depth;
    m_actor->SetPosition(position);
  }

  m_actor->SetVisibility(isVisible);
  m_actor->Modified();
}