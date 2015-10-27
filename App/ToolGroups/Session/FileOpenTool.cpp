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

// ESPINA
#include "FileOpenTool.h"
#include "ChunkReporter.h"

#include "AutoSave.h"
#include "RecentDocuments.h"
#include <Core/IO/ProgressReporter.h>
#include <Core/Utils/AnalysisUtils.h>
#include <EspinaErrorHandler.h>
#include <EspinaMainWindow.h>
#include <GUI/Widgets/Styles.h>

#include <QElapsedTimer>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
FileOpenTool::FileOpenTool(const QString& id, const QString& icon, const QString& tooltip, Support::Context& context, EspinaErrorHandlerSPtr handler)
: ProgressTool(id, icon, tooltip, context)
, m_errorHandler{handler}
, m_closeCallaback{nullptr}
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onTriggered()));
}

//----------------------------------------------------------------------------
QStringList FileOpenTool::loadedFiles() const
{
  return m_loadedFiles;
}

//----------------------------------------------------------------------------
void FileOpenTool::setCloseCallback(EspinaMainWindow *callback)
{
  m_closeCallaback = callback;
}

//----------------------------------------------------------------------------
void FileOpenTool::onTriggered()
{
  if (m_closeCallaback && !m_closeCallaback->closeCurrentAnalysis()) return;

  auto title  = tr("Open Analysis");
  auto filter = getFactory()->supportedFileExtensions();

  RecentDocuments recent;

  auto files = DefaultDialogs::OpenFiles(title, filter, DefaultDialogs::DefaultPath(), recent.recentDocumentUrls());

  if (!files.isEmpty())
  {
    auto fileInfo = QFileInfo(files.first());
    m_errorHandler->setDefaultDir(fileInfo.absoluteDir());

    load(files);
  }
}

//----------------------------------------------------------------------------
void FileOpenTool::loadAnalysis(const QString& file)
{
  load(QStringList(file));
}

//----------------------------------------------------------------------------
void FileOpenTool::load(const QStringList &files)
{
  m_loadedFiles.clear();

  QElapsedTimer timer;
  timer.start();

  WaitingCursor cursor;

  AnalysisSPtr analysis;

  QList<AnalysisSPtr> analyses;
  QStringList failedFiles;

  auto factory = getContext().factory();

  ChunkReporter reporter(files.size(), this);

  reporter.setProgress(0);

  AutoSave autoSave;
  RecentDocuments recent;

  for (auto file : files)
  {
    m_errorHandler->setDefaultDir(QFileInfo(file).dir());

    auto readers = getContext().factory()->readers(file);

    if (readers.isEmpty())
    {
      auto title = tr("File Extension is not supported");
      DefaultDialogs::InformationMessage(file, title);

      continue;
    }

    auto reader = readers.first();

    if (readers.size() > 1)
    {
      //TODO 2015-04-20: show reader selection dialog
    }

    try
    {
      analyses << factory->read(reader, file, &reporter, m_errorHandler);

      m_loadedFiles << file;

      if (!autoSave.isAutoSaveFile(file))
      {
        recent.addDocument(file);
      }

      reporter.nextChunk();
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

    auto number = (failedFiles.size() > 1) ? QString("them") : QString("it");
    message.append(tr("Do you want to remove %1 from the recent folders list?").arg(number));

    if (DefaultDialogs::UserConfirmation(message))
    {
      for (auto file : failedFiles)
      {
        recent.removeDocument(file);
      }
    }
  }

  if (!analyses.isEmpty())
  {
    analysis = analyses.first();

    for(int i = 1; i < analyses.size(); ++i)
    {
      analysis = merge(analysis, analyses[i]);
    }
  }

  int secs = timer.elapsed()/1000.0;
  int mins = 0;
  if (secs > 60)
  {
    mins = secs / 60;
    secs = secs % 60;
  }

  qDebug() << QString("File Loaded in %1:%2 seconds").arg(mins).arg(secs);

  reporter.setProgress(100);

  if(analysis)
  {
    emit analysisLoaded(analysis);
  }
}


