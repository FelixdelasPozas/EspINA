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
#include "FileSaveTool.h"
#include "ChunkReporter.h"

#include <Core/IO/SegFile.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/SupportedFormats.h>
#include <GUI/Widgets/Styles.h>

using ESPINA::GUI::DefaultDialogs;
using ESPINA::GUI::SupportedFormats;
using ESPINA::IO::SegFile::save;

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support;

//----------------------------------------------------------------------------
FileSaveTool::FileSaveTool(const QString &id,
                           const QString &icon,
                           const QString &tooltip,
                           Context       &context,
                           AnalysisSPtr  &analysis,
                           EspinaErrorHandlerSPtr handler)
: ProgressTool(id, icon, tooltip, context)
, m_analysis(analysis)
, m_errorHandler{handler}
{
  setEnabled(false);

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(saveAnalysis()));

//   updateUndoStackIndex();

}

//----------------------------------------------------------------------------
FileSaveTool::~FileSaveTool()
{
}

//----------------------------------------------------------------------------
void FileSaveTool::updateUndoStackIndex()
{
  m_undoStackIndex = getContext().undoStack()->index();
}

//----------------------------------------------------------------------------
void FileSaveTool::setSaveFilename(const QString& filename)
{
  m_filename = filename;
}

//----------------------------------------------------------------------------
const QString FileSaveTool::saveFilename() const
{
  auto filename = m_filename;

  if (!filename.endsWith(".seg"))
  {
    QFileInfo file = filename;

    filename = file.baseName() + ".seg";

    filename = DefaultDialogs::SaveFile(tr("Save ESPINA Analysis"),
                                        SupportedFormats().addSegFormat(),
                                        file.absolutePath(),
                                        "seg",
                                        filename);
  }

  return filename;
}

//----------------------------------------------------------------------------
void FileSaveTool::saveAnalysis(const QString &filename)
{
  if (!filename.isEmpty())
  {
    WaitingCursor cursor;

    emit aboutToSaveSession();

    auto current = icon();

    if (filename.endsWith("espina-autosave.seg"))
    {
      setIcon(QIcon(":/espina/file_auto_save.svg"));
    }

    ChunkReporter reporter(1, this);

    IO::SegFile::save(m_analysis.get(), filename, &reporter, m_errorHandler);

    setIcon(current);

    emit sessionSaved(filename);
  }
}

//----------------------------------------------------------------------------
void FileSaveTool::saveAnalysis()
{
  saveAnalysis(saveFilename());
}