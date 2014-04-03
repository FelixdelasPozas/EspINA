/*
 * ZoomSelectionWidget.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "ZoomSelectionWidget.h"
#include "vtkZoomSelectionWidget.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <GUI/View/RenderView.h>
#include "vtkZoomSelectionWidget.h"
#include <Support/ViewManager.h>
#include <GUI/View/View2D.h>

// vtk
#include <vtkAbstractWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkWidgetRepresentation.h>
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

// Qt
#include <QMouseEvent>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomSelectionWidget::ZoomSelectionWidget()
: m_command{vtkSmartPointer<vtkZoomCommand>::New()}
{
  m_command->setWidget(this);
}

//----------------------------------------------------------------------------
ZoomSelectionWidget::~ZoomSelectionWidget()
{
  for(vtkZoomSelectionWidget *widget: m_views.values())
  {
    widget->SetEnabled(false);
    widget->RemoveObserver(m_command);
    widget->SetCurrentRenderer(nullptr);
    widget->SetInteractor(nullptr);
    widget->Delete();
  }
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::registerView(RenderView *view)
{
  if (m_views.keys().contains(view))
    return;

  View2D *view2d = dynamic_cast<View2D *>(view);
  if (view2d)
  {
    auto widget = vtkZoomSelectionWidget::New();
    widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);

    switch (view2d->plane())
    {
      case Plane::XY:
        widget->SetWidgetType(vtkZoomSelectionWidget::AXIAL_WIDGET);
        break;
      case Plane::XZ:
        widget->SetWidgetType(vtkZoomSelectionWidget::CORONAL_WIDGET);
        break;
      case Plane::YZ:
        widget->SetWidgetType(vtkZoomSelectionWidget::SAGITTAL_WIDGET);
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    widget->SetCurrentRenderer(view2d->renderWindow()->GetRenderers()->GetFirstRenderer());
    widget->SetInteractor(view2d->renderWindow()->GetInteractor());
    widget->SetEnabled(true);

    m_views.insert(view, widget);
  }
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::unregisterView(RenderView *view)
{
  if (!m_views.keys().contains(view))
    return;

  m_views[view]->SetEnabled(false);
  m_views[view]->SetCurrentRenderer(nullptr);
  m_views[view]->SetInteractor(nullptr);
  m_views[view]->Delete();

  m_views.remove(view);
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::setEnabled(bool enable)
{
  for(auto widget: m_views.values())
    widget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
bool ZoomSelectionWidget::filterEvent(QEvent *e, RenderView *view)
{
  return false;
}

//----------------------------------------------------------------------------
void vtkZoomCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  // this is needed to update the thumbnail when zooming the view.
  vtkZoomSelectionWidget *widget = dynamic_cast<vtkZoomSelectionWidget *>(caller);
  if(!widget)
    return;

  if(!m_widget->m_views.values().contains(widget))
    return;

  m_widget->m_views.key(widget)->updateView();
}
