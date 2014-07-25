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

#include "ZipUtils.h"

#include <quazip/quazipfile.h>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::IO;

//-----------------------------------------------------------------------------
void ZipUtils::AddFileToZip(const QString&    fileName,
                            const QByteArray& content,
                            QuaZip&           zip)
throw (IO_Zip_Exception)
{
  QuaZipFile zFile(&zip);
  QuaZipNewInfo zFileInfo = QuaZipNewInfo(fileName, fileName);

  zFileInfo.externalAttr = 0x01A40000; // Permissions of the files 644
  if (!zFile.open(QIODevice::WriteOnly, zFileInfo))
  {
    qWarning() << "ZipUtils::AddFileToZip(): Could not open" << fileName << "in zip file"
               << "- Code error:" << zFile.getZipError();
    throw (IO_Zip_Exception());
  }

  zFile.write(content);
  if (zFile.getZipError() != UNZ_OK)
  {
    qWarning() << "ZipUtils::AddFileToZip(): Could not write" << fileName << "constent in zip file"
               << "- Code error:" << zFile.getZipError();
    throw (IO_Zip_Exception());
  }

  zFile.close();

  if (zFile.getZipError() != UNZ_OK)
  {
    qWarning() << "ZipUtils::AddFileToZip(): Could not close" << fileName << "in zip file"
               << "- Code error:" << zFile.getZipError();
    throw (IO_Zip_Exception());
  }
}

//-----------------------------------------------------------------------------
QByteArray ZipUtils::readFileFromZip(const QString& fileName, QuaZip& zip)
{
  if (!zip.setCurrentFile(fileName))
  {
    throw (IO_Zip_Exception());
  }

  return readCurrentFileFromZip(zip);
}

//-----------------------------------------------------------------------------
QByteArray ZipUtils::readCurrentFileFromZip(QuaZip& zip)
{
  QuaZipFile zFile(&zip);
  if (!zFile.open(QIODevice::ReadOnly))
  {
    throw (IO_Zip_Exception());
  }

  return zFile.readAll();
}