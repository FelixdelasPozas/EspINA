/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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
#include "SegFile.h"
#include <EspinaConfig.h>
#include <Core/IO/SegFile_V5.h>
#include <Core/IO/SegFile_V4.h>
#include <Core/Utils/TemporalStorage.h>
#include <Core/Analysis/Analysis.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Utils/EspinaException.h>

// QuaZip
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;

const QString SEG_FILE_VERSION = "5";

using SegFileLoaderSPtr = std::shared_ptr<SegFileInterface>;

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile::load(const QFileInfo  &file,
                           CoreFactorySPtr   factory,
                           ProgressReporter *reporter,
                           ErrorHandlerSPtr  handler,
                           LoadOptions       options)
{
  QuaZip zip(file.filePath());
  if (!zip.open(QuaZip::mdUnzip))
  {
    if (handler)
    {
      handler->error("IOEspinaFile: Could not open file" + file.filePath());
    }

    auto what    = QObject::tr("Can't open ZIP container, file: %1, error code: %2").arg(file.filePath()).arg(zip.getZipError());
    auto details = QObject::tr("SegFile::load() -> Can't open ZIP container, file: %1, error code: %2").arg(file.filePath()).arg(zip.getZipError());

    throw EspinaException(what, details);
  }

  SegFileLoaderSPtr loader;
  if (!zip.setCurrentFile(SegFile_V5::FORMAT_INFO_FILE))
  {
    if (!zip.setCurrentFile(SegFile_V4::FORMAT_INFO_FILE))
    {
      auto what    = QObject::tr("Unknown SEG file, can't find format info: %1, error code: %2").arg(file.filePath()).arg(zip.getZipError());
      auto details = QObject::tr("SegFile::load() -> Unknown SEG file, can't find format info: %1, error code: %2").arg(file.filePath()).arg(zip.getZipError());

      throw EspinaException(what, details);
    }
    // NOTE: it may be necessary to select another reader depending on the file content
    loader = std::make_shared<SegFile_V4>();
  }
  else
  {
    // NOTE: it may be necessary to select another reader depending on the file content
    loader = std::make_shared<SegFile_V5>();
  }

  auto coreFactory = factory;
  if (!coreFactory)
  {
    coreFactory = std::make_shared<CoreFactory>();
  }

  return loader->load(zip, coreFactory, reporter, handler, options);
}

//-----------------------------------------------------------------------------
class TmpSegFile
{
public:
  explicit TmpSegFile(QDir& tmpDir) :
  File { tmpDir.absoluteFilePath(QUuid::createUuid().toString()) }, m_tmpDir { tmpDir }
  {}

  ~TmpSegFile()
  {
    if (m_tmpDir.exists(File.fileName()))
    {
      m_tmpDir.remove(File.fileName());
    }
  }

  QFile File;

private:
  QDir m_tmpDir;
};

//-----------------------------------------------------------------------------
void SegFile::save(AnalysisPtr analysis,
                   const QFileInfo& file,
                   ProgressReporter *reporter,
                   ErrorHandlerSPtr handler)
{
  if (file.baseName().isEmpty())
  {
    if (handler)
    {
      handler->error(QObject::tr("Invalid empty filename."));
    }

    auto what    = QObject::tr("Attempting to save without filename");
    auto details = QObject::tr("SegFile::save() -> Attempting to save without filename");

    throw EspinaException(what, details);
  }

  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  TmpSegFile tmpFile(tmpDir);
  QuaZip zip(&(tmpFile.File));

  if (!zip.open(QuaZip::mdCreate))
  {
    auto what    = QObject::tr("Can't create file inside ZIP container, file: %1, error code: %2").arg(tmpFile.File.fileName()).arg(zip.getZipError());
    auto details = QObject::tr("SegFile::save() -> Can't create file inside ZIP container, file: %1, error code: %2").arg(tmpFile.File.fileName()).arg(zip.getZipError());

    throw EspinaException(what, details);
  }

  SegFile_V5 segFile;
 
  segFile.save(analysis, zip, reporter, handler);

  zip.close();

  if (zip.getZipError() != UNZ_OK)
  {
    auto what    = QObject::tr("Can't close file inside ZIP container, file: %1, error code: %2").arg(tmpFile.File.fileName()).arg(zip.getZipError());
    auto details = QObject::tr("SegFile::save() -> Can't close file inside ZIP container, file: %1, error code: %2").arg(tmpFile.File.fileName()).arg(zip.getZipError());

    throw EspinaException(what, details);
  }

  if (file.exists())
  {
    file.absoluteDir().remove(file.fileName());
  }

  if (!tmpFile.File.copy(file.absoluteFilePath()))
  {
    auto what    = QObject::tr("Can't copy file to path, file: %1, path: %2").arg(tmpFile.File.fileName()).arg(file.absoluteFilePath());
    auto details = QObject::tr("SegFile::save() -> Can't copy file to path, file: %1, path: %2").arg(tmpFile.File.fileName()).arg(file.absoluteFilePath());

    throw EspinaException(what, details);
  }
}
