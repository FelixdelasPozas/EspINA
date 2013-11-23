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

#include "SegFile.h"

#include <EspinaConfig.h>

#include "Core/IO/SegFile_V5.h"
#include "Core/IO/SegFile_V4.h"
#include <Core/Analysis/Storage.h>
#include <Core/Analysis/Analysis.h>
#include <Core/Factory/CoreFactory.h>

#include <quazip.h>
#include <quazipfile.h>

using namespace EspINA;
using namespace EspINA::IO;
using namespace EspINA::IO::SegFile;


const QString SEG_FILE_VERSION = "5";
//const QString SEG_FILE_COMPATIBLE_VERSION = "1";

using SegFileLoaderSPtr = std::shared_ptr<SegFileInterface>;

//-----------------------------------------------------------------------------
AnalysisReader::ExtensionList SegFileReader::supportedFileExtensions() const
{
  ExtensionList supportedExtensions;

  Extensions extensions;
  extensions << "seg";

  supportedExtensions["EspINA Analysis"] = extensions;

  return supportedExtensions;
}


//-----------------------------------------------------------------------------
AnalysisSPtr SegFileReader::read(const QFileInfo file,
                                 CoreFactorySPtr factory,
                                 ErrorHandlerPtr handler)
{
  return load(file, factory, handler);
}

//-----------------------------------------------------------------------------
AnalysisSPtr SegFile::load(const QFileInfo& file,
                           CoreFactorySPtr  factory,
                           ErrorHandlerPtr  handler)
{
  QuaZip zip(file.filePath());
  if (!zip.open(QuaZip::mdUnzip))
  {
    if (handler)
      handler->error("IOEspinaFile: Could not open file" + file.filePath());

    throw (IO_Error_Exception());
  }

  SegFileLoaderSPtr loader;
  if (!zip.setCurrentFile(SegFile_V5::FORMAT_INFO_FILE))
  {
    if (!zip.setCurrentFile(SegFile_V4::FORMAT_INFO_FILE))
    {
      throw (IO_Error_Exception());
    }
    // NOTE: it may be necessary to select another reader depending on the file content
    loader = SegFileLoaderSPtr{new SegFile_V4()};
  } else 
  {
    // NOTE: it may be necessary to select another reader depending on the file content
    loader = SegFileLoaderSPtr{new SegFile_V5()};
  }

  CoreFactorySPtr coreFactory = factory;
  if (coreFactory == nullptr)
  {
    coreFactory = CoreFactorySPtr(new CoreFactory());
  }

  return loader->load(zip, coreFactory, handler);
}

//-----------------------------------------------------------------------------
class TmpSegFile 
{
public:
  TmpSegFile(QDir& tmpDir) 
  : File{tmpDir.absoluteFilePath(QUuid::createUuid().toString())}
  , m_tmpDir{tmpDir}
  {}

  ~TmpSegFile() 
  {
    if (m_tmpDir.exists(File.fileName())) m_tmpDir.remove(File.fileName());
  }

  QFile File;

private:
  QDir m_tmpDir;
};


//-----------------------------------------------------------------------------
void SegFile::save(AnalysisPtr analysis, const QFileInfo& file, ErrorHandlerPtr handler)
{
  if (file.baseName().isEmpty())
  {
    if (handler)
      handler->error(QObject::tr("Invalid empty filename."));
    throw (IO_Error_Exception());
  }

  QDir tmpDir = QDir::tempPath();
  tmpDir.mkpath("espina");
  tmpDir.cd("espina");

  TmpSegFile tmpFile(tmpDir);

  QuaZip zip(&(tmpFile.File));
  if (!zip.open(QuaZip::mdCreate))
  {
    if (handler)
      handler->error("Failed to create " + tmpFile.File.fileName() + " file");

    throw (IO_Error_Exception());
  }

  SegFile_V5 segFile;
  segFile.save(analysis, zip, handler);

  zip.close();

  if (zip.getZipError() != UNZ_OK)
  {
    if (handler)
      handler->error("Unable to create " + tmpFile.File.fileName());

    throw(IO_Error_Exception());
  }

  if (file.exists())
  {
    file.absoluteDir().remove(file.fileName());
  }

  if (!tmpFile.File.copy(file.absoluteFilePath()))
  {
    if (handler)
      handler->error("Couldn't save file on " + file.absoluteFilePath());

    throw(IO_Error_Exception());
  }

}
