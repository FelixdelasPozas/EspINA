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
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <GUI/View/Widgets/PlanarSplit/vtkPlanarSplitWidget.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitEventHandler.h>

// VTK
#include <vtkImplicitPlaneWidget2.h>

using namespace ESPINA::GUI::View::Widgets;

//-----------------------------------------------------------------------------
PlanarSplitWidget::PlanarSplitWidget(PlanarSplitEventHandler * handler)
: m_handler{handler}
{
  connect(this,    SIGNAL(created(PlanarSplitWidgetPtr)),
          handler, SIGNAL(widgetCreated(PlanarSplitWidgetPtr)));
  connect(this,    SIGNAL(destroyed(PlanarSplitWidgetPtr)),
          handler, SIGNAL(widgetDestroyed(PlanarSplitWidgetPtr)));
  connect(this,    SIGNAL(planeDefined(PlanarSplitWidgetPtr)),
          handler, SIGNAL(planeDefined(PlanarSplitWidgetPtr)));

  emit created(this);
}

//-----------------------------------------------------------------------------
PlanarSplitWidget::~PlanarSplitWidget()
{
  emit destroyed(this);
}

//-----------------------------------------------------------------------------
void PlanarSplitWidget::emitSetSignal()
{
  emit planeDefined(this);
}

//-----------------------------------------------------------------------------
void vtkSplitCommand::Execute(vtkObject *caller, unsigned long eventId, void *callData)
{
  auto widget2d = dynamic_cast<vtkPlanarSplitWidget *>(caller);
  if(widget2d != nullptr)
  {
    widget2d->RemoveObserver(this);
    m_widget->emitSetSignal();
  }
  else
  {
    auto widget3d = static_cast<vtkImplicitPlaneWidget2 *>(caller);
    if(widget3d != nullptr)
    {
      widget3d->RemoveObserver(this);
      m_widget->emitSetSignal();
    }
  }
}
