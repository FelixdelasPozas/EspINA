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

namespace ESPINA
{
  vtkStandardNewMacro(vtkSkeletonWidget);

  //-----------------------------------------------------------------------------
  vtkSkeletonWidget::vtkSkeletonWidget()
  : m_widgetState     {vtkSkeletonWidget::Start}
  , m_currentHandle   {0}
  , m_orientation     {Plane::UNDEFINED}
  , m_drawTolerance   {40}
  , m_slice           {-1}
  , m_shift           {1}
  , m_color           {QColor{254,254,154}}
  , m_parent          {nullptr}
  {
    this->ManagesCursor = 0; // from Superclass
    this->CreateDefaultRepresentation();

    // These are the event callbacks supported by this widget
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkWidgetEvent::Translate, this, vtkSkeletonWidget::TranslateAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent, vtkWidgetEvent::Translate, this, vtkSkeletonWidget::TranslateAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent, vtkWidgetEvent::ModifyEvent, this, vtkSkeletonWidget::StopAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, vtkSkeletonWidget::MoveAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent, vtkWidgetEvent::Select, this, vtkSkeletonWidget::KeyPressAction);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyReleaseEvent, vtkWidgetEvent::EndSelect, this, vtkSkeletonWidget::ReleaseKeyPressAction);

    QPixmap crossMinusPixmap, crossPlusPixmap, crossCheckPixmap;
    crossMinusPixmap.load(":espina/cross-minus.png", "PNG", Qt::ColorOnly);
    crossPlusPixmap.load(":espina/cross-plus.png", "PNG", Qt::ColorOnly);
    crossCheckPixmap.load(":espina/cross-check.png", "PNG", Qt::ColorOnly);

    this->m_crossMinusCursor = QCursor(crossMinusPixmap, -1, -1);
    this->m_crossPlusCursor = QCursor(crossPlusPixmap, -1, -1);
    this->m_crossCheckCursor = QCursor(crossCheckPixmap, -1, -1);
  }
  
  //-----------------------------------------------------------------------------
  vtkSkeletonWidget::~vtkSkeletonWidget()
  {
    // restore the pointer if the widget has changed it
    if (this->ManagesCursor == true)
      QApplication::restoreOverrideCursor();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::SetEnabled(int enabling)
  {
    if (enabling)
    {
      if (this->m_widgetState == vtkSkeletonWidget::Start)
      {
        reinterpret_cast<vtkSkeletonWidgetRepresentation*>(this->WidgetRep)->VisibilityOff();
      }
      else
      {
        reinterpret_cast<vtkSkeletonWidgetRepresentation*>(this->WidgetRep)->VisibilityOn();
      }
    }

    this->Superclass::SetEnabled(enabling);
    this->Render();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::CreateDefaultRepresentation()
  {
    if (this->WidgetRep == nullptr)
    {
      auto rep = vtkSkeletonWidgetRepresentation::New();
      reinterpret_cast<vtkSkeletonWidgetRepresentation *>(rep)->SetOrientation(m_orientation);
      reinterpret_cast<vtkSkeletonWidgetRepresentation *>(rep)->SetSlice(m_slice);
      reinterpret_cast<vtkSkeletonWidgetRepresentation *>(rep)->SetShift(m_shift);
      reinterpret_cast<vtkSkeletonWidgetRepresentation *>(rep)->SetTolerance(m_drawTolerance);
      reinterpret_cast<vtkSkeletonWidgetRepresentation *>(rep)->SetColor(m_color);
      this->WidgetRep = rep;
    }
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::PrintSelf(ostream& os, vtkIndent indent)
  {
    //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
    this->Superclass::PrintSelf(os, indent);

    os << indent << "WidgetState: " << this->m_widgetState << endl;
    os << indent << "CurrentHandle: " << this->m_currentHandle << endl;
    os << indent << "Orientation: " << (int)this->m_orientation << endl;
    os << indent << "Tolerance: " << this->m_drawTolerance << endl;
    os << indent << "Shift in Z: " << this->m_shift << endl;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::Initialize(vtkSmartPointer<vtkPolyData> pd)
  {
    if (!this->WidgetRep)
      this->CreateDefaultRepresentation();

    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep);

    if (pd == nullptr)
    {
      rep->ClearAllNodes();
      this->Render();
      rep->NeedToRenderOff();
      rep->VisibilityOff();
    }
    else
    {
      rep->Initialize(pd);
    }

    this->m_widgetState = vtkSkeletonWidget::Start;

    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];
    this->WidgetRep->ComputeInteractionState(X,Y);
    int wState = this->WidgetRep->GetInteractionState();

    if (pd == nullptr)
      this->SetCursor(vtkSkeletonWidgetRepresentation::Outside);
    else
      this->SetCursor(wState);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::SetOrientation(Plane plane)
  {
    if(this->m_orientation == Plane::UNDEFINED)
    {
      this->m_orientation = plane;

      if(this->WidgetRep)
        reinterpret_cast<vtkSkeletonWidgetRepresentation*>(this->WidgetRep)->SetOrientation(plane);
    }
  }

  //-----------------------------------------------------------------------------
  Plane vtkSkeletonWidget::GetOrientation()
  {
    return m_orientation;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::MoveAction(vtkAbstractWidget *w)
  {
    auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation*>(self->WidgetRep);

    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    switch(self->m_widgetState)
    {
      case vtkSkeletonWidget::Define:
        if (!rep->IsPointTooClose(X,Y))
        {
          rep->AddNodeAtDisplayPosition(X,Y);
          self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        }
        break;
      case vtkSkeletonWidget::Manipulate:
        rep->SetActiveNodeToDisplayPosition(X,Y);
        self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        break;
      case vtkSkeletonWidget::Start:
        if(rep->IsNearNode(X,Y))
          rep->ActivateNode(X,Y);
        else
          rep->DeactivateNode();
        break;
      default:
        break;
    }

    if (self->WidgetRep->GetNeedToRender())
    {
      self->Render();
      self->WidgetRep->NeedToRenderOff();
    }

    rep->ComputeInteractionState(X,Y);
    int wState = self->WidgetRep->GetInteractionState();
    self->SetCursor(wState);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::TranslateAction(vtkAbstractWidget *w)
  {
    auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(self->WidgetRep);

    auto X = self->Interactor->GetEventPosition()[0];
    auto Y = self->Interactor->GetEventPosition()[1];

    switch(self->m_widgetState)
    {
      case vtkSkeletonWidget::Start:
        if(rep->IsNearNode(X,Y))
        {
          rep->ActivateNode(X,Y);

          self->m_widgetState = vtkSkeletonWidget::Manipulate;
          self->Superclass::StartInteraction();
          self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
          self->StartInteraction();
        }
        break;
      case vtkSkeletonWidget::Manipulate:
        rep->SetActiveNodeToDisplayPosition(X,Y);

        self->m_widgetState = vtkSkeletonWidget::Start;
        self->Superclass::EndInteraction();
        self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
        self->EndInteraction();
        break;
      case vtkSkeletonWidget::Define:
      default:
        break;
    }

  }
  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::StopAction(vtkAbstractWidget *w)
  {
    auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(self->WidgetRep);

    switch(self->m_widgetState)
    {
      case vtkSkeletonWidget::Define:
      {
        self->m_widgetState = vtkSkeletonWidget::Start;
        rep->DeactivateNode();

        self->Superclass::EndInteraction();
        self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
        self->EndInteraction();
      }
        break;
      case vtkSkeletonWidget::Manipulate:
        self->m_widgetState = vtkSkeletonWidget::Start;
        self->Superclass::EndInteraction();
        self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
        self->EndInteraction();
        break;
      case vtkSkeletonWidget::Start:
      default:
        break;
    }

    if (self->WidgetRep->GetNeedToRender())
    {
      self->Render();
      self->WidgetRep->NeedToRenderOff();
    }
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::ResetAction(vtkAbstractWidget *w)
  {
    auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
    self->Initialize(nullptr);

    self->Render();
    self->WidgetRep->NeedToRenderOff();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::KeyPressAction(vtkAbstractWidget *w)
  {
    auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(self->WidgetRep);

    auto key = std::string(self->Interactor->GetKeySym());

    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    if("BackSpace" == key)
    {
      self->m_widgetState = vtkSkeletonWidget::Start;
      self->EnabledOff();
      self->ResetAction(w);
      self->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
      self->EnabledOn();
    }

    if(("Delete" == key) && self->m_widgetState == vtkSkeletonWidget::Start)
    {
      rep->DeleteCurrentNode();
      self->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }

    self->WidgetRep->ComputeInteractionState(X, Y);
    int state = self->WidgetRep->GetInteractionState();
    self->SetCursor(state);

    if (self->WidgetRep->GetNeedToRender())
    {
      self->Render();
      self->WidgetRep->NeedToRenderOff();
    }
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::ReleaseKeyPressAction(vtkAbstractWidget *w)
  {
    auto self = reinterpret_cast<vtkSkeletonWidget*>(w);
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(self->WidgetRep);

    auto key = std::string(self->Interactor->GetKeySym());

    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    self->WidgetRep->ComputeInteractionState(X, Y);
    int state = self->WidgetRep->GetInteractionState();
    self->SetCursor(state);

    if("Control_L" == key)
    {
      switch(self->m_widgetState)
      {
        case vtkSkeletonWidget::Define:
        {
          self->m_widgetState = vtkSkeletonWidget::Start;
          vtkSkeletonWidgetRepresentation::SkeletonNode *nullnode = nullptr;
          rep->TryToJoin(X,Y);
          rep->ActivateNode(nullnode);

          self->Superclass::EndInteraction();
          self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
          self->EndInteraction();
        }
          break;
        case vtkSkeletonWidget::Start:
          self->m_widgetState = vtkSkeletonWidget::Define;
          self->Superclass::StartInteraction();
          self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
          self->StartInteraction();

          switch(state)
          {
            case vtkSkeletonWidgetRepresentation::NearContour:
              rep->AddNodeOnContour(X,Y);
              break;
            case vtkSkeletonWidgetRepresentation::NearPoint:
              rep->ActivateNode(X,Y);
              if(!rep->IsPointTooClose(X,Y))
                rep->AddNodeAtDisplayPosition(X,Y);
              break;
            case vtkSkeletonWidgetRepresentation::Outside:
              rep->AddNodeAtDisplayPosition(X,Y);
              break;
            default:
              break;
          }

          rep->VisibilityOn();
          self->EventCallbackCommand->SetAbortFlag(1);
          self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
          break;
        case vtkSkeletonWidget::Manipulate:
        default:
          break;
      }
    }

    self->SetCursor(state);

    if (self->WidgetRep->GetNeedToRender())
    {
      self->Render();
      self->WidgetRep->NeedToRenderOff();
    }
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::SetCursor(int State)
  {
    switch(this->m_widgetState)
    {
      case vtkSkeletonWidget::Define:
        if(this->ManagesCursor)
        {
          this->ManagesCursorOff();
          QApplication::restoreOverrideCursor();
        }
        break;
      case vtkSkeletonWidget::Manipulate:
        if(!this->ManagesCursor)
        {
          QApplication::setOverrideCursor(Qt::PointingHandCursor);
          this->ManagesCursorOn();
        }
        break;
      case vtkSkeletonWidget::Start:
        switch (State)
        {
          case vtkSkeletonWidgetRepresentation::NearPoint:
            if(!this->ManagesCursor)
            {
              QApplication::setOverrideCursor(Qt::PointingHandCursor);
              this->ManagesCursorOn();
            }
            else
              QApplication::changeOverrideCursor(Qt::PointingHandCursor);
            break;
          case vtkSkeletonWidgetRepresentation::NearContour:
          {
            if(!this->ManagesCursor)
            {
              QApplication::setOverrideCursor(m_crossPlusCursor);
              this->ManagesCursorOn();
            }
            else
              QApplication::changeOverrideCursor(m_crossPlusCursor);
          }
            break;
          case vtkSkeletonWidgetRepresentation::Outside:
          default:
            if (this->ManagesCursor)
            {
              this->ManagesCursor = false;
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
    if(m_orientation != plane)
      return;

    m_slice = value;

    if(this->m_widgetState == vtkSkeletonWidget::Define)
    {
      double pos[3]{0,0,0};
      auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep);
      if(rep->GetActiveNodeWorldPosition(pos))
      {
        pos[normalCoordinateIndex(this->m_orientation)] = value;
        rep->SetActiveNodeToWorldPosition(pos, false);
      }
    }

    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep);
    rep->SetSlice(value);

    if(this->Interactor != nullptr)
    {
      int X = this->Interactor->GetEventPosition()[0];
      int Y = this->Interactor->GetEventPosition()[1];

      rep->ComputeInteractionState(X, Y);
      auto state = rep->GetInteractionState();
      this->SetCursor(state);

      if(this->m_widgetState == vtkSkeletonWidget::Start)
      {
        if(rep->IsNearNode(X,Y))
          rep->ActivateNode(X,Y);
        else
          rep->DeactivateNode();
      }
    }

    if (this->WidgetRep->GetNeedToRender())
    {
      this->Render();
      this->WidgetRep->NeedToRenderOff();
    }
  }

  //-----------------------------------------------------------------------------
  vtkSmartPointer<vtkPolyData> vtkSkeletonWidget::getSkeleton()
  {
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep);
    return rep->GetRepresentationPolyData();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::SetShift(const Nm shift)
  {
    if(m_shift == shift)
      return;

    m_shift = shift;

    if(this->WidgetRep == nullptr)
    {
      this->CreateDefaultRepresentation();
      return;
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep)->SetShift(shift);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::SetTolerance(const double tolerance)
  {
    if(m_drawTolerance == tolerance)
      return;

    m_drawTolerance = tolerance;

    if(this->WidgetRep == nullptr)
    {
      this->CreateDefaultRepresentation();
      return;
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep)->SetTolerance(m_drawTolerance);
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::setRepresentationColor(const QColor &color)
  {
    this->m_color = color;

    if(this->WidgetRep == nullptr)
    {
      this->CreateDefaultRepresentation();
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep)->SetColor(color);
    this->Render();
    this->WidgetRep->NeedToRenderOff();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::UpdateRepresentation()
  {
    if(!this->WidgetRep)
      return;

    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep);
    rep->BuildRepresentation();
    rep->NeedToRenderOff();
    this->Render();
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidget::SetSpacing(const NmVector3 &spacing)
  {
    this->m_spacing = spacing;

    if(this->WidgetRep == nullptr)
    {
      this->CreateDefaultRepresentation();
    }

    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(this->WidgetRep)->SetSpacing(spacing);
  }

} // namespace ESPINA
