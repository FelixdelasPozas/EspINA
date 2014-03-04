/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include "RenderersSelector.h"

// Qt
#include <QStandardItemModel>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  RenderersSelector::RenderersSelector(RendererSList renderersList,
                                       QStringList activeRenderersList,
                                       RendererTypes filter)
  : m_renderers(renderersList)
  , m_activeRenderers(activeRenderersList)
  {
    setupUi(this);
    
    QStandardItemModel *active, *available;

    active    = new QStandardItemModel(this);
    available = new QStandardItemModel(this);

    for(auto renderer : m_renderers)
    {
      if (!canRender(renderer, filter))
        continue;

      QStandardItem *item = new QStandardItem(renderer->icon(), renderer->name());
      item->setDropEnabled(false);
      item->setDragEnabled(true);
      item->setToolTip(renderer->tooltip());

      bool isActive = false;
      for(auto rendererName : m_activeRenderers)
        if (renderer->name() == rendererName)
          isActive = true;

      if (isActive)
      {
        active->appendRow(item);
        deactivate->setEnabled(true);
      }
      else
      {
        available->appendRow(item);
        activate->setEnabled(true);
      }
    }

    activeRenderers->setModel(active);
    availableRenderers->setModel(available);

    connect(activeRenderers->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onActivateRenderersDropped()));
    connect(availableRenderers->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onAvailableRenderersDropped()));

    connect(activate, SIGNAL(clicked(bool)),
            this, SLOT(activateRenderers()));
    connect(deactivate, SIGNAL(clicked(bool)),
            this, SLOT(deactivateRenderers()));
  }

  //-----------------------------------------------------------------------------
  RenderersSelector::~RenderersSelector()
  {
  }
  
  //-----------------------------------------------------------------------------
  RendererSList RenderersSelector::getActiveRenderers()
  {
    RendererSList renderers;

    QAbstractItemModel *activeModel = activeRenderers->model();
    for(int i=0; i < activeModel->rowCount(); i++)
      renderers << renderer(activeModel->index(i,0).data().toString());

    return renderers;
  }
  
  //-----------------------------------------------------------------------------
  void RenderersSelector::onActivateRenderersDropped()
  {
    int activeRows    = activeRenderers->model()->rowCount();
    int availableRows = availableRenderers->model()->rowCount();

    activate  ->setEnabled((availableRows - activeRows) > 0);
    deactivate->setEnabled(activeRows > 0);
  }

  //-----------------------------------------------------------------------------
  void RenderersSelector::onAvailableRenderersDropped()
  {
    int activeRows    = activeRenderers->model()->rowCount();
    int availableRows = availableRenderers->model()->rowCount();

    activate  ->setEnabled(availableRows  > 0);
    deactivate->setEnabled((activeRows - availableRows) > 0);
  }

  //-----------------------------------------------------------------------------
  void RenderersSelector::activateRenderers()
  {
    moveSelection(availableRenderers, activeRenderers);
    activate->setEnabled(availableRenderers->model()->rowCount() > 0);
    deactivate->setEnabled(true);
  }

  //-----------------------------------------------------------------------------
  void RenderersSelector::deactivateRenderers()
  {
    moveSelection(activeRenderers, availableRenderers);
    activate->setEnabled(true);
    deactivate->setEnabled(activeRenderers->model()->rowCount() > 0);
  }

  //-----------------------------------------------------------------------------
  void RenderersSelector::moveSelection(QListView *source, QListView *destination)
  {
    auto sourceModel      = dynamic_cast<QStandardItemModel *>(source->model());
    auto destinationModel = dynamic_cast<QStandardItemModel *>(destination->model());
    for(auto index : source->selectionModel()->selectedIndexes())
    {
      auto item = sourceModel->item(index.row());
      destinationModel->appendRow(item->clone());
      sourceModel->removeRow(index.row());
    }
  }

  //-----------------------------------------------------------------------------
  RendererSPtr RenderersSelector::renderer(const QString& name) const
  {
    for(auto renderer : m_renderers)
      if (renderer->name() == name)
        return renderer;

    return RendererSPtr();
  }


} /* namespace EspINA */
