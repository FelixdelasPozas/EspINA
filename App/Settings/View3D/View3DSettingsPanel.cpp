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

  QStandardItemModel *active, *available;

  active    = new QStandardItemModel(this);
  available = new QStandardItemModel(this);

  for(auto renderer : m_renderers)
  {
    if (!canRender(renderer, RendererType::RENDERER_VIEW3D))
      continue;

    QStandardItem *item = new QStandardItem(renderer->icon(), renderer->name());
    item->setDropEnabled(false);
    item->setDragEnabled(true);
    item->setToolTip(renderer->tooltip());

    bool isActive = false;
    for(auto activeRenderer : m_view->renderers())
    {
      if (renderer->name() == activeRenderer->name()) isActive = true;
    }

    if (isActive)
      active->appendRow(item);
    else
      available->appendRow(item);
  }

  activeRenderers->setModel(active);
  availableRenderers->setModel(available);

  connect(activeRenderers->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(onActivateRenderersDropped()));
  connect(availableRenderers->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(onAvailableRenderersDropped()));
}

//-----------------------------------------------------------------------------
void View3DSettingsPanel::acceptChanges()
{
  RendererSList renderers;

  QAbstractItemModel *activeModel = activeRenderers->model();
  for(int i=0; i < activeModel->rowCount(); i++)
  {
    renderers << renderer(activeModel->index(i,0).data().toString());
  }

  m_view->setRenderers(renderers);
}

//-----------------------------------------------------------------------------
void View3DSettingsPanel::rejectChanges()
{

}

//-----------------------------------------------------------------------------
bool View3DSettingsPanel::modified() const
{
  QSet<QString> current, previous;

  QAbstractItemModel *activeModel = activeRenderers->model();
  for(int i=0; i < activeModel->rowCount(); i++)
  {
    current << activeModel->index(i,0).data().toString();
  }

  for(auto activeRenderer : m_view->renderers())
  {
    previous << activeRenderer->name();
  }

  return current != previous;
}


//-----------------------------------------------------------------------------
SettingsPanelPtr View3DSettingsPanel::clone()
{
  return new View3DSettingsPanel(m_view, m_renderers);
}

//-----------------------------------------------------------------------------
void View3DSettingsPanel::onActivateRenderersDropped()
{
  int activeRows    = activeRenderers->model()->rowCount();
  int availableRows = availableRenderers->model()->rowCount();

  activate  ->setEnabled((availableRows - activeRows) > 0);
  deactivate->setEnabled(activeRows > 0);
}

void View3DSettingsPanel::onAvailableRenderersDropped()
{
  int activeRows    = activeRenderers->model()->rowCount();
  int availableRows = availableRenderers->model()->rowCount();

  activate  ->setEnabled(availableRows  > 0);
  deactivate->setEnabled((activeRows - availableRows) > 0);
}

//-----------------------------------------------------------------------------
RendererSPtr View3DSettingsPanel::renderer(const QString& name) const
{
  RendererSPtr result;

  for(auto renderer : m_renderers)
  {
    if (renderer->name() == name)
    {
      result = renderer;
      break;
    }
  }

  return result;
}
