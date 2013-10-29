/*
 * vtkPlaneContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "vtkPlaneContourWidget.h"
#include "vtkPlaneContourRepresentation.h"
#include "vtkPlaneContourRepresentationGlyph.h"
#include <Core/EspinaTypes.h>
#include "ContourWidget.h"

// VTK
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkEvent.h>
#include <vtkWidgetEvent.h>
#include <vtkPolyData.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>
#include <vtkLinearContourLineInterpolator.h>
#include <vtkMath.h>

// Qt
#include <QApplication>
#include <QPixmap>
#include <QObject>

using namespace EspINA;

vtkStandardNewMacro(vtkPlaneContourWidget);

//----------------------------------------------------------------------------
vtkPlaneContourWidget::vtkPlaneContourWidget()
: WidgetState(vtkPlaneContourWidget::Start)
, CurrentHandle(0)
, AllowNodePicking(true)
, FollowCursor(true)
, ContinuousDraw(true)
, ContinuousActive(0)
, Orientation(CORONAL)
, ContinuousDrawTolerance(40)
, mouseButtonDown(false)
, m_polygonColor(Qt::black)
, m_parent(NULL)
, m_contourMode(Brush::BRUSH)
, m_actualBrushMode(Brush::BRUSH)
{
  this->ManagesCursor = 0; // from Superclass
  this->CreateDefaultRepresentation();

  // These are the event callbacks supported by this widget
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkWidgetEvent::Select, this, vtkPlaneContourWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent, vtkWidgetEvent::EndSelect, this, vtkPlaneContourWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent, vtkWidgetEvent::AddFinalPoint, this, vtkPlaneContourWidget::AddFinalPointAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, vtkPlaneContourWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent, vtkWidgetEvent::ModifyEvent, this, vtkPlaneContourWidget::KeyPressAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyReleaseEvent, vtkWidgetEvent::ModifyEvent, this, vtkPlaneContourWidget::KeyPressAction);

  QPixmap crossMinusPixmap, crossPlusPixmap, crossCheckPixmap;
  crossMinusPixmap.load(":espina/cross-minus.png", "PNG", Qt::ColorOnly);
  crossPlusPixmap.load(":espina/cross-plus.png", "PNG", Qt::ColorOnly);
  crossCheckPixmap.load(":espina/cross-check.png", "PNG", Qt::ColorOnly);

  this->crossMinusCursor = QCursor(crossMinusPixmap, -1, -1);
  this->crossPlusCursor = QCursor(crossPlusPixmap, -1, -1);
  this->crossCheckCursor = QCursor(crossCheckPixmap, -1, -1);
}

//----------------------------------------------------------------------------
vtkPlaneContourWidget::~vtkPlaneContourWidget()
{
  // restore the pointer if the widget has changed it
  if (this->ManagesCursor == true)
    QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    vtkPlaneContourRepresentationGlyph *rep = vtkPlaneContourRepresentationGlyph::New();
    this->WidgetRep = rep;

    vtkSmartPointer<vtkLinearContourLineInterpolator> interpolator = vtkSmartPointer<vtkLinearContourLineInterpolator>::New();
    rep->SetLineInterpolator(interpolator);

    vtkSphereSource *ss = vtkSphereSource::New();
    ss->SetRadius(0.5);
    rep->SetActiveCursorShape(ss->GetOutput());
    ss->Delete();

    vtkProperty *property = vtkProperty::SafeDownCast(rep->GetActiveProperty());
    if (property)
    {
      property->SetRepresentationToSurface();
      property->SetAmbient(0.1);
      property->SetDiffuse(0.9);
      property->SetSpecular(0.0);
    }
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::CloseLoop()
{
  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep);
  if (!rep->GetClosedLoop() && rep->GetNumberOfNodes() > 1)
  {
    this->WidgetState = vtkPlaneContourWidget::Manipulate;
    rep->ClosedLoopOn();
    rep->UseContourPolygon(true);
    this->Render();
    m_parent->endContourFromWidget();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::SetEnabled(int enabling)
{
  // The handle widgets are not actually enabled until they are placed.
  // The handle widgets take their representation from the vtkPlaneContourRepresentation.
  if (enabling)
  {
    if (this->WidgetState == vtkPlaneContourWidget::Start)
    {
      reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep)->VisibilityOff();
      reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep)->UseContourPolygon(false);
    }
    else
    {
      reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep)->VisibilityOn();
      if (this->WidgetState == vtkPlaneContourWidget::Manipulate)
        reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep)->UseContourPolygon(true);
    }
  }

  this->Superclass::SetEnabled(enabling);
}

//----------------------------------------------------------------------------
// The following methods are the callbacks that the contour widget responds to.
void vtkPlaneContourWidget::SelectAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);
  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  Qt::KeyboardModifiers pressedKeys = QApplication::keyboardModifiers();

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->WidgetRep->ComputeInteractionState(X, Y);
  int state = self->WidgetRep->GetInteractionState();
  self->SetCursor(state);

  double pos[2];
  pos[0] = X;
  pos[1] = Y;

  if (self->ContinuousDraw)
  {
    self->mouseButtonDown = true;
    self->ContinuousActive = 0;
  }

  switch (self->WidgetState)
  {
    case vtkPlaneContourWidget::Start:
    case vtkPlaneContourWidget::Define:
    {
      // If we are following the cursor, let's add 2 nodes rightaway, on the
      // first click. The second node is the one that follows the cursor
      // around.
      if ((self->FollowCursor || self->ContinuousDraw) && (rep->GetNumberOfNodes() == 0))
      {
        self->m_parent->startContourFromWidget();
        vtkPlaneContourRepresentationGlyph *repGlyph = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(self->WidgetRep);
        switch(self->m_actualBrushMode)
        {
          case Brush::BRUSH:
            repGlyph->SetLineColor(0,0,1);
            break;
          case Brush::ERASER:
            repGlyph->SetLineColor(1,0,0);
            break;
          default:
            Q_ASSERT(false);
            break;
        }

        self->AddNode();
      }

      self->AddNode();

      if (self->WidgetState == vtkPlaneContourWidget::Manipulate)
        break;

      // check if the cursor crosses the actual contour, if so then close the contour
      // and end defining
      if (rep->CheckAndCutContourIntersection())
      {
        if (self->ContinuousDraw)
          self->ContinuousActive = 0;

        // set the closed loop now
        rep->ClosedLoopOn();
        rep->UseContourPolygon(true);
        self->WidgetState = vtkPlaneContourWidget::Manipulate;
        self->EventCallbackCommand->SetAbortFlag(1);
        self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
        self->m_contourMode = self->m_actualBrushMode;
        self->m_parent->endContourFromWidget();
      }
      else
      {
        if (self->ContinuousDraw)
          self->ContinuousActive = 1;
      }

      break;
    }

    case vtkPlaneContourWidget::Manipulate:
    {
      // NOTE: the 'reset' action is in vtkPlaneContourWidget::KeyPressAction() as it happens when
      //       the user presses the backspace or delete key
      if (pressedKeys & Qt::ShiftModifier)
      {
        self->DeleteAction(w);
        state = self->WidgetRep->GetInteractionState();
        self->SetCursor(state);
        break;
      }

      if (self->WidgetRep->GetInteractionState() == vtkPlaneContourRepresentation::Inside)
      {
        self->TranslateContourAction(w);
        break;
      }

      if (rep->ActivateNode(X, Y))
      {
        self->Superclass::StartInteraction();
        self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
        self->StartInteraction();
        rep->SetCurrentOperationToTranslate();
        rep->StartWidgetInteraction(pos);
        self->EventCallbackCommand->SetAbortFlag(1);
        break;
      }
      else
        if (rep->AddNodeOnContour(X, Y))
        {
          if (rep->ActivateNode(X, Y))
          {
            rep->SetCurrentOperationToTranslate();
            rep->StartWidgetInteraction(pos);
          }
          self->EventCallbackCommand->SetAbortFlag(1);
          break;
        }

      if (!rep->GetNeedToRender())
        rep->SetRebuildLocator(true);

      // start a new contour
      self->m_parent->startContourFromWidget();
      self->SelectAction(w);
      break;
    }
  }

  if (rep->GetNeedToRender())
  {
    self->Render();
    rep->NeedToRenderOff();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::AddFinalPointAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);
  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  if (self->WidgetState == vtkPlaneContourWidget::Manipulate)
  {
    // pass the event forward
    vtkInteractorStyle *style = reinterpret_cast<vtkInteractorStyle*>(self->GetInteractor()->GetInteractorStyle());
    style->OnRightButtonDown();
    return;
  }

  int numnodes = rep->GetNumberOfNodes();

  // the last node is the cursor so we need to check if there are really that amount of unique points in the representation
  if (rep->CheckNodesForDuplicates(numnodes - 1, numnodes - 2))
    numnodes--;

  if (numnodes < 3)
    return;

  // assumed at this point
  Q_ASSERT(self->WidgetState != vtkPlaneContourWidget::Manipulate);

  // In follow cursor and continuous draw mode, the "extra" node
  // has already been added for us.
  if (!self->FollowCursor && !self->ContinuousDraw)
    self->AddNode();

  // need to modify the contour if intersects with itself
  rep->CheckAndCutContourIntersectionInFinalPoint();

  if (self->ContinuousDraw)
    self->ContinuousActive = 0;

  // set the closed loop now
  rep->ClosedLoopOn();
  rep->UseContourPolygon(true);

  self->WidgetState = vtkPlaneContourWidget::Manipulate;
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->WidgetRep->ComputeInteractionState(X, Y);
  int state = self->WidgetRep->GetInteractionState();
  self->SetCursor(state);

  if (rep->GetNeedToRender())
  {
    self->Render();
    rep->NeedToRenderOff();
  }

  self->m_contourMode = self->m_actualBrushMode;
  self->m_parent->endContourFromWidget();
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::AddNode()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // If the rep already has at least 2 nodes, check how close we are to the first
  vtkPlaneContourRepresentation* rep = reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep);

  int numNodes = rep->GetNumberOfNodes();
  if (numNodes > 1)
  {
    int closestNode = this->FindClosestNode();

    // if in continuous draw mode, we don't want to close the loop until we are at least
    // numNodes > pixelTolerance away
    if ((closestNode != -1) && !this->ContinuousActive && (numNodes > 2))
    {
      double nodePos[3];
      if (this->FollowCursor)
        rep->DeleteLastNode();
      rep->GetNthNodeWorldPosition(closestNode, nodePos);
      rep->AddNodeAtWorldPosition(nodePos);
      rep->CheckAndCutContourIntersectionInFinalPoint();

      // set the closed loop now
      rep->ClosedLoopOn();
      rep->UseContourPolygon(true);

      this->WidgetState = vtkPlaneContourWidget::Manipulate;
      this->Render();
      this->EventCallbackCommand->SetAbortFlag(1);
      this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
      this->m_contourMode = this->m_actualBrushMode;
      this->m_parent->endContourFromWidget();

      // change cursor
      int X = this->Interactor->GetEventPosition()[0];
      int Y = this->Interactor->GetEventPosition()[1];

      this->WidgetRep->ComputeInteractionState(X, Y);
      int state = this->WidgetRep->GetInteractionState();
      this->SetCursor(state);

      return;
    }
  }

  // check we are in continuous mode and try not to add too many points
  if (rep->AddNodeAtDisplayPosition(X, Y))
  {
    if (this->WidgetState == vtkPlaneContourWidget::Start)
      this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);

    this->WidgetState = vtkPlaneContourWidget::Define;
    rep->VisibilityOn();
    this->EventCallbackCommand->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::TranslateContourAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);

  if (self->WidgetState != vtkPlaneContourWidget::Manipulate)
    return;

  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double pos[2];
  pos[0] = X;
  pos[1] = Y;

  self->Superclass::StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  self->StartInteraction();
  rep->SetCurrentOperationToShift(); // Here
  rep->StartWidgetInteraction(pos);
  self->EventCallbackCommand->SetAbortFlag(1);

  if (rep->GetNeedToRender())
  {
    self->Render();
    rep->NeedToRenderOff();
  }
}

//----------------------------------------------------------------------------
// not used
void vtkPlaneContourWidget::ScaleContourAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);

  if (self->WidgetState != vtkPlaneContourWidget::Manipulate)
    return;

  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double pos[2];
  pos[0] = X;
  pos[1] = Y;

  if (rep->ActivateNode(X, Y))
  {
    self->Superclass::StartInteraction();
    self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
    self->StartInteraction();
    rep->SetCurrentOperationToScale(); // Here
    rep->StartWidgetInteraction(pos);
    self->EventCallbackCommand->SetAbortFlag(1);
  }
  else
  {
    double p[3];
    int idx;
    if (rep->FindClosestPointOnContour(X, Y, p, &idx))
    {
      rep->GetNthNodeDisplayPosition(idx, pos);
      rep->ActivateNode(pos);
      self->Superclass::StartInteraction();
      self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
      self->StartInteraction();
      rep->SetCurrentOperationToScale(); // Here
      rep->StartWidgetInteraction(pos);
      self->EventCallbackCommand->SetAbortFlag(1);
    }
  }

  if (rep->GetNeedToRender())
  {
    self->Render();
    rep->NeedToRenderOff();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::DeleteAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);

  if (self->WidgetState == vtkPlaneContourWidget::Start)
    return;

  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  if (self->WidgetState == vtkPlaneContourWidget::Define)
  {
    if (rep->DeleteLastNode())
      self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  }
  else
  {
    // do not allow less than three nodes, i don't want to use the old solution of opening the contour
    // if we have less than three nodes and put the widget into ::Define state if we have just one node
    if (rep->GetNumberOfNodes() <= 3)
      return;

    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];
    rep->ActivateNode(X, Y);
    if (rep->DeleteActiveNode())
      self->InvokeEvent(vtkCommand::InteractionEvent, NULL);

    rep->ActivateNode(X, Y);
  }

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->WidgetRep->ComputeInteractionState(X, Y);
  int state = self->WidgetRep->GetInteractionState();
  self->SetCursor(state);

  if (rep->GetNeedToRender())
  {
    self->Render();
    rep->NeedToRenderOff();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  if (self->WidgetState == vtkPlaneContourWidget::Start)
    return;

  self->WidgetRep->ComputeInteractionState(X, Y);
  int state = self->WidgetRep->GetInteractionState();
  self->SetCursor(state);

  if (self->WidgetState == vtkPlaneContourWidget::Define)
  {
    if (self->FollowCursor || self->ContinuousDraw)
    {
      // Have the last node follow the mouse in this case...
      const int numNodes = rep->GetNumberOfNodes();

      if (numNodes > 1)
      {
        if (rep->GetClosedLoop() == 0)
        {
          if (self->ContinuousDraw && self->ContinuousActive)
          {
            if (self->IsPointTooClose(X, Y) && (rep->GetNumberOfNodes() > 2))
              return;

            rep->AddNodeAtDisplayPosition(X, Y);
            self->InvokeEvent(vtkCommand::InteractionEvent, NULL);

            // check the contour detect if it intersects with itself
            if (rep->CheckAndCutContourIntersection())
            {
              // last check
              int numNodes = rep->GetNumberOfNodes();
              if (rep->CheckNodesForDuplicates(numNodes - 1, numNodes - 2))
                rep->DeleteNthNode(numNodes - 2);

              if (self->ContinuousDraw)
                self->ContinuousActive = 0;

              // set the closed loop now
              rep->ClosedLoopOn();
              rep->UseContourPolygon(true);

              self->WidgetState = vtkPlaneContourWidget::Manipulate;
              self->EventCallbackCommand->SetAbortFlag(1);
              self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
              self->m_contourMode = self->m_actualBrushMode;
              self->m_parent->endContourFromWidget();
              return;
            }
          }
          else
          {
            // If we aren't changing the loop topology, simply update the position of the latest node to follow the mouse cursor position (X,Y).
            rep->SetNthNodeDisplayPosition(numNodes - 1, X, Y);
            self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
          }
        }
      }
    }
    else
      return;
  }

  if (rep->GetCurrentOperation() == vtkPlaneContourRepresentation::Inactive)
  {
    rep->ComputeInteractionState(X, Y);
    rep->ActivateNode(X, Y);
  }
  else
  {
    double pos[2];
    pos[0] = X;
    pos[1] = Y;
    self->WidgetRep->WidgetInteraction(pos);
    self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  }

  if (self->WidgetRep->GetNeedToRender())
  {
    self->Render();
    self->WidgetRep->NeedToRenderOff();
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);
  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(self->WidgetRep);

  if (self->ContinuousDraw)
  {
    self->mouseButtonDown = false;
    self->ContinuousActive = 0;
  }

  // Do nothing if inactive
  if (rep->GetCurrentOperation() == vtkPlaneContourRepresentation::Inactive)
  {
    rep->SetRebuildLocator(true);
    return;
  }

  rep->SetCurrentOperationToInactive();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Superclass::EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);

  // Node picking
  if (self->AllowNodePicking && self->Interactor->GetControlKey() && self->WidgetState == vtkPlaneContourWidget::Manipulate)
    rep->ToggleActiveNodeSelected();

  if (self->WidgetRep->GetNeedToRender())
  {
    self->Render();
    self->WidgetRep->NeedToRenderOff();
  }

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->WidgetRep->ComputeInteractionState(X, Y);
  int state = self->WidgetRep->GetInteractionState();
  self->SetCursor(state);
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::ResetAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);
  self->Initialize(NULL);
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::Initialize(vtkPolyData * pd, int state)
{
  if (!this->WidgetRep)
    this->CreateDefaultRepresentation();

  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep);
  rep->UseContourPolygon(false);

  Brush::BrushMode brushMode;
  if (pd == NULL)
  {
    while (rep->DeleteLastNode())
    {
      ;
    }
    rep->ClosedLoopOff();
    this->Render();
    rep->NeedToRenderOff();
    rep->VisibilityOff();
    this->WidgetState = vtkPlaneContourWidget::Start;
    brushMode = m_actualBrushMode;
  }
  else
  {
    rep->Initialize(pd);
    this->WidgetState = vtkPlaneContourWidget::Manipulate;
    rep->UseContourPolygon(true);
    brushMode = m_contourMode;
  }

  vtkPlaneContourRepresentationGlyph *repGlyph = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->WidgetRep);
  switch(brushMode)
  {
    case Brush::BRUSH:
      repGlyph->SetLineColor(0,0,1);
      break;
    case Brush::ERASER:
      repGlyph->SetLineColor(1,0,0);
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];
  this->WidgetRep->ComputeInteractionState(X,Y);
  int wState = this->WidgetRep->GetInteractionState();
  if (pd == NULL)
    this->SetCursor(vtkPlaneContourRepresentation::Outside);
  else
    this->SetCursor(wState);
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::SetAllowNodePicking(int val)
{
  if (this->AllowNodePicking == val)
    return;

  this->AllowNodePicking = val;
  if (this->AllowNodePicking)
  {
    vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep);
    rep->SetShowSelectedNodes(this->AllowNodePicking);
  }
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "WidgetState: " << this->WidgetState << endl;
  os << indent << "CurrentHandle: " << this->CurrentHandle << endl;
  os << indent << "AllowNodePicking: " << this->AllowNodePicking << endl;
  os << indent << "FollowCursor: " << (this->FollowCursor ? "On" : "Off") << endl;
  os << indent << "ContinuousDraw: " << (this->ContinuousDraw ? "On" : "Off") << endl;
}

//----------------------------------------------------------------------------
int vtkPlaneContourWidget::FindClosestNode()
{
  double lastNodePos[3], nodePos[3];
  int closestNode = -1;
  double closestDistance = VTK_DOUBLE_MAX;

  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep);
  if (rep->GetNumberOfNodes() >= 4)
  {
    int pixelTolerance = rep->GetPixelTolerance();
    rep->GetNthNodeWorldPosition(rep->GetNumberOfNodes()-1, lastNodePos);

    for (int i = 0; i < rep->GetNumberOfNodes()-1; ++i)
    {
      rep->GetNthNodeWorldPosition(i, nodePos);
      double distance = vtkMath::Distance2BetweenPoints(nodePos, lastNodePos);
      if ((distance < pixelTolerance) && (distance < closestDistance))
      {
        closestDistance = distance;
        closestNode = i;
      }
    }
  }

  return closestNode;
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::SetCursor(int cState)
{
  // cursor will only change in manipulate or start mode only
  if (this->WidgetState == vtkPlaneContourWidget::Define)
  {
    if (-1 != FindClosestNode() && !this->mouseButtonDown)
    {
      if (!this->ManagesCursor)
      {
        this->ManagesCursor = true;
        QApplication::setOverrideCursor(crossCheckCursor);
      }
      return;
    }

    if (this->ManagesCursor)
    {
      this->ManagesCursor = false;
      QApplication::restoreOverrideCursor();
    }
    return;
  }

  // using vtk keypress/keyrelease events is useless when the interactor loses it's focus
  Qt::KeyboardModifiers pressedKeys = QApplication::keyboardModifiers();

  if (!this->ManagesCursor && cState != vtkPlaneContourRepresentation::Outside)
  {
    this->ManagesCursor = true;
    QApplication::setOverrideCursor(Qt::CrossCursor);
  }

  switch (cState)
  {
    case vtkPlaneContourRepresentation::Nearby:
    case vtkPlaneContourRepresentation::NearPoint:
      if (pressedKeys & Qt::ShiftModifier)
        QApplication::changeOverrideCursor(crossMinusCursor);
      else
        QApplication::changeOverrideCursor(Qt::PointingHandCursor);
      break;
    case vtkPlaneContourRepresentation::NearContour:
      if (pressedKeys & Qt::ShiftModifier)
        QApplication::changeOverrideCursor(Qt::CrossCursor);
      else
        QApplication::changeOverrideCursor(crossPlusCursor);
      break;
    case vtkPlaneContourRepresentation::Inside:
      this->RequestCursorShape(VTK_CURSOR_SIZEALL);
      break;
    default:
      if (this->ManagesCursor)
      {
        this->ManagesCursor = false;
        QApplication::restoreOverrideCursor();
      }
      break;
  }
}

//----------------------------------------------------------------------------
// NOTE: keypress/keyrelease vtk events are mostly useless. when the interactor loses focus and then user
// 		 presses the keys we are watching outside the interactor it really fucks the widget state as it
//		 still has the keyboard focus. the solution is a mixed vtk/qt key events watching and using a
//		 Qt event filter to the classes that use the contour widget to take away or put the keyboard
//     focus in the interactor associated with the widgets when the mouse leaves or enters those widgets.
void vtkPlaneContourWidget::KeyPressAction(vtkAbstractWidget *w)
{
  vtkPlaneContourWidget *self = reinterpret_cast<vtkPlaneContourWidget*>(w);

  std::string key = std::string(self->Interactor->GetKeySym());

  if (("Delete" == key) || ("BackSpace" == key))
  {
    self->EnabledOff();
    self->ResetAction(w);
    self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    self->SetCursor(vtkPlaneContourRepresentation::Outside);
    self->EnabledOn();
    return;
  }

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->WidgetRep->ComputeInteractionState(X, Y);
  int state = self->WidgetRep->GetInteractionState();
  self->SetCursor(state);
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::SetOrientation(PlaneType plane)
{
  Orientation = plane;
  vtkPlaneContourRepresentation *rep = reinterpret_cast<vtkPlaneContourRepresentation*>(this->WidgetRep);
  rep->SetOrientation(Orientation);
}

//----------------------------------------------------------------------------
PlaneType vtkPlaneContourWidget::GetOrientation()
{
  return Orientation;
}

//----------------------------------------------------------------------------
bool vtkPlaneContourWidget::IsPointTooClose(int X, int Y)
{
  vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->WidgetRep);
  return (rep->Distance2BetweenPoints(X,Y, rep->GetNumberOfNodes()-2) < this->ContinuousDrawTolerance);
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::setPolygonColor(QColor color)
{
  this->m_polygonColor = color;
  vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph *>(this->WidgetRep);
  rep->setPolygonColor(color);
}

//----------------------------------------------------------------------------
QColor vtkPlaneContourWidget::getPolygonColor()
{
  return this->m_polygonColor;
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::setActualContourMode(Brush::BrushMode mode)
{
  m_contourMode = mode;
}

//----------------------------------------------------------------------------
void vtkPlaneContourWidget::setContourMode(Brush::BrushMode mode)
{
  m_actualBrushMode = mode;

  if (this->WidgetState != vtkPlaneContourWidget::Manipulate)
  {
    vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->WidgetRep);
    switch(m_actualBrushMode)
    {
      case Brush::BRUSH:
        rep->SetLineColor(0,0,1);
        break;
      case Brush::ERASER:
        rep->SetLineColor(1,0,0);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
  }
}

//----------------------------------------------------------------------------
Brush::BrushMode vtkPlaneContourWidget::getContourMode()
{
  return m_contourMode;
}