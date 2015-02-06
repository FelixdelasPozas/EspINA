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
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <GUI/View/Widgets/Contour/vtkVoxelContour2D.h>

// VTK
#include <vtkImageCanvasSource2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTransformTextureCoords.h>
#include <vtkProperty.h>
#include <vtkTubeFilter.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
ROIWidget::ROIWidget(ROISPtr roi)
: m_ROI{roi}
, m_color{Qt::yellow}
{
  connect(m_ROI.get(), SIGNAL(dataChanged()),
          this,        SLOT(updateROIRepresentations()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
ROIWidget::~ROIWidget()
{
  disconnect(m_ROI.get(), SIGNAL(dataChanged()),
             this,        SLOT(updateROIRepresentations()));

  m_representations.clear();
}

//----------------------------------------------------------------------------
void ROIWidget::updateActor(View2D *view)
{
  auto bounds = m_ROI->bounds();
  auto index  = normalCoordinateIndex(view->plane());
  auto pos    = view->crosshair()[index];

  bounds[2 * index] = bounds[(2 * index) + 1] = pos;

  bounds.setUpperInclusion(toAxis(index), true);

  if (!intersect(m_ROI->bounds(), bounds))
  {
    if(m_representations[view].actor != nullptr)
    {
      m_representations[view].actor->SetVisibility(false);
      view->updateView();
    }
    return;
  }

  auto image = vtkImage<itkVolumeType>(m_ROI->itkImage(bounds), bounds);

  if(m_representations[view].actor == nullptr)
  {
    m_representations[view].contour = vtkSmartPointer<vtkVoxelContour2D>::New();
    m_representations[view].contour->SetInputData(image);
    m_representations[view].contour->UpdateWholeExtent();

    m_representations[view].mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_representations[view].mapper->SetInputConnection(m_representations[view].contour->GetOutputPort());
    m_representations[view].mapper->SetUpdateExtent(image->GetExtent());
    m_representations[view].mapper->SetColorModeToDefault();
    m_representations[view].mapper->ScalarVisibilityOff();
    m_representations[view].mapper->StaticOff();
    m_representations[view].mapper->UpdateWholeExtent();

    m_representations[view].actor = vtkSmartPointer<vtkActor>::New();
    m_representations[view].actor->SetMapper(m_representations[view].mapper);
    m_representations[view].actor->GetProperty()->SetColor(m_color.redF(), m_color.greenF(), m_color.blueF());
    m_representations[view].actor->GetProperty()->SetOpacity(1);
    m_representations[view].actor->GetProperty()->Modified();
    m_representations[view].actor->SetVisibility(true);
    m_representations[view].actor->SetDragable(false);
    m_representations[view].actor->Modified();

    double position[3];
    m_representations[view].actor->GetPosition(position);
    position[index] += view->widgetDepth();
    m_representations[view].actor->SetPosition(position);

    view->addActor(m_representations[view].actor);
  }
  else
  {
    m_representations[view].contour->SetInputData(image);
    m_representations[view].contour->SetUpdateExtent(image->GetExtent());
    m_representations[view].contour->Update();

    m_representations[view].mapper->SetUpdateExtent(image->GetExtent());
    m_representations[view].mapper->Update();

    m_representations[view].actor->SetVisibility(true);
    m_representations[view].actor->Modified();
  }

  view->updateView();
}

//-----------------------------------------------------------------------------
void ROIWidget::registerView(RenderView* view)
{
  auto view2d = dynamic_cast<View2D *>(view);

  if(!view2d || m_representations.contains(view2d)) return;

  connect(view2d, SIGNAL(sliceChanged(Plane, Nm)),
          this,   SLOT(sliceChanged(Plane, Nm)));

  updateActor(view2d);
}

//-----------------------------------------------------------------------------
void ROIWidget::unregisterView(RenderView* view)
{
  auto view2d = dynamic_cast<View2D *>(view);

  if(!view2d || !m_representations.contains(view2d)) return;

  disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));

  view2d->removeActor(m_representations[view2d].actor);
  m_representations.remove(view2d);
}

//-----------------------------------------------------------------------------
void ROIWidget::setEnabled(bool enable)
{
  for(auto pipeline: m_representations)
  {
    pipeline.actor->SetVisibility(enable);
  }
}

//-----------------------------------------------------------------------------
void ROIWidget::setColor(const QColor& color)
{
  if (m_color != color)
  {
    m_color = color;

    for (auto pipeline : m_representations)
    {
      auto actorProp = pipeline.actor->GetProperty();
      actorProp->SetColor(color.redF(), color.greenF(), color.blueF());
    }
  }
}

//-----------------------------------------------------------------------------
void ROIWidget::sliceChanged(Plane plane, Nm pos)
{
  auto rView = qobject_cast<View2D *>(sender());

  for(auto view: m_representations.keys())
  {
    if(view == rView)
    {
      updateActor(view);
      return;
    }
  }
}

//----------------------------------------------------------------------------
void ROIWidget::updateROIRepresentations()
{
  for(auto view: m_representations.keys())
  {
    updateActor(view);
  }
}

