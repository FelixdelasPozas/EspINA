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
#include <GUI/View/ViewRendererMenu.h>
#include <GUI/Representations/Renderers/Renderer.h>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  ViewRendererMenu::ViewRendererMenu(QWidget* parent)
  : QMenu{parent}
  {
    connect(this, SIGNAL(triggered(QAction *)), this, SLOT(activate(QAction *)), Qt::QueuedConnection);
  }

  //-----------------------------------------------------------------------------
  ViewRendererMenu::~ViewRendererMenu()
  {
    RendererSList renderers = m_renderers.values();
    for (auto renderer: renderers)
      remove(renderer);
  }

  //-----------------------------------------------------------------------------
  void ViewRendererMenu::add(RendererSPtr renderer)
  {
    if (m_renderers.values().contains(renderer))
      return;

    QAction *rendererAction = new QAction(this);
    rendererAction->setCheckable(true);
    rendererAction->setChecked(!renderer->isHidden());
    rendererAction->setText(renderer->name());
    rendererAction->setIcon(renderer->icon());
    rendererAction->setToolTip(renderer->tooltip());

    addAction(rendererAction);
    m_renderers.insert(rendererAction, renderer);
  }

  //-----------------------------------------------------------------------------
  void ViewRendererMenu::remove(RendererSPtr renderer)
  {
    if (!m_renderers.values().contains(renderer))
      return;

    QAction *rendererAction = m_renderers.key(renderer);
    removeAction(rendererAction);
    m_renderers.remove(m_renderers.key(renderer));
    delete rendererAction;
  }

  //-----------------------------------------------------------------------------
  void ViewRendererMenu::activate(QAction *action)
  {
    auto renderer = m_renderers.value(action);
    action->setChecked(renderer->isHidden());
    renderer->setEnable(renderer->isHidden());
  }

} /* namespace ESPINA */
