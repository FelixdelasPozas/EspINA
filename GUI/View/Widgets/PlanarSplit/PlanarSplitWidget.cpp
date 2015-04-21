/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "PlanarSplitWidget.h"
#include "vtkPlanarSplitWidget.h"
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkPoints.h>
#include <vtkPlane.h>
#include <vtkMath.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkAbstractWidget.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkRenderWindow.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
PlanarSplitWidget::PlanarSplitWidget()
: m_mainWidget          {NONE}
, m_vtkVolumeInformation{nullptr}
, m_command             {vtkSmartPointer<vtkSplitCommand>::New()}
{
  m_command->setWidget(this);
}

//-----------------------------------------------------------------------------
PlanarSplitWidget::~PlanarSplitWidget()
{
  if (m_mainWidget != NONE)
    for(auto view: m_widgets.keys())
      unregisterView(view);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::registerView(RenderView *view)
{
  if(m_widgets.keys().contains(view))
    return;

  if(isView2D(view))
  {
    auto view2d = dynamic_cast<View2D *>(view);
    auto plane = view2d->plane();
    auto slice = view2d->crosshair()[normalCoordinateIndex(plane)];

    auto widget = vtkPlanarSplitWidget::New();
    widget->SetCurrentRenderer(view->mainRenderer());
    widget->SetInteractor(view->renderWindow()->GetInteractor());
    widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
    widget->setOrientation(plane);
    widget->setShift(view2d->widgetDepth());
    widget->setSlice(slice);
    widget->CreateDefaultRepresentation();
    widget->SetEnabled(true);

    m_widgets.insert(view, widget);

    connect(view2d, SIGNAL(crosshairPlaneChanged(Plane,Nm)), this, SLOT(changeSlice(Plane, Nm)));
  }
  else
    if(isView3D(view))
    {
      auto widget = vtkImplicitPlaneWidget2::New();
      widget->SetCurrentRenderer(view->mainRenderer());
      widget->SetInteractor(view->renderWindow()->GetInteractor());
      widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
      widget->CreateDefaultRepresentation();
      widget->SetEnabled(true);

      m_widgets.insert(view, widget);
    }
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::unregisterView(RenderView *view)
{
  if(!m_widgets.keys().contains(view))
    return;

  if(isView2D(view))
  {
    auto view2d = dynamic_cast<View2D *>(view);
    disconnect(view2d, SIGNAL(crosshairPlaneChanged(Plane,Nm)), this, SLOT(changeSlice(Plane, Nm)));
  }

  m_widgets[view]->SetCurrentRenderer(nullptr);
  m_widgets[view]->SetInteractor(nullptr);
  m_widgets[view]->RemoveObserver(m_command);
  m_widgets[view]->Delete();
  m_widgets.remove(view);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setEnabled(bool enable)
{
  for(auto widget: m_widgets.values())
    widget->SetEnabled(enable);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setPlanePoints(vtkSmartPointer<vtkPoints> points)
{
  for(auto widget: m_widgets.values())
  {
    auto widget2d = dynamic_cast<vtkPlanarSplitWidget *>(widget);
    if(widget2d != nullptr)
      widget2d->setPoints(points);
    else
    {
      auto widget3d = dynamic_cast<vtkImplicitPlaneWidget2 *>(widget);
      if(widget3d != nullptr)
      {
        Q_ASSERT(m_mainWidget != VOLUME_WIDGET);

        double point1[3], point2[3], normal[3];
        points->GetPoint(0, point1);
        points->GetPoint(1, point2);
        double vector[3] = { point2[0]-point1[0], point2[1]-point1[1], point2[2]-point1[2] };
        double upVector[3] = { 0,0,0 };
        upVector[m_mainWidget] = 1;

        vtkMath::Cross(vector, upVector, normal);
        vtkImplicitPlaneRepresentation *rep = static_cast<vtkImplicitPlaneRepresentation*>(widget->GetRepresentation());
        rep->SetNormal(normal);
        rep->SetOrigin(point1);
        rep->Modified();
      }
    }
  }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPoints> PlanarSplitWidget::getPlanePoints() const
{
  vtkSmartPointer<vtkPoints> points = nullptr;

  for(auto widget: m_widgets.values())
  {
    auto widget2d = dynamic_cast<vtkPlanarSplitWidget *>(widget);
    if((widget2d != nullptr) && (widget2d->getOrientation() == toPlane(m_mainWidget)))
    {
      points = widget2d->getPoints();
      return points;
    }
  }

  return points;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::setSegmentationBounds(const Bounds &bounds)
{
  double dBounds[6]{bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};

  for(auto widget: m_widgets.values())
  {
    auto widget2d = dynamic_cast<vtkPlanarSplitWidget *>(widget);
    if(widget2d != nullptr)
    {
      widget2d->setSegmentationBounds(dBounds);
      widget2d->SetEnabled(true);
    }
    else
    {
      auto widget3d = dynamic_cast<vtkImplicitPlaneWidget2 *>(widget);
      if(widget3d != nullptr)
      {
        vtkImplicitPlaneRepresentation *rep = static_cast<vtkImplicitPlaneRepresentation*>(widget->GetRepresentation());
        rep->SetPlaceFactor(1);
        rep->PlaceWidget(dBounds);
        rep->SetOrigin((bounds[0]+bounds[1])/2,(bounds[2]+bounds[3])/2,(bounds[4]+bounds[5])/2);
        rep->UpdatePlacement();
        rep->OutlineTranslationOff();
        rep->ScaleEnabledOff();
        rep->OutsideBoundsOff();
        rep->SetVisibility(true);
        rep->Modified();
      }
    }
  }
}

//-----------------------------------------------------------------------------
bool PlanarSplitWidget::planeIsValid() const
{
  if(this->m_mainWidget == NONE)
    return false;

  if(this->m_mainWidget == VOLUME_WIDGET)
    return true;

  vtkSmartPointer<vtkPoints> points = this->getPlanePoints();
  double point1[3], point2[3];
  points->GetPoint(0, point1);
  points->GetPoint(1, point2);

  return ((point1[0] != point2[0]) || (point1[1] != point2[1]) || (point1[2] != point2[2]));
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPlane> PlanarSplitWidget::getImplicitPlane(const NmVector3 &spacing) const
{
  vtkSmartPointer<vtkPlane> plane = nullptr;

  if (m_mainWidget == NONE || !this->planeIsValid())
    return plane;

  plane = vtkSmartPointer<vtkPlane>::New();

  if(m_mainWidget != VOLUME_WIDGET)
  {
    vtkSmartPointer<vtkPoints> points = getPlanePoints();
    double point1[3], point2[3];
    points->GetPoint(0, point1);
    points->GetPoint(1, point2);

    double normal[3], upVector[3];
    double planeVector[3] = { point2[0]-point1[0] - spacing[0]/2,
                              point2[1]-point1[1] - spacing[1]/2,
                              point2[2]-point1[2] - spacing[2]/2};

    switch(m_mainWidget)
    {
      case AXIAL_WIDGET:
        upVector[0] = upVector[1] = 0;
        upVector[2] = 1;
        break;
      case CORONAL_WIDGET:
        upVector[0] = upVector[2] = 0;
        upVector[1] = 1;
        break;
      case SAGITTAL_WIDGET:
        upVector[1] = upVector[2] = 0;
        upVector[0] = 1;
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    vtkMath::Cross(planeVector, upVector, normal);
    plane->SetOrigin(point1);
    plane->SetNormal(normal);
    plane->Modified();
  }
  else
  {
    for(auto widget: m_widgets.values())
    {
      auto widget3d = dynamic_cast<vtkImplicitPlaneWidget2 *>(widget);
      if(widget3d != nullptr)
      {
        auto rep = dynamic_cast<vtkImplicitPlaneRepresentation*>(widget->GetRepresentation());
        rep->GetPlane(plane);
        break;
      }
    }
  }

  return plane;
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::changeSlice(Plane plane, Nm slice)
{
  auto view = qobject_cast<RenderView *>(sender());
  Q_ASSERT(view && m_widgets.keys().contains(view));

  auto widget = dynamic_cast<vtkPlanarSplitWidget *>(m_widgets[view]);
  widget->setSlice(slice);
}

//-----------------------------------------------------------------------------
void vtkSplitCommand::Execute(vtkObject *caller, unsigned long eventId, void *callData)
{
  auto widget2d = dynamic_cast<vtkPlanarSplitWidget *>(caller);
  if(widget2d != nullptr)
  {
    widget2d->RemoveObserver(this);
    m_widget->m_mainWidget = PlanarSplitWidget::toSplitType(widget2d->getOrientation());
  }
  else
  {
    auto widget3d = static_cast<vtkImplicitPlaneWidget2 *>(caller);
    if(widget3d != nullptr)
    {
      widget3d->RemoveObserver(this);
      m_widget->m_mainWidget = PlanarSplitWidget::SplitWidgetType::VOLUME_WIDGET;
    }
  }

  for(auto widget: m_widget->m_widgets.values())
  {
    if(widget == caller)
      continue;

    auto otherWidget2d = dynamic_cast<vtkPlanarSplitWidget *>(widget);
    if(otherWidget2d != nullptr)
    {
      otherWidget2d->disableWidget();
    }
    else
    {
      auto otherWidget3d = dynamic_cast<vtkImplicitPlaneWidget2 *>(widget);
      if(otherWidget3d != nullptr)
      {
        otherWidget3d->EnabledOff();
      }
    }

    m_widget->m_widgets.key(widget)->refresh();
  }
}
