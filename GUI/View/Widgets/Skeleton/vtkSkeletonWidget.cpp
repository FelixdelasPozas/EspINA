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
using namespace ESPINA::Core;
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
    representation()->ClearRepresentation();
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
  if (!WidgetRep)
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
void vtkSkeletonWidget::SetOrientation(Plane plane)
{
  if (m_orientation == Plane::UNDEFINED)
  {
    m_orientation = plane;

    representation()->SetOrientation(plane);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::addPoint()
{
  if(m_widgetState != vtkSkeletonWidget::Define) return;

  auto rep = representation();

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
  auto rep = representation();

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
  auto rep = representation();
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
  auto rep = representation();

  switch (m_widgetState)
  {
    case vtkSkeletonWidget::Define:
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
      case vtkSkeletonWidget::Mark:
        switch (State)
        {
          case vtkSkeletonWidgetRepresentation::NearPoint:
          case vtkSkeletonWidgetRepresentation::NearContour:
            if (!ManagesCursor)
            {
              QApplication::setOverrideCursor(m_truncateCursor);
              ManagesCursorOn();
            }
            else
            {
              QApplication::changeOverrideCursor(m_truncateCursor);
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

  auto rep = representation();

  if (m_widgetState == vtkSkeletonWidget::Define)
  {
    double pos[3]{0,0,0};

    if (rep->GetActiveNodeWorldPosition(pos))
    {
      pos[normalCoordinateIndex(m_orientation)] = value;
      rep->SetActiveNodeToWorldPosition(pos, false);
    }
  }

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
  return representation()->GetRepresentationPolyData();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetShift(const Nm shift)
{
  if (m_shift != shift)
  {
    m_shift = shift;

    representation()->SetShift(shift);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::UpdateRepresentation()
{
  if (!WidgetRep)
  {
    CreateDefaultRepresentation();
    WidgetRep->SetRenderer(GetCurrentRenderer());
  }

  auto rep = representation();
  rep->BuildRepresentation();

  m_widgetState = vtkSkeletonWidget::Define;

  if (GetCurrentRenderer() != nullptr)
  {
    int X, Y;
    Interactor->GetEventPosition(X,Y);
    rep->ComputeInteractionState(X, Y);
    int wState = WidgetRep->GetInteractionState();

    SetCursor(wState);
  }

  rep->NeedToRenderOff();
  Render();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::SetSpacing(const NmVector3 &spacing)
{
  if(m_spacing != spacing)
  {
    m_spacing = spacing;

    representation()->SetSpacing(spacing);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setCurrentOperationMode(const int mode)
{
  if(m_widgetState != mode)
  {
    m_widgetState = mode;

    representation()->DeactivateNode();

    updateCursor();
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
    return reinterpret_cast<const vtkSkeletonWidgetRepresentation *>(WidgetRep)->GetNumberOfNodes();
  }

  return 0;
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidget::selectNode()
{
  if(WidgetRep && Interactor)
  {
    int X, Y;
    Interactor->GetEventPosition(X, Y);

    auto rep = representation();

    if(rep->ActivateNode(X, Y))
    {
      rep->UpdatePointer();
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::updateCursor()
{
  if(WidgetRep && Interactor)
  {
    int X, Y;
    Interactor->GetEventPosition(X, Y);

    auto rep = representation();
    rep->ComputeInteractionState(X, Y);

    SetCursor(rep->GetInteractionState());
  }
  else
  {
    SetCursor(vtkSkeletonWidgetRepresentation::Outside);
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::BuildRepresentation()
{
  if(!WidgetRep)
  {
    CreateDefaultRepresentation();
  }
  else
  {
    reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->BuildRepresentation();
  }
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setStroke(const Core::SkeletonStroke& stroke)
{
  representation()->setStroke(stroke);
}

//-----------------------------------------------------------------------------
const Core::SkeletonStroke vtkSkeletonWidget::stroke() const
{
  auto stroke = Core::SkeletonStroke();

  if(WidgetRep && numberOfPoints() != 0)
  {
    auto rep = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
    stroke = rep->currentStroke();
  }

  return stroke;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::removeStroke(const Core::SkeletonStroke& stroke)
{
  representation()->removeStroke(stroke);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::createCursors()
{
  QPixmap crossMinusPixmap, crossPlusPixmap, crossCheckPixmap, crossScissorsPixmap;
  crossMinusPixmap.load(":espina/cross-minus.png", "PNG", Qt::ColorOnly);
  crossPlusPixmap.load(":espina/cross-plus.png", "PNG", Qt::ColorOnly);
  crossCheckPixmap.load(":espina/cross-check.png", "PNG", Qt::ColorOnly);
  crossScissorsPixmap.load(":espina/cross-scissors.png", "PNG", Qt::ColorOnly);

  m_crossMinusCursor = QCursor(crossMinusPixmap, -1, -1);
  m_crossPlusCursor  = QCursor(crossPlusPixmap, -1, -1);
  m_crossCheckCursor = QCursor(crossCheckPixmap, -1, -1);
  m_truncateCursor   = QCursor(crossScissorsPixmap, -1, -1);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::ClearRepresentation()
{
  vtkSkeletonWidgetRepresentation::ClearRepresentation();
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::createConnection(const Core::SkeletonStroke& stroke)
{
  representation()->createConnection(stroke);
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidget::isStartNode(const NmVector3 &point) const
{
  if(WidgetRep)
  {
    return reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->isStartNode(point);
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::changeStroke(const Core::SkeletonStroke& stroke)
{
  auto rep = representation();

  int X,Y;
  Interactor->GetEventPosition(X,Y);
  rep->setIgnoreCursorNode(true);
  rep->ComputeInteractionState(X,Y);
  auto State = rep->GetInteractionState();
  rep->setIgnoreCursorNode(false);

  bool result = false;
  switch(State)
  {
    case vtkSkeletonWidgetRepresentation::NearContour:
      result = rep->AddNodeOnContour(X, Y);
      break;
    case vtkSkeletonWidgetRepresentation::NearPoint:
      result = rep->TryToJoin(X, Y);
      break;
    default:
      break;
  }

  if(!result)
  {
    rep->switchToStroke(stroke);
  }

  if (WidgetRep->GetNeedToRender())
  {
    Render();
    WidgetRep->NeedToRenderOff();
  }
}

//-----------------------------------------------------------------------------
bool vtkSkeletonWidget::markAsTruncated()
{
  int X,Y;
  Interactor->GetEventPosition(X,Y);

  return representation()->ToggleStrokeProperty(Core::SkeletonNodeProperty::TRUNCATED, X,Y);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setRepresentationWidth(const int width)
{
  representation()->setWidth(width);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setRepresentationShowText(bool value)
{
  representation()->setShowLabels(value);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setRepresentationTextSize(int size)
{
  representation()->setLabelsSize(size);
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setRepresentationTextColor(const QColor &color)
{
  representation()->setLabelsColor(color);
}

//-----------------------------------------------------------------------------
const Core::PathList vtkSkeletonWidget::selectedPaths() const
{
  PathList result;

  if(WidgetRep)
  {
    int X,Y;
    Interactor->GetEventPosition(X,Y);

    result = reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->currentSelectedPaths(X,Y);
  }

  return result;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::setStrokeHueModification(const bool value)
{
  representation()->setChangeCoincidetHue(value);
}

//-----------------------------------------------------------------------------
const bool vtkSkeletonWidget::strokeHueModification() const
{
  if(WidgetRep)
  {
    return reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep)->changeCoincidentHue();
  }

  return false;
}

//-----------------------------------------------------------------------------
void vtkSkeletonWidget::renameStroke(const QString& oldName, const QString& newName)
{
  representation()->renameStroke(oldName, newName);
}

//-----------------------------------------------------------------------------
vtkSkeletonWidgetRepresentation* vtkSkeletonWidget::representation()
{
  if(!WidgetRep) CreateDefaultRepresentation();

  return reinterpret_cast<vtkSkeletonWidgetRepresentation *>(WidgetRep);
}
