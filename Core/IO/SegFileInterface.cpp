/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "SegFileInterface.h"
#include "SegFile.h"

#include <quazipfile.h>
#include <QUuid>

using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;


//-----------------------------------------------------------------------------
void SegFileInterface::addFileToZip(const QString&    fileName,
                                    const QByteArray& content,
                                    QuaZip&           zip,
                                    ErrorHandlerPtr   handler)
{
  QuaZipFile zFile(&zip);
  QuaZipNewInfo zFileInfo = QuaZipNewInfo(fileName, fileName);

  zFileInfo.externalAttr = 0x01A40000; // Permissions of the files 644
  if (!zFile.open(QIODevice::WriteOnly, zFileInfo))
  {
    if (handler)
      handler->error(QObject::tr("Could not save %1 into seg file").arg(fileName));
      qWarning() << "SegFileInterface::addFileToZip(): Could not open" << fileName << "in zip file"
                 << "- Code error:" << zFile.getZipError();
    throw (SegFile::IO_Error_Exception());
  }
  zFile.write(content);
  if (zFile.getZipError() != UNZ_OK)
  {
    if (handler)
      handler->error(QObject::tr("Could not save %1 into seg file").arg(fileName));
      qWarning() << "SegFileInterface::addFileToZip(): Could not write" << fileName << "in zip file"
                 << "- Code error:" << zFile.getZipError();
    throw (SegFile::IO_Error_Exception());
  }

  zFile.close();

  if (zFile.getZipError() != UNZ_OK)
  {
    if (handler)
      handler->error(QObject::tr("Could not save %1 into seg file").arg(fileName));
      qWarning() << "SegFileInterface::addFileToZip(): Could not close" << fileName << "in zip file"
                 << "- Code error:" << zFile.getZipError();
    throw (SegFile::IO_Error_Exception());
  }
}

//-----------------------------------------------------------------------------
QByteArray SegFileInterface::readFileFromZip(const QString&  fileName,
                                             QuaZip&         zip,
                                             ErrorHandlerPtr handler)
{
  if (!zip.setCurrentFile(fileName))
  {
    if (handler)
      handler->error(QObject::tr("Could not find %1 in seg file").arg(fileName));

    throw (File_Not_Found_Exception());
  }

  return readCurrentFileFromZip(zip, handler);
}

//-----------------------------------------------------------------------------
QByteArray SegFileInterface::readCurrentFileFromZip(QuaZip& zip,
                                                    ErrorHandlerPtr handler)
{
  QuaZipFile zFile(&zip);
  if (!zFile.open(QIODevice::ReadOnly))
  {
    if (handler)
      handler->error(QObject::tr("Couldn't extract %1 from seg file").arg(zFile.getFileName()));

    throw (IO_Error_Exception());
  }
  return zFile.readAll();
}
