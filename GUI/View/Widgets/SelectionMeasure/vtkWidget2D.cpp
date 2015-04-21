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
#include "vtkWidget2D.h"

// VTK
#include <vtkObjectFactory.h>
#include <vtkAxisActor2D.h>
#include <vtkRenderer.h>
#include <vtkMath.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>
#include <vtkRenderWindow.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;

vtkStandardNewMacro(vtkWidget2D);

//----------------------------------------------------------------------------
vtkWidget2D::vtkWidget2D()
: m_enabled(false)
, m_plane(Plane::UNDEFINED)
, m_up(vtkSmartPointer<vtkAxisActor2D>::New())
, m_right(vtkSmartPointer<vtkAxisActor2D>::New())
{
  m_up->SetVisibility(false);
  m_up->SetPoint1(0,0);
  m_up->SetPoint2(0,0);
  m_up->SetFontFactor(0.5);
  m_up->AdjustLabelsOff();
  m_up->SetNumberOfLabels(2);
  m_up->SetNumberOfMinorTicks(0);
  m_up->SetPickable(false);

  vtkTextProperty *uplabels = m_up->GetLabelTextProperty();
  uplabels->SetOpacity(0);
  uplabels->Modified();

  m_right->SetVisibility(false);
  m_right->SetPoint1(0,0);
  m_right->SetPoint2(0,0);
  m_right->SetFontFactor(0.5);
  m_right->AdjustLabelsOff();
  m_right->SetNumberOfLabels(2);
  m_right->SetNumberOfMinorTicks(0);
  m_right->SetPickable(false);

  vtkTextProperty *rightlabels = m_right->GetLabelTextProperty();
  rightlabels->SetOpacity(0);
  rightlabels->Modified();
}

//----------------------------------------------------------------------------
vtkWidget2D::~vtkWidget2D()
{
  SetEnabled(false);
}

//----------------------------------------------------------------------------
void vtkWidget2D::CreateDefaultRepresentation()
{

}

//----------------------------------------------------------------------------
void vtkWidget2D::SetEnabled(int enabled)
{
  if (enabled != m_enabled && CurrentRenderer)
  {
    m_enabled = enabled;

    if (enabled)
    {
      CurrentRenderer->AddActor2D(m_up);
      CurrentRenderer->AddActor2D(m_right);

      drawActors();
    }
    else
    {
      CurrentRenderer->RemoveActor2D(m_up);
      CurrentRenderer->RemoveActor2D(m_right);

      CurrentRenderer->GetRenderWindow()->Render();
    }
  }
}

//----------------------------------------------------------------------------
void vtkWidget2D::drawActors()
{
  bool validActors = m_plane != Plane::UNDEFINED
                  && m_bounds.areValid()
                  && m_enabled;
  if (validActors)
  {
    // Beware, here be dragons...
    Nm point[3] = { 0, 0, 0 };
    switch(m_plane)
    {
      case Plane::XY:
        point[0] = m_bounds[0];
        point[1] = m_bounds[3];
        point[2] = 0;
        transformCoordsWorldToNormView(point);
        m_up   ->SetPosition2(point[0], point[1]);
        m_right->SetPosition (point[0], point[1]);
        point[0] = m_bounds[0];
        point[1] = m_bounds[2];
        point[2] = 0;
        transformCoordsWorldToNormView(point);
        m_up->SetPosition(point[0], point[1]);
        point[0] = m_bounds[1];
        point[1] = m_bounds[3];
        point[2] = 0;
        transformCoordsWorldToNormView(point);
        m_right->SetPosition2(point[0], point[1]);
        setTitle(m_up,    Axis::Y);
        setTitle(m_right, Axis::X);
        break;
      case Plane::XZ:
        point[0] = m_bounds[0];
        point[1] = 0;
        point[2] = m_bounds[4];
        transformCoordsWorldToNormView(point);
        m_up->SetPosition(point[0], point[1]);
        point[0] = m_bounds[0];
        point[1] = 0;
        point[2] = m_bounds[5];
        transformCoordsWorldToNormView(point);
        m_up   ->SetPosition2(point[0], point[1]);
        m_right->SetPosition (point[0], point[1]);
        point[0] = m_bounds[1];
        point[1] = 0;
        point[2] = m_bounds[5];
        transformCoordsWorldToNormView(point);
        m_right->SetPosition2(point[0], point[1]);
        setTitle(m_up,    Axis::Z);
        setTitle(m_right, Axis::X);
        break;
      case Plane::YZ:
        point[0] = 0;
        point[1] = m_bounds[3];
        point[2] = m_bounds[4];
        transformCoordsWorldToNormView(point);
        m_up   ->SetPosition (point[0], point[1]);
        m_right->SetPosition2(point[0], point[1]);
        point[0] = 0;
        point[1] = m_bounds[3];
        point[2] = m_bounds[5];
        transformCoordsWorldToNormView(point);
        m_up->SetPosition2(point[0], point[1]);
        point[0] = 0;
        point[1] = m_bounds[2];
        point[2] = m_bounds[4];
        transformCoordsWorldToNormView(point);
        m_right->SetPosition(point[0], point[1]);
        setTitle(m_up,    Axis::Z);
        setTitle(m_right, Axis::Y);
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    m_up   ->Modified();
    m_right->Modified();
  }

  m_up   ->SetVisibility(validActors);
  m_right->SetVisibility(validActors);
  CurrentRenderer->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void vtkWidget2D::setBounds(Bounds bounds)
{
  m_bounds = bounds;

  drawActors();
}

//----------------------------------------------------------------------------
void vtkWidget2D::transformCoordsWorldToNormView(Nm *inout)
{
  double worldPoint[4] = { inout[0], inout[1], inout[2], 0 };
  CurrentRenderer->SetWorldPoint(worldPoint);
  CurrentRenderer->WorldToDisplay();
  CurrentRenderer->GetDisplayPoint(inout);
  CurrentRenderer->DisplayToNormalizedDisplay(inout[0], inout[1]);
}

//----------------------------------------------------------------------------
void vtkWidget2D::setTitle(vtkAxisActor2D *actor, const Axis axis)
{
  auto label   = axis == Axis::X?"X":(axis == Axis::Y?"Y":"Z");
  auto measure = m_bounds[2*idx(axis)+1]-m_bounds[2*idx(axis)];

  actor->SetTitle(QObject::tr("%1: %2 nm").arg(label).arg(measure).toUtf8());
}
