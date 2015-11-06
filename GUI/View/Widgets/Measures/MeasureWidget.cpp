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
#include "MeasureWidget.h"

#include <Core/Types.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>

//vtk
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

// Qt
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::Measures;


class MeasureWidget::vtkDistanceCommand
: public vtkCommand
{
  vtkTypeMacro(vtkDistanceCommand, vtkCommand);

  virtual ~vtkDistanceCommand();

  /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
   *
   */
  static vtkDistanceCommand* New()
  { return new vtkDistanceCommand(); }

  void setDistanceWidget(vtkDistanceWidget *widget);

  virtual void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

private:
  /** \brief Class vtkDistanceCommand class private constructor.
   *
   */
  explicit vtkDistanceCommand()
  : m_widget{nullptr}
  , m_camera{nullptr}
  {}

  vtkProperty2D *pointProperty(vtkHandleRepresentation *point) const;

  /** \brief Computes optimal tick distance for current representation
   * \param[in] length numerical value.
   *
   */
  Nm optimalTickDistance(vtkDistanceRepresentation2D *representation) const;

private:
  vtkDistanceWidget *m_widget;
  vtkCamera         *m_camera;
};

//----------------------------------------------------------------------------
MeasureWidget::vtkDistanceCommand::~vtkDistanceCommand()
{
}

//----------------------------------------------------------------------------
void MeasureWidget::vtkDistanceCommand::setDistanceWidget(vtkDistanceWidget *widget)
{
  m_widget = widget;

  if (!m_widget && m_camera)
  {
    m_camera->RemoveObserver(this);
    m_camera = nullptr;
  }
}

//----------------------------------------------------------------------------
void MeasureWidget::vtkDistanceCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  if (strcmp("vtkOpenGLCamera", caller->GetClassName()) == 0)
  {
    double p1[3], p2[3];
    // Force representation update when zoom is applied
    m_widget->GetDistanceRepresentation()->GetPoint1WorldPosition(p1);
    m_widget->GetDistanceRepresentation()->SetPoint1WorldPosition(p1);
    m_widget->GetDistanceRepresentation()->GetPoint2WorldPosition(p2);
    m_widget->GetDistanceRepresentation()->SetPoint2WorldPosition(p2);
    m_widget->GetDistanceRepresentation()->BuildRepresentation();
  }
  else if (strcmp("vtkDistanceWidget", caller->GetClassName()) == 0)
  {
    auto widget = reinterpret_cast<vtkDistanceWidget*>(caller);
    auto rep    = reinterpret_cast<vtkDistanceRepresentation2D*>(widget->GetRepresentation());

    if (vtkCommand::StartInteractionEvent == eventId)
    {
      if (!m_camera)
      {
        auto renderers = widget->GetInteractor()->GetRenderWindow()->GetRenderers();
        auto renderer  = renderers->GetFirstRenderer();

        m_camera = renderer->GetActiveCamera();
        m_camera->AddObserver(vtkCommand::AnyEvent, this);
      }

      rep->SetLabelFormat("%.1f nm");
      rep->RulerModeOn();
      rep->SetRulerDistance(optimalTickDistance(rep));

      auto property = pointProperty(rep->GetPoint1Representation());
      property->SetColor(0.0, 1.0, 0.0);
      property->SetLineWidth(2);

      property = pointProperty(rep->GetPoint2Representation());
      property->SetColor(0.0, 1.0, 0.0);
      property->SetLineWidth(2);
    }
    else if (vtkCommand::InteractionEvent == eventId)
    {
      Nm newTickDistance = optimalTickDistance(rep);

      if (rep->GetRulerDistance() != newTickDistance)
      {
        rep->SetRulerDistance(newTickDistance);
        rep->BuildRepresentation();
      }
    }
  }
}

//----------------------------------------------------------------------------
vtkProperty2D *MeasureWidget::vtkDistanceCommand::pointProperty(vtkHandleRepresentation *point) const
{
  return reinterpret_cast<vtkPointHandleRepresentation2D*>(point)->GetProperty();
}

//----------------------------------------------------------------------------
Nm MeasureWidget::vtkDistanceCommand::optimalTickDistance(vtkDistanceRepresentation2D *representation) const
{
  Nm result   = 1.0;
  Nm distance = representation->GetDistance();

  while (distance >= 10)
  {
    result   *= 10.0;
    distance /= 10.0;
  }

  return result;
}

//----------------------------------------------------------------------------
MeasureWidget::MeasureWidget(MeasureEventHandler *eventHandler)
: m_eventHandler(eventHandler)
, m_command{vtkSmartPointer<vtkDistanceCommand>::New()}
, m_widget{vtkSmartPointer<vtkDistanceWidget>::New()}
{
  m_widget->AddObserver(vtkCommand::StartInteractionEvent, m_command);
  m_widget->AddObserver(vtkCommand::InteractionEvent,      m_command);
  m_widget->CreateDefaultRepresentation();

  connect(eventHandler, SIGNAL(clear()),
          this,         SLOT(onClear()));
}

//----------------------------------------------------------------------------
MeasureWidget::~MeasureWidget()
{
}

//----------------------------------------------------------------------------
void MeasureWidget::setPlane(Plane plane)
{

}

//----------------------------------------------------------------------------
void MeasureWidget::setRepresentationDepth(Nm depth)
{

}

//----------------------------------------------------------------------------
TemporalRepresentation2DSPtr MeasureWidget::clone()
{
  return std::make_shared<MeasureWidget>(m_eventHandler);
}

//----------------------------------------------------------------------------
bool MeasureWidget::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return false;
}

//----------------------------------------------------------------------------
bool MeasureWidget::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return false;
}

//----------------------------------------------------------------------------
bool MeasureWidget::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//----------------------------------------------------------------------------
void MeasureWidget::initializeImplementation(RenderView *view)
{
  m_command->setDistanceWidget(m_widget);
}

//----------------------------------------------------------------------------
void MeasureWidget::uninitializeImplementation()
{
  m_command->setDistanceWidget(nullptr);
}

//----------------------------------------------------------------------------
vtkAbstractWidget *MeasureWidget::vtkWidget()
{
  return m_widget;
}

//----------------------------------------------------------------------------
void MeasureWidget::onClear()
{
  m_widget->SetWidgetStateToStart();
  m_widget->GetDistanceRepresentation()->VisibilityOff();
  m_widget->GetDistanceRepresentation()->GetPoint1Representation()->VisibilityOff();
  m_widget->GetDistanceRepresentation()->GetPoint2Representation()->VisibilityOff();
  m_widget->Render();
}
