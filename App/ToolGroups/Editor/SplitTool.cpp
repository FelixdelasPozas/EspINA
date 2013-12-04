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

#include "SplitTool.h"

#include <QAction>
#include <QToolButton>
#include <QDebug>

namespace EspINA
{
  //------------------------------------------------------------------------
  SplitTool::SplitTool(ModelAdapterSPtr model,
                       ModelFactorySPtr factory,
                       ViewManagerSPtr  viewManager,
                       QUndoStack      *undoStack)
  : m_model(model)
  , m_factory(factory)
  , m_viewManager(viewManager)
  , m_undoStack(undoStack)
  , m_splitToolSelector(new ActionSelector())
  , m_enabled(false)
  {
    QAction *planarSplit = new QAction(QIcon(":/espina/planar_split.svg"),
                                       tr("Split segmentation"),
                                       m_splitToolSelector);

//    PlanarSplitToolSPtr planarSplitTool(new PlanarSplitTool(m_model,
//                                                            m_undoStack,
//                                                            m_viewManager));
//    connect(planarSplitTool.get(), SIGNAL(splittingStopped()),
//            this, SLOT(cancelSplitOperation()));

//    m_splitTools[planarSplit] = planarSplitTool;
    m_splitTools[planarSplit] = nullptr;
    m_splitToolSelector->addAction(planarSplit);


//    // Add Split Tool Selector to Editor Tool Bar
//    addAction(m_splitToolSelector);
//    connect(m_splitToolSelector, SIGNAL(triggered(QAction*)),
//            this, SLOT(changeSplitTool(QAction*)));
//    connect(m_splitToolSelector, SIGNAL(actionCanceled()),
//            this, SLOT(cancelSplitOperation()));

    m_splitToolSelector->setDefaultAction(planarSplit);
  }

  //------------------------------------------------------------------------
  SplitTool::~SplitTool()
  {
  }

  //------------------------------------------------------------------------
  void SplitTool::setEnabled(bool value)
  {
    m_enabled = value;
    m_splitToolSelector->setEnabled(value);
  }

  //------------------------------------------------------------------------
  bool SplitTool::enabled() const
  {
    return m_enabled;
  }

  //------------------------------------------------------------------------
  QList<QAction *> SplitTool::actions() const
  {
    QList<QAction *> actions;
    actions << m_splitToolSelector;

    return actions;
  }


} // namespace EspINA
