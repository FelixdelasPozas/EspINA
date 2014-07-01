/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "View3DSettingsPanel.h"

#include <QStandardItemModel>

using namespace EspINA;

//-----------------------------------------------------------------------------
View3DSettingsPanel::View3DSettingsPanel(View3D* view, const RendererSList& renderers)
: m_view(view)
, m_renderers(renderers)
{
  setupUi(this);

  showAxis->setVisible(false);

  QStringList activeRenderersList;
  for(auto renderer: m_view->renderers())
    activeRenderersList << renderer->name();

  m_rendererSelector = new RenderersSelector(m_renderers, activeRenderersList, RendererTypes(RendererType::RENDERER_VIEW3D));

  layout()->addWidget(m_rendererSelector);
}

//-----------------------------------------------------------------------------
void View3DSettingsPanel::acceptChanges()
{
  m_view->setRenderers(m_rendererSelector->getActiveRenderers());
}

//-----------------------------------------------------------------------------
void View3DSettingsPanel::rejectChanges()
{

}

//-----------------------------------------------------------------------------
bool View3DSettingsPanel::modified() const
{
  QSet<QString> current, previous;

  for(auto renderer: m_view->renderers())
    previous << renderer->name();

  for(auto renderer: m_rendererSelector->getActiveRenderers())
    current << renderer->name();

  return current != previous;
}


//-----------------------------------------------------------------------------
SettingsPanelPtr View3DSettingsPanel::clone()
{
  return new View3DSettingsPanel(m_view, m_renderers);
}
