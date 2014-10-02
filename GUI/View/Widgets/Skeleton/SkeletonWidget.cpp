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
#include <GUI/View/Widgets/Skeleton/SkeletonWidget.h>
#include <GUI/View/View2D.h>
#include "vtkSkeletonWidget.h"

// Qt
#include <QEvent>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  SkeletonWidget::SkeletonWidget()
  : m_command  {vtkSkeletonWidgetCommand::New()}
  , m_tolerance{1}
  {
    m_command->setWidget(this);
  }
  
  //-----------------------------------------------------------------------------
  SkeletonWidget::~SkeletonWidget()
  {
    for(auto view: m_widgets.keys())
      unregisterView(view);

    m_widgets.clear();
    m_command = nullptr;
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::registerView(RenderView* view)
  {
    auto view2d = dynamic_cast<View2D *>(view);

    if (!view2d || m_widgets.keys().contains(view))
      return;

    // TODO: create vtk widget and add widget command as observer.
    // m_widgets.insert(view2d, widget);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::unregisterView(RenderView* view)
  {
    if (!m_widgets.keys().contains(view))
      return;

    auto widget = m_widgets[view];

    // TODO: un-initialize & remove widget from view.

    m_widgets.remove(view);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setEnabled(bool enable)
  {
    for(auto widget: m_widgets.values())
      widget->SetEnabled(enable);
  }

  //-----------------------------------------------------------------------------
  bool SkeletonWidget::filterEvent(QEvent* e, RenderView* view)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent *ke = reinterpret_cast<QKeyEvent*>(e);

      // TODO: manage tool keypresses.
    }

    return false;
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setInUse(bool value)
  {
    if(m_inUse == value)
      return;

    m_inUse = value;

    emit eventHandlerInUse(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonWidget::setTolerance(int value)
  {
    m_tolerance = value;
  }

  //-----------------------------------------------------------------------------
  void vtkSkeletonWidgetCommand::Execute(vtkObject* caller, unsigned long int eventId, void *callData)
  {
    SkeletonWidget *eWidget = dynamic_cast<SkeletonWidget *>(m_widget);

    if (strcmp("vtkSkeletonWidget", caller->GetClassName()) == 0)
    {
      // TODO
    }
  }

} // namespace ESPINA

