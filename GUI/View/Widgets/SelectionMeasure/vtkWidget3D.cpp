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

#include "vtkWidget3D.h"

// VTK
#include <vtkObjectFactory.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkAxisActor2D.h>
#include <vtkMath.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <QDebug>
#include <vtkTextProperty.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;


vtkStandardNewMacro(vtkWidget3D);

//----------------------------------------------------------------------------
vtkWidget3D::vtkWidget3D()
: m_enabled(false)
, m_actor(vtkSmartPointer<vtkCubeAxesActor2D>::New())
{
  m_actor->SetBounds(0,0,0,0,0,0);
  m_actor->SetVisibility(false);
  m_actor->SetDragable(false);
  m_actor->SetPickable(false);
  m_actor->SetFlyModeToClosestTriad();
  m_actor->SetFontFactor(1);
  m_actor->SetNumberOfLabels(0);
  m_actor->DragableOff();

  // don't want axis labels, as the measure goes in the axis title
  vtkTextProperty *labelText = m_actor->GetAxisLabelTextProperty();
  labelText->SetOpacity(0);
  labelText->Modified();
}

//----------------------------------------------------------------------------
vtkWidget3D::~vtkWidget3D()
{
}

//----------------------------------------------------------------------------
void vtkWidget3D::CreateDefaultRepresentation()
{
}

//----------------------------------------------------------------------------
void vtkWidget3D::SetEnabled(int enabled)
{
  if (enabled != m_enabled && CurrentRenderer)
  {
    m_enabled = enabled;

    m_actor->SetVisibility(enabled);

    if(enabled)
    {
      if (m_actor->GetCamera() == NULL || CurrentRenderer->GetActiveCamera() != m_actor->GetCamera())
      {
        m_actor->SetCamera(CurrentRenderer->GetActiveCamera());
        m_actor->Modified();
      }
      CurrentRenderer->AddViewProp(m_actor);
    }
    else
    {
      CurrentRenderer->RemoveViewProp(m_actor);
    }

    CurrentRenderer->GetRenderWindow()->Render();
  }
}

//----------------------------------------------------------------------------
void vtkWidget3D::setBounds(Bounds bounds)
{
  auto validActor = bounds.areValid();

  if (validActor)
  {
    double actorBounds[6]{ bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5] };
    m_actor->SetBounds(actorBounds);

    m_actor->SetXLabel(label(Axis::X, bounds));
    m_actor->SetYLabel(label(Axis::Y, bounds));
    m_actor->SetZLabel(label(Axis::Z, bounds));

    m_actor->Modified();
  }
  m_actor->SetVisibility(validActor);
  CurrentRenderer->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
QByteArray vtkWidget3D::label(const Axis axis, const Bounds &bounds)
{
  auto label   = axis == Axis::X?"X":(axis == Axis::Y?"Y":"Z");
  auto measure = bounds[2*idx(axis)+1]-bounds[2*idx(axis)];

  return QObject::tr("%1: %2 nm").arg(label).arg(measure).toUtf8();
}
