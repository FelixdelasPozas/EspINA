/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#include <Menus/RenderersMenu.h>
#include <GUI/View/RenderView.h>
#include <QAction>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  RenderersMenu::RenderersMenu(ViewManagerSPtr vm,
                               QWidget* parent)
  : QMenu(QString("Renderers"), parent)
  , m_viewManager(vm)
  {
    m_menu2D = new QMenu("View 2D", this);
    m_menu3D = new QMenu("View 3D", this);
    addMenu(m_menu2D);
    addMenu(m_menu3D);
    connect(m_menu2D, SIGNAL(triggered(QAction *)), this, SLOT(activate(QAction *)), Qt::QueuedConnection);
    connect(m_menu3D, SIGNAL(triggered(QAction *)), this, SLOT(activate(QAction *)), Qt::QueuedConnection);
  }
  
  //-----------------------------------------------------------------------------
  RenderersMenu::~RenderersMenu()
  {
    delete m_menu2D;
    delete m_menu3D;
  }
  
  //-----------------------------------------------------------------------------
  void RenderersMenu::addRenderer(RendererSPtr renderer)
  {
    bool found = false;

    auto action = new QAction(renderer->name(), this);
    action->setIcon(renderer->icon());
    action->setCheckable(true);

    if (renderer->renderType().testFlag(RendererType::RENDERER_VIEW2D))
    {
      for(auto action: m_menu2D->actions())
        if (action->text() == renderer->name())
          found = true;

      if (!found)
      {
        action->setChecked(true);
        m_menu2D->addAction(action);
        return;
      }
    }

    if (renderer->renderType().testFlag(RendererType::RENDERER_VIEW3D))
    {
      for(auto action: m_menu3D->actions())
        if (action->text() == renderer->name())
        {
          found = true;
          break;
        }

      if (!found)
      {
        action->setChecked(false);
        m_menu3D->addAction(action);
        return;
      }
    }

    // already present or does not apply
    delete action;
  }
  
  //-----------------------------------------------------------------------------
  void RenderersMenu::removeRenderer(RendererSPtr renderer)
  {
    for (auto menu: { m_menu2D, m_menu3D })
      for(auto action: menu->actions())
        if (action->text() == renderer->name())
        {
          menu->removeAction(action);
        }
  }
  
  //-----------------------------------------------------------------------------
  void RenderersMenu::clear()
  {
    m_menu2D->clear();
    m_menu3D->clear();
  }

  //-----------------------------------------------------------------------------
  void RenderersMenu::activate(QAction *action)
  {
    for (auto view: m_viewManager->renderViews())
      if (action->isChecked())
        view->activateRender(action->text());
      else
        view->deactivateRender(action->text());
  }

} /* namespace EspINA */
