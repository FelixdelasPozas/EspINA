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
#include <pluginInterfaces/Renderer.h>

#include <QStandardItemModel>

//-----------------------------------------------------------------------------
VolumeViewSettingsPanel::VolumeViewSettingsPanel(VolumeView::SettingsPtr settings)
: m_settings(settings)
{
  setupUi(this);

  QStandardItemModel *active, *available;

  active    = new QStandardItemModel(this);
  available = new QStandardItemModel(this);

  foreach(Renderer *renderer, EspinaFactory::instance()->renderers())
  {
    QStandardItem *item = new QStandardItem(renderer->icon(), renderer->name());
    item->setToolTip(renderer->tooltip());
    available->appendRow(item);
  }

  activeRenderers->setModel(active);
  availableRenderers->setModel(available);
}

//-----------------------------------------------------------------------------
void VolumeViewSettingsPanel::acceptChanges()
{
  QList<Renderer *> renderers;
  QMap<QString, Renderer *> rendererFactory = EspinaFactory::instance()->renderers();

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
  return ISettingsPanel::modified();
}


//-----------------------------------------------------------------------------
ISettingsPanel* VolumeViewSettingsPanel::clone()
{
  return new VolumeViewSettingsPanel(m_settings);
}

