/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include "EditionTools.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  EditionTools::EditionTools(ModelAdapterSPtr model,
                             ModelFactorySPtr factory,
                             ViewManagerSPtr  viewManager,
                             QUndoStack      *undoStack,
                             QWidget         *parent = nullptr)
  : ToolGroup(viewManager, QIcon(":/espina/pencil.png"), tr("Edition Tools"), parent)
  , m_manualEdition(ManualEditionToolSPtr(new ManualEditionTool()))
  , m_split(SplitToolSPtr(new SplitTool()))
  {
    m_morphological = (MorphologicalEditionToolSPtr(new MorphologicalEditionTool(model, factory, viewManager, undoStack)));

    connect(m_viewManager.get(), SIGNAL(selectionChanged(SelectableView::Selection)), this, SLOT(selectionChanged()));
  }

  //-----------------------------------------------------------------------------
  EditionTools::~EditionTools()
  {
    disconnect(m_viewManager.get(), SIGNAL(selectionChanged(SelectableView::Selection)), this, SLOT(selectionChanged()));
  }

  //-----------------------------------------------------------------------------
  void EditionTools::setEnabled(bool value)
  {
    m_enabled = value;

    m_manualEdition->setEnabled(value);
    m_split->setEnabled(value);
    m_morphological->setEnabled(value);
  }

  //-----------------------------------------------------------------------------
  bool EditionTools::enabled() const
  {
    return (m_manualEdition->enabled() || m_split->enabled() || m_morphological->enabled());
  }

  //-----------------------------------------------------------------------------
  ToolSList EditionTools::tools()
  {
    selectionChanged();

    ToolSList availableTools;
    availableTools << m_manualEdition;
    availableTools << m_split;
    availableTools << m_morphological;

    return availableTools;
  }

  //-----------------------------------------------------------------------------
  void EditionTools::selectionChanged()
  {
    SegmentationAdapterList selection  = m_viewManager->selection()->segmentations();
    int listSize = selection.size();
    m_manualEdition->setEnabled(listSize == 1 || listSize == 0);
    m_split->setEnabled(listSize == 1);
    m_morphological->setEnabled(listSize != 0);
  }

} /* namespace EspINA */
