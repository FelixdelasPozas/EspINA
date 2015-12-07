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
#include "ZipUtils.h"
#include <Core/Utils/EspinaException.h>

// QuaZip
#include <quazip/quazipfile.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
void ZipUtils::AddFileToZip(const QString&    fileName,
                            const QByteArray& content,
                            QuaZip&           zip)
{
  QuaZipFile zFile(&zip);
  QuaZipNewInfo zFileInfo = QuaZipNewInfo(fileName, fileName);

  zFileInfo.externalAttr = 0x01A40000; // Permissions of the files 644
  if (!zFile.open(QIODevice::WriteOnly, zFileInfo))
  {
    auto what    = QObject::tr("Couldn't create a file inside ZIP container, file: %1, cause: %2").arg(fileName).arg(zFile.errorString());
    auto details = QObject::tr("ZipUtils::AddFileToZip() -> Can't create file inside ZIP container, file: %1, cause: %2").arg(fileName).arg(zFile.errorString());
    throw EspinaException(what, details);
  }

  zFile.write(content);
  if (zFile.getZipError() != UNZ_OK)
  {
    auto what    = QObject::tr("Couldn't write a file inside ZIP container, file: %1, cause: %2").arg(fileName).arg(zFile.errorString());
    auto details = QObject::tr("ZipUtils::AddFileToZip() -> Can't write file inside ZIP container, file: %1, cause: %2").arg(fileName).arg(zFile.errorString());
    throw EspinaException(what, details);
  }

  zFile.close();
  if (zFile.getZipError() != UNZ_OK)
  {
    auto what    = QObject::tr("Couldn't close a ZIP container, cause: %1").arg(zFile.errorString());
    auto details = QObject::tr("ZipUtils::AddFileToZip() -> Can't close ZIP container, cause: %1").arg(zFile.errorString());
    throw EspinaException(what, details);
  }
}

//-----------------------------------------------------------------------------
QByteArray ZipUtils::readFileFromZip(const QString& fileName, QuaZip& zip)
{
  if (!zip.setCurrentFile(fileName))
  {
    auto what    = QObject::tr("Couldn't find a file inside a ZIP container, file: %1, error code: %2").arg(fileName).arg(zip.getZipError());
    auto details = QObject::tr("ZipUtils::AddFileToZip() -> Can't find a file inside ZIP container, file: %1, error code: %2").arg(fileName).arg(zip.getZipError());
    throw EspinaException(what, details);
  }

  return readCurrentFileFromZip(zip);
}

//-----------------------------------------------------------------------------
QByteArray ZipUtils::readCurrentFileFromZip(QuaZip& zip)
{
  QuaZipFile zFile(&zip);
  if (!zFile.open(QIODevice::ReadOnly))
  {
    auto what    = QObject::tr("Couldn't open a file inside ZIP container, file: %1, cause: %2").arg(zip.getCurrentFileName()).arg(zFile.errorString());
    auto details = QObject::tr("ZipUtils::AddFileToZip() -> Can't open a file inside ZIP container, file: %1, cause: %2").arg(zip.getCurrentFileName()).arg(zFile.errorString());
    throw EspinaException(what, details);
  }

  return zFile.readAll();
}
