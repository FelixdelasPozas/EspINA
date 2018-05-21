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
#include <Core/Utils/EspinaException.h>
#include <EspinaErrorHandler.h>
#include <EspinaMainWindow.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>

// Qt
#include <QElapsedTimer>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
FileOpenTool::FileOpenTool(const QString& id, const QString& icon, const QString& tooltip, Support::Context& context, EspinaErrorHandlerSPtr handler)
: ProgressTool   {id, icon, tooltip, context}
, m_errorHandler {handler}
, m_closeCallback{nullptr}
{
  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(onTriggered()));
}

//----------------------------------------------------------------------------
FileOpenTool::~FileOpenTool()
{
  disconnect(this, SIGNAL(triggered(bool)),
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
  m_closeCallback = callback;
}

//----------------------------------------------------------------------------
void FileOpenTool::onTriggered()
{
  auto title  = tr("Open Analysis");
  auto filter = getFactory()->supportedFileExtensions();

  RecentDocuments recent;

  auto files = DefaultDialogs::OpenFiles(title, filter, DefaultDialogs::DefaultPath(), recent.recentDocumentUrls());

  if (!files.isEmpty())
  {
    if (m_closeCallback && !m_closeCallback->closeCurrentAnalysis())
    {
      return;
    }

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

  AnalysisSPtr analysis = nullptr;

  QList<AnalysisSPtr> analyses;
  QStringList failedFiles;
  QString failDetails;

  auto factory = getContext().factory();

  ChunkReporter reporter(files.size(), this);

  reporter.setProgress(0);

  AutoSave autoSave;
  RecentDocuments recent;

  for (auto file : files)
  {
    auto fileInfo = QFileInfo{file};
    m_errorHandler->setDefaultDir(fileInfo.dir());

    auto readers = getContext().factory()->readers(fileInfo);

    if (readers.isEmpty())
    {
      auto title = tr("File Extension is not supported");
      DefaultDialogs::InformationMessage(fileInfo.fileName(), title);

      continue;
    }

    auto reader = readers.first();

    if (readers.size() > 1)
    {
      //TODO 2015-04-20: show reader selection dialog
    }

    try
    {
      analyses << factory->read(reader, fileInfo, &reporter, m_errorHandler);

      m_loadedFiles << file;

      if (!autoSave.isAutoSaveFile(file))
      {
        recent.addDocument(file);
      }

      reporter.nextChunk();
    }
    catch (const EspinaException &e)
    {
      qWarning() << QString("EXCEPTION: error loading file: %1").arg(file);
      qWarning() << e.what();
      qWarning() << e.details();

      failedFiles << fileInfo.fileName();
      failDetails.append(QObject::tr("File %1 error: %2").arg(fileInfo.fileName()).arg(QString(e.what())));
    }
    catch(const itk::ExceptionObject &e)
    {
      qWarning() << QString("EXCEPTION: error loading file: %1").arg(file);
      qWarning() << e.what();
      qWarning() << "File:" << e.GetFile() << "Line: " << e.GetLine();
      qWarning() << "Location:" << e.GetLocation();

      failedFiles << fileInfo.fileName();
      failDetails.append(QObject::tr("File %1 error: %2").arg(fileInfo.fileName()).arg(QString(e.what())));
    }
    catch(...)
    {
      qWarning() << QString("EXCEPTION: unspecified error loading file: %1").arg(file);

      failedFiles << fileInfo.fileName();
    }
  }

  if(!failedFiles.empty())
  {
    auto buttons = QMessageBox::Yes|QMessageBox::Cancel;
    auto message = tr("The following files couldn't be loaded:\n");
    auto title   = tr("EspINA");

    for(auto file: failedFiles)
    {
      message.append(QString("%1\n").arg(file));
    }

    auto number = (failedFiles.size() > 1) ? QString("them") : QString("it");
    message.append(tr("Do you want to remove %1 from the recent folders list?").arg(number));

    if (DefaultDialogs::UserQuestion(message, buttons, title, failDetails) == QMessageBox::Yes)
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

  qDebug() << QString("File Loaded in %1:%2 seconds").arg(mins).arg(secs, 2, 10, QChar('0'));

  reporter.setProgress(100);

  if(analysis.get() != nullptr)
  {
    emit analysisLoaded(analysis);
  }
}


