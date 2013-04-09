/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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
#include "VolumeViewSettingsPanel.h"
#include <Core/Model/EspinaFactory.h>

#include <QStandardItemModel>

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeViewSettingsPanel::VolumeViewSettingsPanel(const EspinaFactoryPtr factory,
                                                 VolumeView::SettingsPtr settings)
: m_factory(factory)
, m_settings(settings)
{
  setupUi(this);

  showAxis->setVisible(false);

  QStandardItemModel *active, *available;

  active    = new QStandardItemModel(this);
  available = new QStandardItemModel(this);

  foreach(IRenderer *renderer, m_factory->renderers())
  {
    QStandardItem *item = new QStandardItem(renderer->icon(), renderer->name());
    item->setDropEnabled(false);
    item->setDragEnabled(true);
    item->setToolTip(renderer->tooltip());
    bool isActive = false;
    foreach(IRenderer *activeRenderer, m_settings->renderers())
    {
      if (renderer->name() == activeRenderer->name())
        isActive = true;
    }
    if (isActive)
      active->appendRow(item);
    else
      available->appendRow(item);
  }

  activeRenderers->setModel(active);
  availableRenderers->setModel(available);
}

//-----------------------------------------------------------------------------
void VolumeViewSettingsPanel::acceptChanges()
{
  IRendererList renderers;
  QMap<QString, IRenderer *> rendererFactory = m_factory->renderers();

  QAbstractItemModel *activeModel = activeRenderers->model();
  for(int i=0; i < activeModel->rowCount(); i++)
  {
    renderers << rendererFactory[activeModel->index(i,0).data().toString()];
  }

  m_settings->setRenderers(renderers);
}

//-----------------------------------------------------------------------------
void VolumeViewSettingsPanel::rejectChanges()
{

}

//-----------------------------------------------------------------------------
bool VolumeViewSettingsPanel::modified() const
{
  QSet<QString> current, previous;

  QAbstractItemModel *activeModel = activeRenderers->model();
  for(int i=0; i < activeModel->rowCount(); i++)
    current << activeModel->index(i,0).data().toString();

  foreach(IRenderer *activeRenderer, m_settings->renderers())
    previous << activeRenderer->name();

  return current != previous;
}


//-----------------------------------------------------------------------------
ISettingsPanelPtr VolumeViewSettingsPanel::clone()
{
  return ISettingsPanelPtr(new VolumeViewSettingsPanel(m_factory, m_settings));
}
