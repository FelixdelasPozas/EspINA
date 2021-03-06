/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <ToolGroups/Session/UndoRedoTools.h>

// QT
#include <QApplication>

using namespace ESPINA;

const QString ELLIPSIS{"..."};

//----------------------------------------------------------------------------
UndoRedoTool::UndoRedoTool(Support::Context &context, const QString &id, const QIcon &icon, const QString &tooltip)
: ProgressTool   (id, icon, tooltip, context)
, m_undoStack    {context.undoStack()}
, m_tooltipPrefix{tooltip}
{
  setEnabled(false);

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(doAction()));
}

//----------------------------------------------------------------------------
void UndoRedoTool::stateChanged(bool value)
{
  this->setEnabled(value);
}

//----------------------------------------------------------------------------
void UndoRedoTool::textChanged(const QString &text)
{
  auto tooltip = m_tooltipPrefix + text;
  if(tooltip.length() > 150) tooltip = tooltip.left(147) + ELLIPSIS;

  setToolTip(tooltip);
}

//----------------------------------------------------------------------------
UndoTool::UndoTool(Support::Context &context)
: UndoRedoTool{context, "FileUndo", QIcon(":/espina/edit-undo.svg"), tr("Undo ")}
{
  setShortcut(Qt::CTRL+Qt::Key_Z);

  connect(m_undoStack, SIGNAL(canUndoChanged(bool)),
          this,        SLOT(stateChanged(bool)));
  connect(m_undoStack, SIGNAL(undoTextChanged(QString)),
          this,        SLOT(textChanged(QString)));
}

//----------------------------------------------------------------------------
void UndoTool::doAction()
{
  emit executed();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_undoStack->undo();
  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
RedoTool::RedoTool(Support::Context &context)
: UndoRedoTool{context, "FileRedo", QIcon(":/espina/edit-redo.svg"), tr("Redo ")}
{
  this->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_Z);

  connect(m_undoStack, SIGNAL(canRedoChanged(bool)),
          this,        SLOT(stateChanged(bool)));
  connect(m_undoStack, SIGNAL(redoTextChanged(QString)),
          this,        SLOT(textChanged(QString)));
}

//----------------------------------------------------------------------------
void RedoTool::doAction()
{
  emit executed();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_undoStack->redo();
  QApplication::restoreOverrideCursor();
}
