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
#include <model/EspinaFactory.h>
#include "common/gui/Renderer.h"

#include <QStandardItemModel>

//-----------------------------------------------------------------------------
VolumeViewSettingsPanel::VolumeViewSettingsPanel(const EspinaFactory *factory,
                                                 VolumeView::SettingsPtr settings)
: m_factory(factory)
, m_settings(settings)
{
  setupUi(this);

  QStandardItemModel *active, *available;

  active    = new QStandardItemModel(this);
  available = new QStandardItemModel(this);

  foreach(Renderer *renderer, m_factory->renderers())
  {
    QStandardItem *item = new QStandardItem(renderer->icon(), renderer->name());
    item->setDropEnabled(false);
    item->setDragEnabled(true);
    item->setToolTip(renderer->tooltip());
    bool isActive = false;
    foreach(Renderer* activeRenderer, m_settings->renderers())
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
  QList<Renderer *> renderers;
  QMap<QString, Renderer *> rendererFactory = m_factory->renderers();

  QAbstractItemModel *activeModel = activeRenderers->model();
  for(int i=0; i < activeModel->rowCount(); i++)
  {
    renderers << rendererFactory[activeModel->index(i,0).data().toString()];
  }

  m_settings->setRenderers(renderers);
}

//-----------------------------------------------------------------------------
bool VolumeViewSettingsPanel::modified() const
{
  QSet<QString> current, previous;

  QAbstractItemModel *activeModel = activeRenderers->model();
  for(int i=0; i < activeModel->rowCount(); i++)
    current << activeModel->index(i,0).data().toString();

  foreach(Renderer* activeRenderer, m_settings->renderers())
    previous << activeRenderer->name();

  return current != previous;
}


//-----------------------------------------------------------------------------
ISettingsPanel* VolumeViewSettingsPanel::clone()
{
  return new VolumeViewSettingsPanel(m_factory, m_settings);
}

