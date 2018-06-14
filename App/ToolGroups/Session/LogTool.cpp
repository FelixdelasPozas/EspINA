/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <ToolGroups/Session/LogTool.h>
#include <App/Dialogs/LogDialog/LogDialog.h>
#include <App/EspinaMainWindow.h>

using namespace ESPINA;

const QString LOG_FILE = "Extra/operations.log";

//--------------------------------------------------------------------
LogTool::LogTool(Support::Context &context, EspinaMainWindow *window)
: ProgressTool{tr("LogTool"), ":/espina/log.svg", tr("Log of operations done on current analysis"), context}
, m_dialog{nullptr}
{
  setShortcut(Qt::CTRL + Qt::Key_L);
  setCheckable(true);
  setEnabled(false);

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolToggled(bool)));

  connect(window, SIGNAL(analysisChanged()),
          this,   SLOT(onAnalysisLoaded()));

  auto undoStack = context.undoStack();
  connect(undoStack, SIGNAL(indexChanged(int)),
          this,      SLOT(onOperationDone()));

  m_index = undoStack->index();
}

//--------------------------------------------------------------------
LogTool::~LogTool()
{
  if(isChecked()) setChecked(false);
}

//--------------------------------------------------------------------
void LogTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  // time to dump the contents of the log to disk.
  if(!m_log.isEmpty())
  {
    appendMessage(tr("Save data to disk."));
    getModel()->storage()->saveSnapshot(SnapshotData{LOG_FILE, m_log});
  }
}

//--------------------------------------------------------------------
void LogTool::onAnalysisLoaded()
{
  m_log.clear();

  auto storage = getModel()->storage();
  if(storage && storage->exists(LOG_FILE))
  {
    m_log = storage->snapshot(LOG_FILE);
  }

  m_index = getUndoStack()->index();

  updateToolStatus();

  if(m_dialog)
  {
    if(m_log.isEmpty())
    {
      m_dialog->clear();
    }
    else
    {
      m_dialog->setText(m_log);
    }
  }
}

//--------------------------------------------------------------------
void LogTool::onOperationDone()
{
  auto undoStack = getUndoStack();
  auto index     = undoStack->index();

  if(index != m_index)
  {
    auto dateTime  = QDateTime::currentDateTime().toString("[dd.MM.yyyy - hh:mm:ss]");

    if(index - m_index > 0)
    {
      if(undoStack->command(index-1))
      {
        appendMessage(undoStack->command(index - 1)->text());
      }
    }
    else
    {
      if(undoStack->command(index))
      {
        appendMessage("UNDO " + undoStack->command(index)->text());
      }
    }

    m_index = index;
  }

  updateToolStatus();
}

//--------------------------------------------------------------------
void LogTool::onToolToggled(bool value)
{
  switch(value)
  {
    case true:
      if(!m_dialog)
      {
        m_dialog = new LogDialog();
        m_dialog->setText(m_log);
        connect(m_dialog, SIGNAL(destroyed(QObject *)), this, SLOT(onDialogDestroyed()));
        m_dialog->show();
      }
      else
      {
        m_dialog->raise();
      }
      break;
    default:
      if(m_dialog) m_dialog->close();
      break;
  }
}

//--------------------------------------------------------------------
void LogTool::updateToolStatus()
{
  if(m_log.isEmpty())
  {
    if(!m_dialog) setEnabled(false);
    setToolTip(tr("No operations have been done on current analysis."));
  }
  else
  {
    setEnabled(true);
    setToolTip(tr("Log of operations done on current analysis."));
  }
}

//--------------------------------------------------------------------
void ESPINA::LogTool::onDialogDestroyed()
{
  m_dialog = nullptr;
  if(isChecked()) setChecked(false);
}

//--------------------------------------------------------------------
void LogTool::appendMessage(const QString& message)
{
  auto dateTime   = QDateTime::currentDateTime().toString("[dd.MM.yyyy - hh:mm:ss]");
  auto logMessage = dateTime + " " + message + "\n";

  m_log.append(logMessage);

  if(m_dialog) m_dialog->addText(logMessage);
}
