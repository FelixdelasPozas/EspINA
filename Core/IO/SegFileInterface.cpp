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
#include "SegFileInterface.h"
#include "SegFile.h"
#include <Core/Utils/EspinaException.h>
#include "ZipUtils.h"

// QuaZip
#include <quazip/quazipfile.h>

// Qt
#include <QUuid>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;

//-----------------------------------------------------------------------------
void SegFileInterface::addFileToZip(const QString    &fileName,
                                    const QByteArray &content,
                                    QuaZip           &zip,
                                    ErrorHandlerSPtr  handler)
{
  try
  {
    ZipUtils::AddFileToZip(fileName, content, zip);
  }
  catch(EspinaException &e)
  {
    if (handler)
    {
      handler->error(QObject::tr("Could not save %1 into seg file").arg(fileName));
    }

    throw(e);
  }
}

//-----------------------------------------------------------------------------
QByteArray SegFileInterface::readFileFromZip(const QString&   fileName,
                                             QuaZip&          zip,
                                             ErrorHandlerSPtr handler)
{
  QByteArray contents;

  try
  {
    contents = ZipUtils::readFileFromZip(fileName, zip);
  }
  catch(EspinaException &e)
  {
    if (handler)
    {
      handler->error(QObject::tr("Could not find %1 in seg file").arg(fileName));
    }

    throw(e);
  }

  return contents;
}

//-----------------------------------------------------------------------------
QByteArray SegFileInterface::readCurrentFileFromZip(QuaZip& zip,
                                                    ErrorHandlerSPtr handler)
{
  QByteArray contents;

  try
  {
    contents = ZipUtils::readCurrentFileFromZip(zip);
  }
  catch(EspinaException &e)
  {
    if (handler)
    {
      handler->error(QObject::tr("Couldn't extract %1 from seg file").arg(zip.getCurrentFileName()));
    }

    throw(e);
  }

  return contents;
}
