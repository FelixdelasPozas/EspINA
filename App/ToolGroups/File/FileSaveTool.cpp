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
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/SupportedFormats.h>
#include <ToolGroups/File/FileSaveTool.h>

using ESPINA::GUI::DefaultDialogs;
using ESPINA::GUI::SupportedFormats;

using namespace ESPINA;

//----------------------------------------------------------------------------
FileSaveTool::FileSaveTool(Support::Context& context)
: ProgressTool{"FileSave",  ":/espina/file_save.svg", tr("Save current session"), context}
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onTriggered()));
}

//----------------------------------------------------------------------------
void FileSaveTool::setSessionFile(const QString &filename)
{
  m_sessionFile = QFileInfo(filename);

  auto enabled = m_sessionFile.suffix().toLower().compare(".seg");
  setEnabled(enabled);
}

//----------------------------------------------------------------------------
void FileSaveTool::save(const QString &filename)
{
//  QString suggestedFileName;
//  if (m_sessionFile.suffix().toLower() == "seg")
//  {
//    suggestedFileName = m_sessionFile.fileName();
//  }
//  else
//  {
//    suggestedFileName = m_sessionFile.baseName() + QString(".seg");
//  }
//
//  auto analysisFile = DefaultDialogs::SaveFile(tr("Save ESPINA Analysis"),
//                                               SupportedFormats().addSegFormat(),
//                                               m_sessionFile.absolutePath(),
//                                               "seg",
//                                               suggestedFileName);
//
//  if (analysisFile.isEmpty()) return;
//
//  Q_ASSERT(analysisFile.toLower().endsWith(tr(".seg")));
//
//  saveAnalysis(analysisFile);
//
//  QStringList fileParts = analysisFile.split(QDir::separator());
//  setWindowTitle(fileParts[fileParts.size()-1]);
//
//  m_saveSessionAnalysis->setEnabled(true);
//  m_sessionFile = analysisFile;
}

//----------------------------------------------------------------------------
void FileSaveTool::onTriggered()
{
  if(m_sessionFile == QFileInfo()) return;

  save(m_sessionFile.fileName());
}

