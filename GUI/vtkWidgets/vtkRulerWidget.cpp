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

#include "GUI/vtkWidgets/vtkRulerWidget.h"

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

namespace EspINA
{
  vtkStandardNewMacro(vtkRulerWidget);
  
  //----------------------------------------------------------------------------
  vtkRulerWidget::vtkRulerWidget()
  : m_enabled(false)
  , m_plane(EspINA::UNDEFINED)
  , m_up(vtkSmartPointer<vtkAxisActor2D>::New())
  , m_right(vtkSmartPointer<vtkAxisActor2D>::New())
  , m_command(NULL)
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

    vtkMath::UninitializeBounds(m_bounds);
  }
  
  //----------------------------------------------------------------------------
  vtkRulerWidget::~vtkRulerWidget()
  {
    if (m_enabled)
      SetEnabled(false);
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget::CreateDefaultRepresentation()
  {
    // the representations is managed here because this is
    // a passive tool and doesn't need interactions
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget::SetEnabled(int value)
  {
    if (value == m_enabled || !CurrentRenderer)
      return;

    m_enabled = value;
    if (value)
    {
      m_command = new RulerCommand();
      m_command->setWidget(this);
      CurrentRenderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, m_command);
      CurrentRenderer->AddActor2D(m_up);
      CurrentRenderer->AddActor2D(m_right);
    }
    else
    {
      CurrentRenderer->GetActiveCamera()->RemoveObserver(m_command);
      m_command->SetReferenceCount(0);
      delete m_command;
      m_command = NULL;
      CurrentRenderer->RemoveActor2D(m_up);
      CurrentRenderer->RemoveActor2D(m_right);
      CurrentRenderer->GetRenderWindow()->Render();
    }
    m_up->SetVisibility(value);
    m_right->SetVisibility(value);
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget::drawActors()
  {
    if (!vtkMath::AreBoundsInitialized(m_bounds))
      return;

    // Beware, here be dragons...
    Nm point[3] = { 0, 0, 0 };
    switch(m_plane)
    {
      case AXIAL:
        point[0] = m_bounds[0];
        point[1] = m_bounds[3];
        point[2] = 0;
        transformCoordsWorldToNormView(point);
        m_up->SetPosition2(point[0], point[1]);
        m_right->SetPosition(point[0], point[1]);
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
        m_up->SetTitle(QObject::tr("Y: %1 nm").arg(m_bounds[3]-m_bounds[2]).toStdString().c_str());
        m_right->SetTitle(QObject::tr("X: %1 nm").arg(m_bounds[1]-m_bounds[0]).toStdString().c_str());
        break;
      case CORONAL:
        point[0] = m_bounds[0];
        point[1] = 0;
        point[2] = m_bounds[4];
        transformCoordsWorldToNormView(point);
        m_up->SetPosition(point[0], point[1]);
        point[0] = m_bounds[0];
        point[1] = 0;
        point[2] = m_bounds[5];
        transformCoordsWorldToNormView(point);
        m_up->SetPosition2(point[0], point[1]);
        m_right->SetPosition(point[0], point[1]);
        point[0] = m_bounds[1];
        point[1] = 0;
        point[2] = m_bounds[5];
        transformCoordsWorldToNormView(point);
        m_right->SetPosition2(point[0], point[1]);
        m_up->SetTitle(QObject::tr("Z: %1 nm").arg(m_bounds[5]-m_bounds[4]).toStdString().c_str());
        m_right->SetTitle(QObject::tr("X: %1 nm").arg(m_bounds[1]-m_bounds[0]).toStdString().c_str());
        break;
      case SAGITTAL:
        point[0] = 0;
        point[1] = m_bounds[3];
        point[2] = m_bounds[4];
        transformCoordsWorldToNormView(point);
        m_up->SetPosition(point[0], point[1]);
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
        m_up->SetTitle(QObject::tr("Z: %1 nm").arg(m_bounds[5]-m_bounds[4]).toStdString().c_str());
        m_right->SetTitle(QObject::tr("Y: %1 nm").arg(m_bounds[3]-m_bounds[2]).toStdString().c_str());
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    m_up->SetVisibility(true);
    m_up->Modified();

    m_right->SetVisibility(true);
    m_right->Modified();

    CurrentRenderer->GetRenderWindow()->Render();
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget::setBounds(Nm *bounds)
  {
    if (!vtkMath::AreBoundsInitialized(bounds))
    {
      m_up->SetVisibility(false);
      m_right->SetVisibility(false);
      return;
    }

    memcpy(m_bounds, bounds, sizeof(Nm)*6);
    drawActors();
  }

  //----------------------------------------------------------------------------
  void vtkRulerWidget::transformCoordsWorldToNormView(Nm *inout)
  {
    double worldPoint[4] = { inout[0], inout[1], inout[2], 0 };
    CurrentRenderer->SetWorldPoint(worldPoint);
    CurrentRenderer->WorldToDisplay();
    CurrentRenderer->GetDisplayPoint(inout);
    CurrentRenderer->DisplayToNormalizedDisplay(inout[0], inout[1]);
  }

  //----------------------------------------------------------------------------
  // RulerCommand class methods
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  RulerCommand::RulerCommand()
  : m_renderer(NULL)
  , m_widget(NULL)
  {
  }

  //----------------------------------------------------------------------------
  void RulerCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
  {
    if (strcmp("vtkOpenGLCamera", caller->GetClassName()) != 0)
      return;

    m_widget->drawActors();
  }

} /* namespace EspINA */
