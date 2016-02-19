/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "EspinaErrorHandler.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Core/Utils/SupportedFormats.h>

// Qt
#include <QUrl>
#include <QDesktopServices>
#include <QApplication>
#include <QMessageBox>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
QFileInfo EspinaErrorHandler::fileNotFound(const QFileInfo                     &file,
                                           QDir                                 dir,
                                           const Core::Utils::SupportedFormats &filters,
                                           const QString                       &hint)
{
  QString key = file.absoluteFilePath();

  if (!m_files.contains(key))
  {
    QString locatedFilename;

    if(m_defaultDir.exists(file.fileName()))
    {
      locatedFilename = m_defaultDir.filePath(file.fileName());
    }
    else
    {
      QString title     = (hint.isEmpty())? QObject::tr("Select file for %1:").arg(file.fileName()) : hint;
      QDir    directory = (dir == QDir()) ? m_defaultDir : dir;

      locatedFilename = DefaultDialogs::OpenFile(title, filters, directory.absolutePath());
    }

    if (!locatedFilename.isEmpty())
    {
      m_files[key] = QFileInfo(locatedFilename);
    }
  }

  return m_files.value(key, QFileInfo());
}

//------------------------------------------------------------------------
void EspinaErrorHandler::error(const QString& msg)
{
  QMessageBox::warning(m_parent, "ESPINA", msg);
}

//------------------------------------------------------------------------
void EspinaErrorHandler::warning(const QString& msg)
{
  QMessageBox::warning(m_parent, "ESPINA", msg);
}
