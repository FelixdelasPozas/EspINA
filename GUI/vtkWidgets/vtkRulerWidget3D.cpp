/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include "vtkRulerWidget3D.h"

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


namespace EspINA
{
  vtkStandardNewMacro(vtkRulerWidget3D);

  //----------------------------------------------------------------------------
  vtkRulerWidget3D::vtkRulerWidget3D()
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
  vtkRulerWidget3D::~vtkRulerWidget3D()
  {
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget3D::CreateDefaultRepresentation()
  {
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget3D::SetEnabled(int value)
  {
    if (value == m_enabled || !CurrentRenderer)
      return;

    m_enabled = value;
    m_actor->SetVisibility(value);

    if(value)
    {
      if (m_actor->GetCamera() == NULL || CurrentRenderer->GetActiveCamera() != m_actor->GetCamera())
      {
        m_actor->SetCamera(CurrentRenderer->GetActiveCamera());
        m_actor->Modified();
      }
      CurrentRenderer->AddViewProp(m_actor);
    }
    else
      CurrentRenderer->RemoveViewProp(m_actor);

    CurrentRenderer->GetRenderWindow()->Render();
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget3D::setBounds(Nm *bounds)
  {
    if (!vtkMath::AreBoundsInitialized(bounds))
    {
      m_actor->SetVisibility(false);
      return;
    }

    m_actor->SetBounds(bounds);
    m_actor->SetXLabel(QObject::tr("X: %1 nm").arg(bounds[1]-bounds[0]).toStdString().c_str());
    m_actor->SetYLabel(QObject::tr("Y: %1 nm").arg(bounds[3]-bounds[2]).toStdString().c_str());
    m_actor->SetZLabel(QObject::tr("Z: %1 nm").arg(bounds[5]-bounds[4]).toStdString().c_str());
    m_actor->SetVisibility(true);
    m_actor->Modified();
    CurrentRenderer->GetRenderWindow()->Render();
  }

} /* namespace EspINA */
