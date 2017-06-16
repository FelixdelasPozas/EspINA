/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "vtkSkeletonWidget.h"
#include "vtkSkeletonWidgetRepresentation.h"

// VTK
#include <vtkObjectFactory.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCommand.h>
#include <vtkWidgetEvent.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>

// Qt
#include <QApplication>
#include <QPixmap>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

vtkStandardNewMacro(vtkSkeletonWidget);

//-----------------------------------------------------------------------------
vtkSkeletonWidget::vtkSkeletonWidget()
: m_widgetState  {vtkSkeletonWidget::Start}
, m_orientation  {Plane::UNDEFINED}
, m_slice        {-1}
, m_shift        {1}
, m_color        {QColor{254,254,154}}
{
  ManagesCursor = false; // from Superclass
  CreateDefaultRepresentation();

  createCursors();
}

//-----------------------------------------------------------------------------
vtkSkeletonWidget::~vtkSkeletonWidget()
{
  // restore the pointer if the widget has changed it
  if (ManagesCursor == true)
  {
    QApplication::restoreOverrideCursor();
  }

  if(GetEnabled())
  {
    SetEnabled(false);
  }

  if(WidgetRep && CurrentRenderer)
  {
    reinterpret_cast<vtkSkeletonWidgetRepresentation*>(WidgetRep)->ClearAllNodes();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetEnabled(int enabling)
{
  if (WidgetRep)
  {
    if (enabling)
    {
      WidgetRep->VisibilityOn();
    }
    else
    {
      WidgetRep->VisibilityOff();
    }
  }

  Superclass::SetEnabled(enabling);
  Render();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::CreateDefaultRepresentation()
{
  if (WidgetRep == nullptr)
  {
    auto rep = vtkSkeletonWidgetRepresentation::New();
    rep->SetOrientation(m_orientation);
    rep->SetSlice(m_slice);
    rep->SetShift(m_shift);
    rep->SetTolerance(0); // handled by event handler
    rep->SetColor(m_color);
    WidgetRep = rep;
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  Superclass::PrintSelf(os, indent);

  os << indent << "WidgetState: " << m_widgetState << endl;
  os << indent << "Orientation: " << (int) m_orientation << endl;
  os << indent << "Shift in Z: " << m_shift << endl;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::Initialize(vtkSmartPointer<vtkPolyData> pd)
{
  if (!WidgetRep)
  {
    CreateDefaultRepresentation();
    WidgetRep->SetRenderer(GetCurrentRenderer());
  }

  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);

  if (pd == nullptr)
  {
    rep->ClearAllNodes();
    Render();
    rep->NeedToRenderOff();
    rep->VisibilityOff();
    m_widgetState = vtkSkeletonWidget::Define;
  }
  else
  {
    rep->Initialize(pd);
    m_widgetState = vtkSkeletonWidget::Start;
  }

  if (GetCurrentRenderer() != nullptr)
  {
    if (pd == nullptr)
    {
      SetCursor(vtkSkeletonWidgetRepresentation::Outside);
    }
    else
    {
      int X, Y;
      Interactor->GetEventPosition(X,Y);
      rep->ComputeInteractionState(X, Y);
      int wState = WidgetRep->GetInteractionState();

      SetCursor(wState);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetOrientation(Plane plane)
{
  if (m_orientation == Plane::UNDEFINED)
  {
    m_orientation = plane;

    if (WidgetRep)
    {
      reinterpret_cast<vtkSkeletonWidgetRepresentation*>(WidgetRep)->SetOrientation(plane);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::addPoint()
{
  if(m_widgetState != vtkSkeletonWidget::Define) return;

  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation*>(WidgetRep);

  double worldPos[3];
  int X,Y;
  Interactor->GetEventPosition(X,Y);
  rep->setIgnoreCursorNode(true);
  rep->ComputeInteractionState(X,Y);
  auto State = rep->GetInteractionState();
  rep->setIgnoreCursorNode(false);

  switch(State)
  {
    case vtkSkeletonWidgetRepresentation::NearContour:
      rep->AddNodeOnContour(X, Y);
      break;
    case vtkSkeletonWidgetRepresentation::NearPoint:
      rep->TryToJoin(X, Y);
      break;
    case vtkSkeletonWidgetRepresentation::Outside:
    default:
      if(!rep->GetActiveNodeWorldPosition(worldPos))
        rep->AddNodeAtDisplayPosition(X, Y);
      rep->AddNodeAtDisplayPosition(X, Y);
      break;
  }

  SetCursor(WidgetRep->GetInteractionState());

  if (WidgetRep->GetNeedToRender())
  {
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::movePoint()
{
  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);

  int X,Y;
  Interactor->GetEventPosition(X,Y);

  switch (m_widgetState)
  {
    case vtkSkeletonWidget::Start:
      if (rep->IsNearNode(X, Y))
      {
        rep->ActivateNode(X, Y);

        m_widgetState = vtkSkeletonWidget::Manipulate;
        Superclass::StartInteraction();
        InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
        StartInteraction();
      }
      break;
    case vtkSkeletonWidget::Manipulate:
      rep->SetActiveNodeToDisplayPosition(X, Y);

      m_widgetState = vtkSkeletonWidget::Start;
      Superclass::EndInteraction();
      InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
      EndInteraction();
      break;
    case vtkSkeletonWidget::Define:
      rep->SetActiveNodeToDisplayPosition(X, Y);
      InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;
    default:
      break;
  }

  if(m_ignoreCursor) rep->setIgnoreCursorNode(true);
  rep->ComputeInteractionState(X, Y);
  if(m_ignoreCursor) rep->setIgnoreCursorNode(false);
  SetCursor(WidgetRep->GetInteractionState());

  if (WidgetRep->GetNeedToRender())
  {
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::deletePoint()
{
  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
  int X,Y;
  Interactor->GetEventPosition(X,Y);

  if(rep->IsNearNode(X, Y))
  {
    rep->ActivateNode(X, Y);
    rep->DeleteCurrentNode();
  }

  if (rep->GetNeedToRender())
  {
    Render();
    rep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::stop()
{
  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);

  switch (m_widgetState)
  {
    case vtkSkeletonWidget::Define:
    {
      rep->DeleteCurrentNode();

      Superclass::EndInteraction();
      InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
      EndInteraction();
    }
      break;
    case vtkSkeletonWidget::Manipulate:
      m_widgetState = vtkSkeletonWidget::Start;
      Superclass::EndInteraction();
      InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
      EndInteraction();
      break;
    case vtkSkeletonWidget::Start:
      rep->DeleteCurrentNode();
      break;
    default:
      break;
  }

  if (WidgetRep->GetNeedToRender())
  {
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
//void vtkSkeletonWidget::KeyPressAction(vtkAbstractWidget *w)
//{
//  auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
//  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(self->WidgetRep);
//
//  auto key = std::string(self->Interactor->GetKeySym());
//
//  int X = self->Interactor->GetEventPosition()[0];
//  int Y = self->Interactor->GetEventPosition()[1];
//
//  if (("Alt_L" == key) && self->m_widgetState == vtkSkeletonWidget::Start)
//  {
//    if (rep->DeleteCurrentNode())
//    {
//      self->m_modified = true;
//    }
//
//    self->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
//  }
//
//  self->WidgetRep->ComputeInteractionState(X, Y);
//  int state = self->WidgetRep->GetInteractionState();
//  self->SetCursor(state);
//
//  if (self->WidgetRep->GetNeedToRender())
//  {
//    self->Render();
//    self->WidgetRep->NeedToRenderOff();
//  }
//}

//-----------------------------------------------------------------------------
//void vtkSkeletonWidget::ReleaseKeyPressAction(vtkAbstractWidget *w)
//{
//  auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
//  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(self->WidgetRep);
//
//  auto key = std::string(self->Interactor->GetKeySym());
//
//  int X = self->Interactor->GetEventPosition()[0];
//  int Y = self->Interactor->GetEventPosition()[1];
//
//  self->WidgetRep->ComputeInteractionState(X, Y);
//  int state = self->WidgetRep->GetInteractionState();
//  self->SetCursor(state);
//
//  if ("Tab" == key)
//  {
//    switch (self->m_widgetState)
//    {
//      case vtkSkeletonWidget::Define:
//      {
//        self->m_widgetState = vtkSkeletonWidget::Start;
//        vtkSkeletonWidgetRepresentation::SkeletonNode *nullnode = nullptr;
//        if (rep->TryToJoin(X, Y))
//        {
//          self->m_modified = true;
//        }
//        rep->ActivateNode(nullnode);
//
//        self->Superclass::EndInteraction();
//        self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
//        self->EndInteraction();
//      }
//        break;
//      case vtkSkeletonWidget::Start:
//        self->m_widgetState = vtkSkeletonWidget::Define;
//        self->Superclass::StartInteraction();
//        self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
//        self->StartInteraction();
//
//        switch (state)
//        {
//          case vtkSkeletonWidgetRepresentation::NearContour:
//            if (rep->AddNodeOnContour(X, Y))
//            {
//              self->m_modified = true;
//            }
//            break;
//          case vtkSkeletonWidgetRepresentation::NearPoint:
//            rep->ActivateNode(X, Y);
//            if (!rep->IsPointTooClose(X, Y))
//            {
//              rep->AddNodeAtDisplayPosition(X, Y);
//              self->m_modified = true;
//            }
//            break;
//          case vtkSkeletonWidgetRepresentation::Outside:
//            rep->AddNodeAtDisplayPosition(X, Y);
//            self->m_modified = true;
//            break;
//          default:
//            break;
//        }
//
//        rep->VisibilityOn();
//        self->EventCallbackCommand->SetAbortFlag(1);
//        self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
//        break;
//      case vtkSkeletonWidget::Manipulate:
//      default:
//        break;
//    }
//  }
//
//  self->SetCursor(state);
//
//  if (self->WidgetRep->GetNeedToRender())
//  {
//    self->Render();
//    self->WidgetRep->NeedToRenderOff();
//  }
//}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetCursor(int State)
{
  switch (m_widgetState)
  {
    case vtkSkeletonWidget::Define:
    {
      switch(State)
      {
        case vtkSkeletonWidgetRepresentation::NearContour:
        case vtkSkeletonWidgetRepresentation::NearPoint:
          if (!ManagesCursor)
          {
            QApplication::setOverrideCursor(m_crossPlusCursor);
            ManagesCursorOn();
          }
          else
          {
            QApplication::changeOverrideCursor(m_crossPlusCursor);
          }
        break;
        default:
          if(ManagesCursor)
          {
            QApplication::restoreOverrideCursor();
            ManagesCursorOff();
          }
          break;
      }
    }

//      if (ManagesCursor)
//      {
//        ManagesCursorOff();
//        QApplication::restoreOverrideCursor();
//      }
      break;
    case vtkSkeletonWidget::Manipulate:
      if (!ManagesCursor)
      {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
        ManagesCursorOn();
      }
      break;
    case vtkSkeletonWidget::Start:
      switch (State)
      {
        case vtkSkeletonWidgetRepresentation::NearPoint:
          if (!ManagesCursor)
          {
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
            ManagesCursorOn();
          }
          else
          {
            QApplication::changeOverrideCursor(Qt::PointingHandCursor);
          }
          break;
        case vtkSkeletonWidgetRepresentation::NearContour:
        {
          if (!ManagesCursor)
          {
            QApplication::setOverrideCursor(m_crossPlusCursor);
            ManagesCursorOn();
          }
          else
          {
            QApplication::changeOverrideCursor(m_crossPlusCursor);
          }
        }
          break;
        case vtkSkeletonWidgetRepresentation::Outside:
        default:
          if (ManagesCursor)
          {
            ManagesCursor = false;
            QApplication::restoreOverrideCursor();
          }
          break;
      }

      break;
    default:
      break;
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::changeSlice(Plane plane, Nm value)
{
  if (m_orientation != plane) return;

  m_slice = value;

  if (m_widgetState == vtkSkeletonWidget::Define)
  {
    double pos[3]{0,0,0};
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
    if (rep->GetActiveNodeWorldPosition(pos))
    {
      pos[normalCoordinateIndex(m_orientation)] = value;
      rep->SetActiveNodeToWorldPosition(pos, false);
    }
  }

  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
  rep->SetSlice(value);

  if (Interactor != nullptr)
  {
    int X, Y;
    Interactor->GetEventPosition(X, Y);

    rep->setIgnoreCursorNode(true);
    rep->ComputeInteractionState(X, Y);
    auto state = rep->GetInteractionState();
    SetCursor(state);
    rep->setIgnoreCursorNode(false);

    if (m_widgetState == vtkSkeletonWidget::Start)
    {
      if (rep->IsNearNode(X, Y))
      {
        rep->ActivateNode(X, Y);
      }
      else
      {
        rep->DeactivateNode();
      }
    }
  }

  if (WidgetRep->GetNeedToRender())
  {
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkSkeletonWidget::getSkeleton()
{
  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
  return rep->GetRepresentationPolyData();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetShift(const Nm shift)
{
  if (m_shift != shift)
  {
    m_shift = shift;

    if (WidgetRep == nullptr)
    {
      CreateDefaultRepresentation();
      return;
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->SetShift(shift);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setRepresentationColor(const QColor &color)
{
  if (m_color != color)
  {
    m_color = color;

    if (WidgetRep == nullptr)
    {
      CreateDefaultRepresentation();
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->SetColor(color);
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::UpdateRepresentation()
{
  if (!WidgetRep) return;

  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
  rep->BuildRepresentation();
  rep->NeedToRenderOff();
  Render();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetSpacing(const NmVector3 &spacing)
{
  if(m_spacing != spacing)
  {
    m_spacing = spacing;

    if(WidgetRep == nullptr)
    {
      CreateDefaultRepresentation();
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->SetSpacing(spacing);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setCurrentOperationMode(const int mode)
{
  if(m_widgetState != mode)
  {
    // TODO acciones necesarias cuando se cambia de modo
    m_widgetState = mode;
  }
}

//-----------------------------------------------------------------------------
const int vtkSkeletonWidget::currentOperationMode() const
{
  return m_widgetState;
}

//-----------------------------------------------------------------------------
const unsigned int vtkSkeletonWidget::numberOfPoints() const
{
  if(WidgetRep)
  {
    return reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->GetNumberOfNodes();
  }

  return 0;
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidget::selectNode()
{
  if(WidgetRep)
  {
    int X, Y;
    Interactor->GetEventPosition(X, Y);

    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
    return (rep->IsNearNode(X, Y) && rep->ActivateNode(X, Y));
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::createCursors()
{
  QPixmap crossMinusPixmap, crossPlusPixmap, crossCheckPixmap;
  crossMinusPixmap.load(":espina/cross-minus.png", "PNG", Qt::ColorOnly);
  crossPlusPixmap.load(":espina/cross-plus.png", "PNG", Qt::ColorOnly);
  crossCheckPixmap.load(":espina/cross-check.png", "PNG", Qt::ColorOnly);

  m_crossMinusCursor = QCursor(crossMinusPixmap, -1, -1);
  m_crossPlusCursor  = QCursor(crossPlusPixmap, -1, -1);
  m_crossCheckCursor = QCursor(crossCheckPixmap, -1, -1);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::cleanup()
{
  vtkSkeletonWidgetRepresentation::cleanup();
}
