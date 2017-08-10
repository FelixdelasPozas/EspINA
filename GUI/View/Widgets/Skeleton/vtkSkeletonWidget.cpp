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

// Qt
#include <QApplication>
#include <QPixmap>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

vtkStandardNewMacro(vtkSkeletonWidget);

//-----------------------------------------------------------------------------
vtkSkeletonWidget::vtkSkeletonWidget()
: m_widgetState  {vtkSkeletonWidget::Define}
, m_orientation  {Plane::UNDEFINED}
, m_slice        {-1}
, m_shift        {1}
, m_ignoreCursor {false}
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
    rep->VisibilityOff();
  }
  else
  {
    rep->Initialize(pd);
  }

  m_widgetState = vtkSkeletonWidget::Define;

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

  if(rep->GetNeedToRender())
  {
    Render();
    rep->NeedToRenderOff();
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

  SetCursor(State);

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
  double worldPos[3];

  switch (m_widgetState)
  {
    case vtkSkeletonWidget::Delete:
      if (rep->IsNearNode(X, Y))
      {
        rep->ActivateNode(X, Y);
      }
      break;
    case vtkSkeletonWidget::Manipulate:
      if(rep->GetActiveNodeWorldPosition(worldPos))
      {
        rep->SetActiveNodeToDisplayPosition(X, Y);
      }
      break;
    case vtkSkeletonWidget::Define:
      rep->SetActiveNodeToDisplayPosition(X, Y);
      break;
    default:
      break;
  }

  if(m_widgetState == vtkSkeletonWidget::Define && m_ignoreCursor) rep->setIgnoreCursorNode(true);
  rep->ComputeInteractionState(X, Y);
  if(m_widgetState == vtkSkeletonWidget::Define && m_ignoreCursor) rep->setIgnoreCursorNode(false);
  SetCursor(WidgetRep->GetInteractionState());

  if (WidgetRep->GetNeedToRender())
  {
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidget::deletePoint()
{
  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
  int X,Y;
  Interactor->GetEventPosition(X,Y);

  auto result = rep->DeleteCurrentNode();

  if (rep->GetNeedToRender())
  {
    Render();
    rep->NeedToRenderOff();
  }

  return result;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::stop()
{
  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);

  switch (m_widgetState)
  {
    case vtkSkeletonWidget::Define:
      rep->DeleteCurrentNode();
      break;
    case vtkSkeletonWidget::Manipulate:
    case vtkSkeletonWidget::Delete:
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
      break;
    case vtkSkeletonWidget::Manipulate:
    {
      switch(State)
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
        default:
          if(ManagesCursor)
          {
            QApplication::restoreOverrideCursor();
            ManagesCursorOff();
          }
          break;
      }
    }
      break;
    case vtkSkeletonWidget::Delete:
      switch (State)
      {
        case vtkSkeletonWidgetRepresentation::NearPoint:
          if (!ManagesCursor)
          {
            QApplication::setOverrideCursor(m_crossMinusCursor);
            ManagesCursorOn();
          }
          else
          {
            QApplication::changeOverrideCursor(m_crossMinusCursor);
          }
          break;
        case vtkSkeletonWidgetRepresentation::NearContour:
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

    if (m_widgetState == vtkSkeletonWidget::Manipulate)
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
    m_widgetState = mode;

    if(WidgetRep)
    {
      auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
      rep->DeactivateNode();
    }
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

    auto nodeSelected = rep->ActivateNode(X, Y);
    rep->UpdatePointer();

    return nodeSelected;
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::updateCursor()
{
  int X, Y;
  Interactor->GetEventPosition(X, Y);

  auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
  rep->ComputeInteractionState(X, Y);

  SetCursor(rep->GetInteractionState());
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::BuildRepresentation()
{
  reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->BuildRepresentation();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setStroke(const Core::SkeletonStroke& stroke)
{
  if(WidgetRep)
  {
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
    rep->setStroke(stroke);
  }
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
