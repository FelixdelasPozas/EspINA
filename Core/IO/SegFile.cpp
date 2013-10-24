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
#include "ClassificationXML.h"
#include <Core/Analysis/Storage.h>
#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Extensions/ExtensionProvider.h>
#include <quazip.h>
#include <quazipfile.h>

using namespace EspINA;
using namespace EspINA::IO;

const QString V4_TRACE_FILE    = "trace.dot";
const QString V4_TAXONOMY_FILE = "taxonomy.xml";
const QString V4_FILE_VERSION  = "version"; //backward compatibility

const QString PIPELINE_FILE       = "pipeline.dot";
const QString RELATIONS_FILE      = "relations.dot";
const QString CLASSIFICATION_FILE = "classification.xml";
const QString SETTINGS_FILE       = "settings.ini";


const QString SEG_FILE_VERSION = "5";
//const QString SEG_FILE_COMPATIBLE_VERSION = "1";


STATUS SegFile::load(const QFileInfo& file, AnalysisPtr analysis, ErrorHandlerPtr handler)
{

//   QDir tmpDir = QDir::tempPath();
//   tmpDir.mkpath("espina");
//   tmpDir.cd("espina");
// 
//   Persistent::StorageSPtr storage{new Persistent::Storage(tmpDir)};
}

//-----------------------------------------------------------------------------
class TmpSegFile 
{
public:
  TmpSegFile(QDir& tmpDir) 
  : m_tmpDir{tmpDir}
  , File{tmpDir.absoluteFilePath(QUuid::createUuid().toString())} 
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
bool addFileToZip(QString fileName,
                  const QByteArray &content,
                  QuaZipFile& zFile)
{
  QuaZipNewInfo zFileInfo = QuaZipNewInfo(fileName, fileName);
  zFileInfo.externalAttr = 0x01A40000; // Permissions of the files 644
  if (!zFile.open(QIODevice::WriteOnly, zFileInfo))
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not open " << fileName << "inside" << zFile.getFileName()
        << ". Code error:" << zFile.getZipError();
    return false;
  }
  zFile.write(content);

  if (zFile.getZipError() != UNZ_OK)
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not store the content in" << fileName << "inside"
        << zFile.getFileName() << ". Code error:" << zFile.getZipError();
    return false;
  }
  zFile.close();

  if (zFile.getZipError() != UNZ_OK)
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not close the file" << fileName << "inside" << zFile.getFileName()
        << ". Code error:" << zFile.getZipError();
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
STATUS SegFile::save(AnalysisPtr analysis, const QFileInfo& file, ErrorHandlerPtr handler)
{
  if (file.baseName().isEmpty())
  {
    if (handler)
      handler->error(QObject::tr("Invalid empty filename."));
    return STATUS::IO_ERROR;
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

    return STATUS::IO_ERROR;
  }
  QuaZipFile outFile(&zip);

  // Store Version Number
  //zipFile(SETTINGS, settings(model), outFile);

  QString classification;
  if (ClassificationXML::dump(analysis->classification(), classification) != STATUS::SUCCESS)
  {
    if (handler)
      handler->error("Error while saving Analysis Classification");

    return STATUS::IO_ERROR;
  }

  if (!addFileToZip(CLASSIFICATION_FILE, classification.toUtf8(), outFile))
  {
    if (handler)
      handler->error("Error while saving Analysis Classification");

    return STATUS::IO_ERROR;
  }

  std::ostringstream pipeline;
  write(analysis->content(), pipeline);
  if (!addFileToZip(PIPELINE_FILE, pipeline.str().c_str(), outFile))
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline");

    return STATUS::IO_ERROR;
  }

  std::ostringstream relations;
  write(analysis->relationships(), relations);
  if (!addFileToZip(PIPELINE_FILE, relations.str().c_str(), outFile))
  {
    if (handler)
      handler->error("Error while saving Analysis Pipeline");

    return STATUS::IO_ERROR;
  }

  foreach(DirectedGraph::Vertex v, analysis->content()->vertices()) {
    PersistentPtr item = dynamic_cast<PersistentPtr>(v.item.get());
    foreach(SnapshotData data, item->saveSnapshot())
    {
      if( !addFileToZip(data.first, data.second, outFile) )
      {
        return STATUS::IO_ERROR;
      }
    }
  }

  foreach(ExtensionProviderSPtr provider, analysis->extensionProviders()) {
    foreach(SnapshotData data, provider->saveSnapshot())
    {
      if( !addFileToZip(data.first, data.second, outFile) )
      {
        return STATUS::IO_ERROR;
      }
    }
  }

  zip.close();

  if (zip.getZipError() != UNZ_OK)
  {
    if (handler)
      handler->error("Unable to create " + file.fileName());

    return STATUS::IO_ERROR;
  }

  if (file.exists())
  {
    file.absoluteDir().remove(file.fileName());
  }

  if (!tmpFile.File.copy(file.absoluteFilePath()))
  {
    if (handler)
      handler->error("Couldn't save file on " + file.absoluteFilePath());

    return STATUS::IO_ERROR;
  }

  return STATUS::SUCCESS;
}
