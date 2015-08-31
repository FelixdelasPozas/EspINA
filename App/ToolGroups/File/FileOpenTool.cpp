/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "FileOpenTool.h"

#include <EspinaErrorHandler.h>
#include <Core/Utils/AnalysisUtils.h>

using namespace ESPINA;
using namespace ESPINA::GUI;

//----------------------------------------------------------------------------
FileOpenTool::FileOpenTool(Support::Context &context, EspinaErrorHandlerSPtr errorHandler)
: ProgressTool  {"FileOpen",  ":/espina/file_open.svg", tr("Open As New Analysis"), context}
, m_errorHandler{errorHandler}
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onTriggered()));
}

//----------------------------------------------------------------------------
QStringList FileOpenTool::files() const
{
  return m_selectedFiles;
}

//----------------------------------------------------------------------------
void FileOpenTool::onTriggered()
{
  auto title = tr("Start New Analysis From File");
  m_selectedFiles = DefaultDialogs::OpenFiles(title, getFactory()->supportedFileExtensions());

  if (!m_selectedFiles.isEmpty())
  {
    load(m_selectedFiles);
  }
}

//----------------------------------------------------------------------------
void FileOpenTool::load(const QStringList &files)
{
  AnalysisSPtr analysis;
  QList<AnalysisSPtr> analyses;
  QStringList loadedFiles, failedFiles;
  auto factory = getContext().factory();
  int i = 0;

  for(auto file : files)
  {
    setProgress(i);

    m_errorHandler->setDefaultDir(QFileInfo(file).dir());

    auto readers = getContext().factory()->readers(file);

    if (readers.isEmpty())
    {
      auto title = tr("File Extension is not supported");
      DefaultDialogs::InformationMessage(title, file);
      continue;
    }

    auto reader = readers.first();

    if (readers.size() > 1)
    {
      //TODO 2015-04-20: show reader selection dialog
    }

    try
    {
      analyses << factory->read(reader, file, m_errorHandler);

      loadedFiles << file;

      setProgress((++i * 100)/files.size());
    }
    catch (...)
    {
      failedFiles << file;
    }
  }

  if(!failedFiles.empty())
  {
    auto message = tr("The following files couldn't be loaded:\n");

    for(auto file: failedFiles)
    {
      message.append(QString("%1\n").arg(file));
    }

    DefaultDialogs::InformationMessage(tr("ESPINA"), message);
  }

  if (!analyses.isEmpty())
  {
    analysis = analyses.first();

    for(int i = 1; i < analyses.size(); ++i)
    {
      analysis = merge(analysis, analyses[i]);
    }
  }

  setProgress(100);

  emit analysisLoaded(analysis);
}


