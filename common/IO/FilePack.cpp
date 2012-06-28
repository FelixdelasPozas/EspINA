#include "FilePack.h"

#include <quazipfile.h>

#include <QDebug>
#include <QFile>
#include <QDir>
#include "espina_debug.h"
#include <model/Segmentation.h>
#include <EspinaCore.h>

#include <itkImageFileWriter.h>
#include <itkMetaImageIO.h>
#include <vtkMetaImageReader.h>
#include <vtkTIFFReader.h>

const QString TRACE    = "trace.dot";
const QString TAXONOMY = "taxonomy.xml";

typedef itk::ImageFileWriter<EspinaVolume> EspinaVolumeWriter;

//-----------------------------------------------------------------------------
/**
 * It set the @param TraceContent and @param TaxonomyContent with trace.dot and 
 * taxonomy.xml files inside @param filePath file.
 * If there are extra files (for the cached disk). They are extracted in a 
 * directory with the same name as seg file (without the extension).
 */
//-----------------------------------------------------------------------------
bool IOEspinaFile::loadFile(QFileInfo file,
                            QSharedPointer<EspinaModel> model)
{
  // Create tmp dir
  qDebug() << file.absolutePath();
  QDir tmpDir = file.absoluteDir();
  tmpDir.mkdir(file.baseName());
  tmpDir.cd(file.baseName());
  EspinaCore::instance()->setTemporalDir(tmpDir);
  qDebug() << "Temporal Dir" << tmpDir;

  QuaZip espinaZip(file.filePath());
  if( !espinaZip.open(QuaZip::mdUnzip) )
  {
    qWarning() << "IOEspinaFile: Could not open file" << file.filePath();
    return false;
  }

  QuaZipFile espinaFile(&espinaZip);

  Taxonomy *taxonomy = NULL;
  QString traceContent;

  bool hasFile = espinaZip.goToFirstFile();
  while(hasFile)
  {
    QFileInfo file = espinaFile.getActualFileName();
    if( !espinaFile.open(QIODevice::ReadOnly) )
    {
      qWarning() << "IOEspinaFile: Could not extract the file" << file.filePath();
      if( file == TAXONOMY || file == TRACE )
        return false;
      continue;
    }

    qDebug() << "IOEspinaFile::loadFile: extracting" << file.filePath();
    if(file.fileName() == TAXONOMY )
    {
      Q_ASSERT(taxonomy == NULL);
      taxonomy = IOTaxonomy::loadXMLTaxonomy(espinaFile.readAll());
      taxonomy->print(3);
    } else if(file.fileName() == TRACE)
    {
      Q_ASSERT(traceContent.isEmpty());
      QTextStream traceStream(&traceContent);
      traceStream << espinaFile.readAll();
    } else
    {
      // Espina Volumes
      QFile destination(tmpDir.filePath(file.fileName()));
      /*qDebug()<< "Permissions set" <<
       *       destination.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
       *                                  QFile::ReadGroup | QFile::ReadOther);*/
      if(!destination.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        qWarning() << "IOEspinaFile::loadFile: could not create file " << file.filePath();
      }
      destination.write(espinaFile.readAll());
      destination.close();
    }
    espinaFile.close();
    hasFile = espinaZip.goToNextFile();
  }

  if(!taxonomy || traceContent.isEmpty())
  {
    qWarning() << "IOEspinaFile::loadFile: could not find taxonomy and/or trace files";
    return false;
  }

  model->addTaxonomy(taxonomy);
  std::istringstream trace(traceContent.toStdString().c_str());
  model->loadSerialization(trace);

  espinaZip.close();
  return true;
}


//-----------------------------------------------------------------------------
bool IOEspinaFile::saveFile(QFileInfo file,
                            QSharedPointer<EspinaModel> model)
{
  // Create tmp dir
  qDebug() << file.absolutePath();
  QDir tmpDir = file.absoluteDir();
  tmpDir.mkdir(file.baseName());
  tmpDir.cd(file.baseName());
  qDebug() << "Temporal Dir" << tmpDir;

  QFile zFile(file.filePath());
  QuaZip zip(&zFile);
  if(!zip.open(QuaZip::mdCreate)) 
  {
    qWarning() << "IOEspinaFile::saveFile" << file.fileName() << "error while creating file";
    return false;
  }
  QuaZipFile outFile(&zip);
  //TODO: File header
  // outFile.write("EspinaSegmenation-1.0\0",21);

  // Set Taxonomy
  QString taxonomy;
  IOTaxonomy::writeXMLTaxonomy(model->taxonomy(), taxonomy);
  if( !IOEspinaFile::zipFile(QString(TAXONOMY), taxonomy.toAscii(), outFile) )
    return false;

  // Set Trace
  std::ostringstream trace;
  model->serializeRelations(trace);
  if( !IOEspinaFile::zipFile(QString(TRACE),  trace.str().c_str(), outFile) )
    return false;

  foreach(Segmentation *seg, model->segmentations())
  {
    qDebug() << "Making" << seg->data().toString() << "snapshot";
    itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
    EspinaVolumeWriter::Pointer writer = EspinaVolumeWriter::New();
    Filter *creator = seg->filter();
    int output = seg->outputNumber();
    QString volumeName = creator->id() + "_" + QString::number(output);
    QString mhd = tmpDir.absoluteFilePath(volumeName + ".mhd");
    QString raw = tmpDir.absoluteFilePath(volumeName + ".raw");
    io->SetFileName(mhd.toStdString());
    writer->SetFileName(mhd.toStdString());
    writer->SetInput(seg->volume());
    writer->SetImageIO(io);
    writer->Write();
    QFile mhdFile(mhd);
    mhdFile.open(QIODevice::ReadOnly);
    QFile rawFile(raw);
    rawFile.open(QIODevice::ReadOnly);
    if( !IOEspinaFile::zipFile(volumeName + ".mhd", mhdFile.readAll() , outFile) )
    {
      qWarning() << "IOEspinaFile::saveFile: Error while zipping" << (volumeName + ".mhd");
      return false;
    }
    if( !IOEspinaFile::zipFile(volumeName + ".raw", rawFile.readAll() , outFile) )
    {
      qWarning() << "IOEspinaFile::saveFile: Error while zipping" << (volumeName + ".raw");
      return false;
    }
    mhdFile.close();
    rawFile.close();
  }

  QStringList filters;
  filters << "*.mhd" << "*.raw";
  foreach(QFileInfo tmpFile, tmpDir.entryInfoList(filters))
  {
    qDebug() << "Removing" << tmpFile.fileName();
    Q_ASSERT(tmpDir.remove(tmpFile.fileName()));
  }
  tmpDir.cdUp();
  tmpDir.rmdir(file.baseName());

  return true;
}

//-----------------------------------------------------------------------------
bool IOEspinaFile::zipFile(QString fileName, QByteArray content, QuaZipFile& zFile)
{
  QuaZipNewInfo zFileInfo = QuaZipNewInfo(fileName, fileName);
  zFileInfo.externalAttr = 0x01A40000; // Permissions of the files 644
  if( !zFile.open(QIODevice::WriteOnly, zFileInfo) )
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not open " << fileName 
              << "inside" << zFile.getFileName() 
              << ". Code error:" << zFile.getZipError();
    return false;
  }
  zFile.write(content);
  if(zFile.getZipError()!=UNZ_OK) 
  {
    qWarning() << "IOEspinaFile::zipFile(): Could not store the content in" << fileName
            << "inside" << zFile.getFileName() 
            << ". Code error:" << zFile.getZipError();
    return false;
  }
  zFile.close();
  if(zFile.getZipError()!=UNZ_OK) {
    qWarning() << "IOEspinaFile::zipFile(): Could not close the file" << fileName
           << "inside" << zFile.getFileName() 
           << ". Code error:" << zFile.getZipError();
    return false;
  }
  return true;
}
