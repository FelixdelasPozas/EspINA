/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "EditorToolBar.h"

#include <selection/SelectionManager.h>
#include <selection/PixelSelector.h>

#include <QAction>
#include <EspinaCore.h>
#include <editor/ImageLogicCommand.h>

//----------------------------------------------------------------------------
EditorToolBar::EditorToolBar(QWidget* parent)
: QToolBar(parent)
, m_combineAction(addAction(tr("Compine Select Segmentations")))
{
//   setWindowTitle("Editor Tool Bar");
  setObjectName("EditorToolBar");

  m_combineAction->setIcon(QIcon(":/espina/add.svg"));
  connect(m_combineAction, SIGNAL(triggered(bool)),
	  this, SLOT(combineSegmentations()));
}

//----------------------------------------------------------------------------
void EditorToolBar::setActivity(QString activity)
{

}

//----------------------------------------------------------------------------
void EditorToolBar::setLOD()
{

}

//----------------------------------------------------------------------------
void EditorToolBar::decreaseLOD()
{

}

//----------------------------------------------------------------------------
void EditorToolBar::increaseLOD()
{

}

//----------------------------------------------------------------------------
void EditorToolBar::combineSegmentations()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  QList<Segmentation *> input;
  foreach(Segmentation *seg, model->segmentations())
  {
    if (seg->isSelected())
      input << seg;
  }

  if (!input.isEmpty())
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::ADDITION));
  }
}
