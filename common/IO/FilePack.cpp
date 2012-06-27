#include "FilePack.h"

#include <quazipfile.h>

#include <QDebug>
#include <QFile>
#include <QDir>
#include "espina_debug.h"
#include <model/Segmentation.h>

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
bool IOEspinaFile::loadFile(QString filePath,
                            QTextStream& TraceContent,
                            QTextStream& TaxonomyContent)
{

  // directory of the cached disk files
  QDir dir(QString(filePath).remove(QRegExp("\\..*$")));
  if( !dir.exists() )
    dir.mkpath(dir.absolutePath());
  
  QuaZip zip(filePath);
  if( !zip.open(QuaZip::mdUnzip) )
  {
    qWarning() << "IOEspinaFile: Could not open file" << filePath;
    return false;
  }
  bool taxPorcessed = false, traceProcess = false;
  QuaZipFile file(&zip);
  for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
    QString actualFileName = file.getActualFileName();
    if( !file.open(QIODevice::ReadOnly) )
    {
      qWarning() << "IOEspinaFile: Could not extract the file" << actualFileName;
      if( actualFileName == TAXONOMY || actualFileName == TRACE )
        return false;
      continue;
    }
//     qDebug() << "IOEspinaFile::loadFile: extracting" << actualFileName; //TODO espina_debug
    if( actualFileName == TAXONOMY ) {
      if( !taxPorcessed )
        TaxonomyContent << file.readAll();
      taxPorcessed = true;
    } else if( actualFileName == TRACE ) {
      if( !traceProcess )
        TraceContent << file.readAll();
      traceProcess = true;
    } else { // Cache Disk files
      // Is a directory
      QFileInfo fileInfo(actualFileName);
      //qDebug() << actualFileName << "has path" << fileInfo.path();
      if( fileInfo.path() != "." )
      {
        dir.mkpath(fileInfo.path());
        /*
        QFile::setPermissions(dir.filePath(fileInfo.path()),
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
            QFile::ReadGroup | QFile::ExeGroup |
            QFile::ReadOther | QFile::ExeOther);
            */
      }
      
      QFile destination(dir.filePath(actualFileName));
      /*qDebug()<< "Permissions set" << 
      destination.setPermissions(QFile::ReadOwner | QFile::WriteOwner | 
                                 QFile::ReadGroup | QFile::ReadOther);*/
      if(!destination.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
        qWarning() << "IOEspinaFile::loadFile: could not create file " << actualFileName;
      }
      destination.write(file.readAll());
      destination.close();
    }
    file.close();
  }
  if( !taxPorcessed || !traceProcess )
  {
    qWarning() << "IOEspinaFile::loadFile: could not find taxonomy and/or trace files";
    return false;
  }
  zip.close();
  return true;
}

//-----------------------------------------------------------------------------
bool IOEspinaFile::loadFile(QFileInfo file,
                            QSharedPointer<EspinaModel> model)
{
  // Create tmp dir
  qDebug() << file.absolutePath();
  QDir tmpDir = file.absoluteDir();
  tmpDir.mkdir(file.baseName());
  tmpDir.cd(file.baseName());
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
bool IOEspinaFile::saveFile(QString& filePath,
                            QString& TraceContent,
                            QString& TaxonomyContent,
                            QStringList& segmentationPaths,
                            QString commonPathToRemove )
{
  QFile zFile(filePath);
  QuaZip zip(&zFile);
  if(!zip.open(QuaZip::mdCreate)) 
  {
    qWarning() << "IOEspinaFile::saveFile" << filePath << "error while creating file";
    return false;
  }
  QFileInfoList files=QDir().entryInfoList(segmentationPaths);
  QuaZipFile outFile(&zip);
  // zip taxonomy and trace files
//   QByteArray(TraceContent.toStdString().c_str());
  if( !IOEspinaFile::zipFile(QString(TRACE),  TraceContent.toStdString().c_str(), outFile) )
    return false;
  if( !IOEspinaFile::zipFile(QString(TAXONOMY), TaxonomyContent.toStdString().c_str(), outFile) )
    return false;
  // zip the segmentation files
  QByteArray buffer;
  foreach(QString pathName, segmentationPaths) 
  {
    QFile inFile(pathName);
    qDebug() << "SaveFile: Processing" << pathName;
    qDebug() << "SaveFile: Opening files"
      << inFile.open(QIODevice::ReadOnly);
    IOEspinaFile::zipFile(inFile.fileName().remove(commonPathToRemove), inFile.readAll(), outFile);
    inFile.close();
  }
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
    QString volumeFile = tmpDir.absoluteFilePath(volumeName + ".mhd");
    io->SetFileName(volumeFile.toStdString());
    writer->SetFileName(volumeFile.toStdString());
    writer->SetInput(seg->volume());
    writer->SetImageIO(io);
    writer->Update();
    QFile segFile(volumeFile);
    segFile.open(QIODevice::ReadOnly);
    if( !IOEspinaFile::zipFile(volumeName + ".mhd", segFile.readAll() , outFile) )
    {
      qWarning() << "IOEspinaFile::saveFile: Error while zipping" << (volumeName + ".mhd");
      return false;
    }
    if( !IOEspinaFile::zipFile(volumeName + ".raw", segFile.readAll() , outFile) )
    {
      qWarning() << "IOEspinaFile::saveFile: Error while zipping" << (volumeName + ".raw");
      return false;
    }
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
