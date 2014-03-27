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

// EspINA
#include <Settings/View2D/View2DRenderersPanel.h>
#include <GUI/View/View2D.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  View2DRenderersPanel::View2DRenderersPanel(RendererSList renderers,
                                             QStringList activeRenderers,
                                             RendererTypes filter,
                                             QList<View2D *> viewList)
  : SettingsPanel()
  , m_renderers(renderers)
  , m_activeRenderers(activeRenderers)
  , m_filter(filter)
  , m_views(viewList)
  {
    m_selector = new RenderersSelector(renderers, activeRenderers, filter);
    setLayout(new QVBoxLayout());
    layout()->addWidget(m_selector);
  }
  
  //-----------------------------------------------------------------------------
  View2DRenderersPanel::~View2DRenderersPanel()
  {
    delete m_selector;
  }

  //-----------------------------------------------------------------------------
  void EspINA::View2DRenderersPanel::acceptChanges()
  {
    for(auto view: m_views)
      view->setRenderers(m_selector->getActiveRenderers());
  }

  //-----------------------------------------------------------------------------
  void EspINA::View2DRenderersPanel::rejectChanges()
  {
  }

  //-----------------------------------------------------------------------------
  bool EspINA::View2DRenderersPanel::modified() const
  {
    QSet<QString> current, previous;

    for(auto name: m_activeRenderers)
      previous << name;

    for(auto renderer: m_selector->getActiveRenderers())
      current << renderer->name();

    return current != previous;
  }

  //-----------------------------------------------------------------------------
  SettingsPanelPtr EspINA::View2DRenderersPanel::clone()
  {
    return new View2DRenderersPanel(m_renderers, m_activeRenderers, m_filter, m_views);
  }

} /* namespace EspINA */

