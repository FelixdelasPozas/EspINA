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
#include "SkeletonToolGroup.h"

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  SkeletonToolGroup::SkeletonToolGroup(ModelAdapterSPtr model,
                                       ModelFactorySPtr factory,
                                       ViewManagerSPtr  viewManager,
                                       QUndoStack      *undoStack,
                                       QObject *parent)
  : ToolGroup  {viewManager, QIcon(":/espina/tubular.svg"), tr("Skeleton tools."), parent}
  , m_model    {model}
  , m_factory  {factory}
  , m_undoStack{undoStack}
  , m_tool     {new SkeletonTool{model, viewManager}}
  , m_enabled  {true}
  {
    connect(m_tool.get(), SIGNAL(stoppedOperation()),
            this,         SLOT(createSegmentation()), Qt::QueuedConnection);
  }
  
  //-----------------------------------------------------------------------------
  SkeletonToolGroup::~SkeletonToolGroup()
  {
    disconnect(m_tool.get(), SIGNAL(stoppedOperation()),
               this,         SLOT(createSegmentation()));
  }

  //-----------------------------------------------------------------------------
  void SkeletonToolGroup::setEnabled(bool value)
  {
    m_enabled = value;

    m_tool->setEnabled(value);
  }

  //-----------------------------------------------------------------------------
  ToolSList SkeletonToolGroup::tools()
  {
    ToolSList list;
    list << m_tool;

    return list;
  }

  //-----------------------------------------------------------------------------
  void SkeletonToolGroup::createSegmentation()
  {
    auto skeleton = m_tool->getSkeleton();
    auto item = m_tool->getSelectedItem();

    if(item)
    {
      // TODO: add the skeleton to the item (undecided if add or replace if another exists).
    }
    else
    {
      auto category = m_tool->getSelectedCategory();

      // TODO: create segmentation, assign outputs.
    }

  }
} // namespace EspINA
