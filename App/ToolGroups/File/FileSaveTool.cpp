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
#include <Core/IO/SegFile.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/SupportedFormats.h>
#include <GUI/Widgets/Styles.h>
#include <ToolGroups/File/FileSaveTool.h>

using ESPINA::GUI::DefaultDialogs;
using ESPINA::GUI::SupportedFormats;
using ESPINA::IO::SegFile::save;

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets::Styles;

//----------------------------------------------------------------------------
FileSaveTool::FileSaveTool(Support::Context& context, AnalysisSPtr analysis, EspinaErrorHandlerSPtr errorHandler, GeneralSettingsSPtr settings)
: ProgressTool  {"FileSave",  ":/espina/file_save.svg", tr("Save current session"), context}
, m_analysis    {analysis}
, m_errorHandler{errorHandler}
, m_settings    {settings}
{
  setEnabled(false);
  setCheckable(false);

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onTriggered()));

  m_autosave.setInterval(settings->autosaveInterval()*60*1000);
  m_autosave.start();
  updateUndoStackIndex();

  connect(&m_autosave, SIGNAL(timeout()),
          this,        SLOT(autosave()));
}

//----------------------------------------------------------------------------
FileSaveTool::~FileSaveTool()
{
  QDir autosavePath = m_settings->autosavePath();
  autosavePath.remove(GeneralSettings::AUTOSAVE_FILE);
}

//----------------------------------------------------------------------------
void FileSaveTool::updateUndoStackIndex()
{
  m_undoStackIndex = getContext().undoStack()->index();
}

//----------------------------------------------------------------------------
void FileSaveTool::setSessionFile(const QString &filename)
{
  if(filename.isEmpty())
  {
    m_sessionFile = QFileInfo();
    setEnabled(false);
  }
  else
  {
    m_sessionFile = QFileInfo(filename);

    auto enabled = m_sessionFile.suffix().toLower().compare(".seg");
    setEnabled(enabled);
  }
}

//----------------------------------------------------------------------------
const QString FileSaveTool::fileName() const
{
  return m_sessionFile.fileName();
}

//----------------------------------------------------------------------------
void FileSaveTool::save(const QString &filename)
{
  WaitingCursor cursor;

  emit aboutToSaveSession();

  IO::SegFile::save(m_analysis.get(), filename, m_errorHandler);

  emit sessionSaved(filename);
}

//----------------------------------------------------------------------------
void FileSaveTool::onTriggered()
{
  if ((m_sessionFile == QFileInfo()) || m_analysis == nullptr) return;

  save(m_sessionFile.fileName());
}

//----------------------------------------------------------------------------
void FileSaveTool::autosave()
{
  m_autosave.setInterval(m_settings->autosaveInterval()*60*1000);

  if(getContext().undoStack()->index() == m_undoStackIndex) return;

  QDir autosavePath = m_settings->autosavePath();
  if (!autosavePath.exists())
  {
    autosavePath.mkpath(autosavePath.absolutePath());
  }

  auto autosaveFilename = autosavePath.absoluteFilePath(GeneralSettings::AUTOSAVE_FILE);

  save(autosaveFilename);

  updateUndoStackIndex();
}

//----------------------------------------------------------------------------
FileSaveAsTool::FileSaveAsTool(Support::Context& context, AnalysisSPtr analysis, EspinaErrorHandlerSPtr errorHandler, GeneralSettingsSPtr settings)
: FileSaveTool{context, analysis, errorHandler, settings}
{
  this->setIcon(QIcon(":/espina/file_save_as.svg"));

  m_autosave.stop();
}

//----------------------------------------------------------------------------
void FileSaveAsTool::onTriggered()
{
  QString suggestedFileName;
  if (m_sessionFile.suffix().toLower() == "seg")
  {
    suggestedFileName = m_sessionFile.fileName();
  }
  else
  {
    suggestedFileName = m_sessionFile.baseName() + QString(".seg");
  }

  auto fileName = DefaultDialogs::SaveFile(tr("Save ESPINA Analysis"),
                                               SupportedFormats().addSegFormat(),
                                               m_sessionFile.absolutePath(),
                                               "seg",
                                               suggestedFileName);

  if (fileName.isEmpty()) return;

  save(fileName);

  m_sessionFile = QFileInfo(fileName);
}
