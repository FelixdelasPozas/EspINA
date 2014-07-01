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

// EspINA
#include "RulerWidget.h"
#include "vtkRulerWidget.h"
#include "vtkRulerWidget3D.h"
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkCubeAxesActor2D.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  RulerWidget::RulerWidget()
  : m_command{vtkSmartPointer<vtkRulerCommand>::New()}
  {
    m_command->setWidget(this);
  }
  
  //----------------------------------------------------------------------------
  RulerWidget::~RulerWidget()
  {
    for(vtkAbstractWidget *widget: m_views.values())
    {
      widget->SetInteractor(nullptr);
      widget->SetCurrentRenderer(nullptr);
      widget->Delete();
    }

    for(auto view: m_views.keys())
    {
      View2D *view2d = dynamic_cast<View2D*>(view);
      if(view2d)
      {
        disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));
        view2d->mainRenderer()->GetActiveCamera()->RemoveObserver(m_command);
      }
    }
    m_views.clear();
  }

  //----------------------------------------------------------------------------
  void RulerWidget::registerView(RenderView *view)
  {
    if (m_views.keys().contains(view))
      return;

    if(isView3D(view))
    {
      auto widget = vtkRulerWidget3D::New();
      widget->SetCurrentRenderer(view->renderWindow()->GetRenderers()->GetFirstRenderer());
      widget->SetInteractor(view->renderWindow()->GetInteractor());
      widget->SetEnabled(true);

      m_views[view] = widget;
      view->mainRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, m_command);
    }
    else
    {
      View2D *view2d = view2D_cast(view);

      if(view2d)
      {
        auto widget = vtkRulerWidget::New();
        widget->SetCurrentRenderer(view->renderWindow()->GetRenderers()->GetFirstRenderer());
        widget->SetInteractor(view->renderWindow()->GetInteractor());
        widget->setPlane(view2d->plane());
        widget->SetEnabled(false);

        m_views[view] = widget;
        connect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)), Qt::QueuedConnection);
        view->mainRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, m_command);
      }
    }
  }

  //----------------------------------------------------------------------------
  void RulerWidget::unregisterView(RenderView* view)
  {
    if (!m_views.keys().contains(view))
      return;

    m_views[view]->SetEnabled(false);
    m_views[view]->SetCurrentRenderer(nullptr);
    m_views[view]->SetInteractor(nullptr);
    m_views[view]->Delete();

    View2D *view2d = dynamic_cast<View2D *>(view);
    if (view2d)
      disconnect(view2d, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(sliceChanged(Plane, Nm)));

    view->mainRenderer()->GetActiveCamera()->RemoveObserver(m_command);
    m_views.remove(view);
  }

  //----------------------------------------------------------------------------
  void RulerWidget::sliceChanged(Plane plane, Nm pos)
  {
    auto rView = qobject_cast<RenderView *>(sender());

    for(auto view: m_views.keys())
    {
      if(view == rView)
      {
        auto widget = dynamic_cast<vtkRulerWidget *>(m_views[view]);
        auto view2d = dynamic_cast<View2D *>(view);
        int index = normalCoordinateIndex(view2d->plane());
        Bounds bounds = widget->bounds();
        bool valid = bounds[2*index] <= pos && bounds[2*index+1] > pos;
        widget->SetEnabled(valid);
      }
    }
  }

  //----------------------------------------------------------------------------
  void RulerWidget::setEnabled(bool enable)
  {
    for(vtkAbstractWidget *widget : m_views.values())
      widget->SetEnabled(enable);
  }

  //----------------------------------------------------------------------------
  void RulerWidget::setBounds(Bounds bounds)
  {
    for(vtkAbstractWidget *widget : m_views.values())
    {
      vtkRulerWidget *widget2d = dynamic_cast<vtkRulerWidget *>(widget);
      if(widget2d)
      {
        if(bounds.areValid())
        {
          widget2d->setBounds(bounds);
          auto view = dynamic_cast<View2D *>(m_views.key(widget));
          auto index = normalCoordinateIndex(view->plane());
          auto pos = view->crosshairPoint()[index];
          bool valid = bounds[2*index] <= pos && bounds[2*index+1] > pos;
          widget2d->SetEnabled(valid);
        }
        else
          widget2d->SetEnabled(false);
      }
      else
      {
        vtkRulerWidget3D *widget3d = dynamic_cast<vtkRulerWidget3D *>(widget);
        if (widget3d)
        {
          if(bounds.areValid())
          {
            widget3d->SetEnabled(true);
            widget3d->setBounds(bounds);
          }
          else
          {
            widget3d->SetEnabled(false);
          }
        }
      }
    }
  }

  //----------------------------------------------------------------------------
  // vtkRulerCommand class methods
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  void vtkRulerCommand::Execute(vtkObject *caller, unsigned long int eventId, void* callData)
  {
    if(!m_widget || (strcmp("vtkOpenGLCamera", caller->GetClassName()) != 0))
      return;

    vtkCamera *camera = dynamic_cast<vtkCamera *>(caller);
    if(!camera)
      return;

    for(auto view: m_widget->m_views.keys())
    {
      auto view2d = dynamic_cast<View2D *>(view);

      if(view2d && view2d->mainRenderer()->GetActiveCamera() == camera)
      {
        auto widget = dynamic_cast<vtkRulerWidget *>(m_widget->m_views[view]);

        if(widget)
        {
          widget->drawActors();
          return;
        }
      }
    }
  }

} /* namespace EspINA */
