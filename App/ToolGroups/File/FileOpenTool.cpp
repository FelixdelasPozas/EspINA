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

class FileOpenTool::LoadTask
: public Task
{
public:
  explicit LoadTask(const QStringList &files,
                    ModelFactorySPtr factory,
                    SchedulerSPtr scheduler);

  AnalysisSPtr analysis() const
  { return m_analysis; }

private:
  virtual void run();

private:
  QStringList      m_files;
  ModelFactorySPtr m_factory;

  AnalysisSPtr     m_analysis;
};

//----------------------------------------------------------------------------
FileOpenTool::LoadTask::LoadTask(const QStringList &files,
                           ModelFactorySPtr factory,
                           SchedulerSPtr scheduler)
: Task(scheduler)
, m_files(files)
, m_factory(factory)
{

}

//----------------------------------------------------------------------------
void FileOpenTool::LoadTask::run()
{
  QList<AnalysisSPtr> analyses;

  reportProgress(0);

  for(auto file : m_files)
  {
    auto errorHandler = std::make_shared<EspinaErrorHandler>();

    errorHandler->setDefaultDir(QFileInfo(file).dir());

    auto readers = m_factory->readers(file);

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
      analyses << m_factory->read(reader, file, errorHandler);

//       if (file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
//       {
//         m_recentDocuments1.addDocument(file);
//         m_recentDocuments2.updateDocumentList();
//       }
    }
    catch (...)
    {
//       QApplication::restoreOverrideCursor();
//
//       if(file != m_settings->autosavePath().absoluteFilePath(AUTOSAVE_FILE))
//       {
//         auto message = tr("File \"%1\" could not be loaded.\n"
//                           "Do you want to remove it from recent documents list?")
//                           .arg(file);
//
//         if (DefaultDialogs::UserConfirmation(windowTitle(), message))
//         {
//           m_recentDocuments1.removeDocument(file);
//           m_recentDocuments2.updateDocumentList();
//         }
//       }
//       else
//       {
        auto message = tr("The autosave file could not be loaded.\n");
        DefaultDialogs::InformationMessage(tr("ESPINA"), message);
//       }
//       QApplication::setOverrideCursor(Qt::WaitCursor);
    }
  }

  if (!analyses.isEmpty())
  {
        m_analysis = analyses.first();

    for(int i = 1; i < analyses.size(); ++i)
    {
      m_analysis = merge(m_analysis, analyses[i]);
    }
  }

  reportProgress(100);
}

//----------------------------------------------------------------------------
FileOpenTool::FileOpenTool(Support::Context &context)
: ProgressTool("FileOpen",  ":/espina/file_open.svg", tr("Open As New Analysis"), context)
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onTriggered()));
}

//----------------------------------------------------------------------------
void FileOpenTool::onTriggered()
{
  auto title = tr("Start New Analysis From File");
  auto selectedFiles = DefaultDialogs::OpenFiles(title, getFactory()->supportedFileExtensions());

  if (!selectedFiles.isEmpty())
  {
    m_loadTask = std::make_shared<LoadTask>(selectedFiles, getFactory(), getScheduler());

    connect(m_loadTask.get(), SIGNAL(finished()),
            this,             SLOT(onTaskFinished()));

    showTaskProgress(m_loadTask);

    setEnabled(false);

    Task::submit(m_loadTask);
  }
}


//----------------------------------------------------------------------------
void FileOpenTool::onTaskFinished()
{
  setEnabled(true);

  if (m_loadTask->analysis())
  {
    emit analysisLoaded(m_loadTask->analysis());
  }
}
