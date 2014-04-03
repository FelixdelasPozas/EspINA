/*
 * ZoomSelectionWidget.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "ZoomSelectionWidget.h"
#include "vtkZoomSelectionWidget.h"
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
#include <QEvent>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomSelectionWidget::ZoomSelectionWidget()
: m_command{vtkSmartPointer<vtkZoomCommand>::New()}
{
  m_command->setWidget(this);

  QPixmap cursorBitmap;
  cursorBitmap.load(":/espina/zoom_cursor.png", "PNG", Qt::ColorOnly);
  m_cursor = QCursor(cursorBitmap, 0, 0);
}

//----------------------------------------------------------------------------
ZoomSelectionWidget::~ZoomSelectionWidget()
{
  for(ZoomSelectionWidgetAdapter *widget: m_views.values())
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
    ZoomSelectionWidgetAdapter *widget = ZoomSelectionWidgetAdapter::New();
    Q_ASSERT(widget);

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
    widget->On();

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
  if (e->type() != QEvent::MouseMove && e->type() != QEvent::MouseButtonPress && e->type() != QEvent::MouseButtonRelease)
    return false;

  if (m_inUse && m_views.contains(view))
    return m_views[view]->ProcessBasicQtEvent(e);

  return false;
}

//----------------------------------------------------------------------------
void ZoomSelectionWidget::setInUse(bool value)
{
  if(m_inUse == value)
    return;

  m_inUse = value;

  emit eventHandlerInUse(value);
}

//----------------------------------------------------------------------------
void vtkZoomCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
{
  // this is needed to update the thumbnail when zooming the view.
  ZoomSelectionWidgetAdapter *widget = dynamic_cast<ZoomSelectionWidgetAdapter *>(caller);
  if(!widget)
    return;

  if(!m_widget->m_views.values().contains(widget))
    return;

  m_widget->m_views.key(widget)->updateView();
}
