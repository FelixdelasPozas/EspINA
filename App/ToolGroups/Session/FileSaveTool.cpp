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
#include <Core/IO/SaveThread.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/SupportedFormats.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Settings/Settings.h>
#include <App/AutoSave.h>

// Qt
#include <QThread>
#include <QDateTime>

using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support;
using namespace ESPINA::Core::Utils;

//----------------------------------------------------------------------------
FileSaveTool::FileSaveTool(const QString &id,
                           const QString &icon,
                           const QString &tooltip,
                           Context       &context,
                           AnalysisSPtr  &analysis,
                           EspinaErrorHandlerSPtr handler)
: ProgressTool  {id, icon, tooltip, context}
, m_analysis    (analysis)
, m_errorHandler{handler}
, m_askAlways   {false}
, m_icon        {icon}
, m_thread      {nullptr}
{
  setEnabled(false);

  connect(this, SIGNAL(triggered(bool)),
          this, SLOT(saveAnalysis()));
}

//----------------------------------------------------------------------------
FileSaveTool::~FileSaveTool()
{
  disconnect(this, SIGNAL(triggered(bool)),
             this, SLOT(saveAnalysis()));

  abortTask();
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

  if (!filename.endsWith(".seg") || m_askAlways)
  {
    if(filename.isEmpty())
    {
      auto name = tr("New EspINA session - %1 %2.seg").arg(QDate::currentDate().toString()).arg(QTime::currentTime().toString());
      filename = QDir::home().absoluteFilePath(name);
    }

    QFileInfo file{filename};

    if(!filename.endsWith(".seg"))
    {
      filename = file.baseName() + ".seg";
    }

    filename = DefaultDialogs::SaveFile(tr("Save ESPINA Analysis"),
                                        SupportedFormats().addSegFormat(),
                                        file.absolutePath(),
                                        "seg",
                                        filename);
  }

  return filename;
}

//----------------------------------------------------------------------------
void FileSaveTool::setAlwaysAskUser(bool value)
{
  m_askAlways = value;
}

//----------------------------------------------------------------------------
bool FileSaveTool::saveAnalysis(const QString &filename)
{
  auto successValue = false;

  if (!filename.isEmpty())
  {
    auto isAutoSave = filename.endsWith("espina-autosave.seg");

    if(m_thread != nullptr)
    {
      if(!isAutoSave)
      {
        auto message = tr("Session is currently being saved in '%1'").arg(m_thread->filename().fileName());
        auto title   = tr("Error saving file");

        DefaultDialogs::ErrorMessage(message, title);
      }

      return false;
    }

    emit aboutToSaveSession();

    auto reporter = std::make_shared<ChunkReporter>(1, this);
    auto info     = QFileInfo{filename};
    info.refresh();

    ESPINA_SETTINGS(settings);

    if (isAutoSave && settings.value(AutoSave::INTHREAD, true).toBool())
    {
      setIcon(saveIcon());

      m_thread = std::make_shared<SaveThread>(getContext().scheduler(), m_analysis.get(), info, reporter, m_errorHandler);
      m_thread->setHidden(false);
      m_thread->setDescription(tr("Session Auto-save"));

      connect(m_thread.get(), SIGNAL(finished()),
              this,           SLOT(onSaveThreadFinished()));

      Task::submit(m_thread);
    }
    else
    {
      WaitingCursor cursor;

      try
      {
        SegFile::save(m_analysis.get(), info, reporter.get(), m_errorHandler);

        successValue = true;
      }
      catch(const EspinaException &e)
      {
        auto message = tr("Couldn't save file: '%1'").arg(filename.split('/').last());
        auto title   = tr("Error saving file");

        DefaultDialogs::ErrorMessage(message, title, e.details());
      }

      emit sessionSaved(filename, successValue);
    }

    setProgress(100);
  }

  return successValue;
}

//----------------------------------------------------------------------------
void FileSaveTool::saveAnalysis()
{
  saveAnalysis(saveFilename());
}

//----------------------------------------------------------------------------
void FileSaveTool::onSaveThreadFinished()
{
  bool success = false;
  QString filename = tr("Unknown");

  auto thread = dynamic_cast<SaveThread *>(sender());

  if(thread)
  {
    filename = thread->filename().absoluteFilePath();
    success  = thread->successful();

    if(!thread->successful())
    {
      auto message = tr("Couldn't save file: '%1'").arg(filename.split('/').last());
      auto title   = tr("Error saving file");

      DefaultDialogs::ErrorMessage(message, title, thread->errorMessage());
    }
  }

  if(m_thread)
  {
    disconnect(m_thread.get(), SIGNAL(finished()),
               this,           SLOT(onSaveThreadFinished()));

    m_thread = nullptr;
  }

  emit sessionSaved(filename, success);

  setProgress(100);

  setIcon(defaultIcon());
}

//----------------------------------------------------------------------------
QIcon FileSaveTool::saveIcon() const
{
  return QIcon(":/espina/file_auto_save.svg");
}

//----------------------------------------------------------------------------
QIcon FileSaveTool::defaultIcon() const
{
  return m_icon;
}

//----------------------------------------------------------------------------
void FileSaveTool::abortTask()
{
  if(m_thread)
  {
    disconnect(m_thread.get(), SIGNAL(finished()),
               this,           SLOT(onSaveThreadFinished()));

    m_thread->abort();

    if(!m_thread->thread()->wait(5000))
    {
      m_thread->thread()->terminate();
    }

    m_thread = nullptr;
  }
}
