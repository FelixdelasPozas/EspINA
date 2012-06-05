#include "FilePack.h"
#include <QDebug>
#include <QFile>
#include "espina_debug.h"

#define TRACE "trace.dot"
#define TAXONOMY "taxonomy.xml"

/*
//-----------------------------------------------------------------------------
FilePack::FilePack(QString FilePackName, FilePack::flags flag)
{
  int zipFlag = ZIP_CHECKCONS;
  if(WRITE == flag)
  {
    zipFlag = ZIP_CREATE;
    if(QFile::exists(FilePackName) )
    {
      qDebug() << "FilePack:Deleting " << FilePackName;
      QFile::remove(FilePackName);
    }
  }  
  m_file = zip_open(FilePackName.toStdString().c_str(), zipFlag, &m_error);
  if( !m_file)
    qDebug() << "FilePack: Error opennening file." << zip_strerror(m_file);
  //TODO comprobar que no existe el fichero. Borrarlo en caso contrario
}

//-----------------------------------------------------------------------------
FilePack::~FilePack()
{
  close();
}

//-----------------------------------------------------------------------------
bool FilePack::fileCreated()
{
  return m_file != NULL;
}

//-----------------------------------------------------------------------------
void FilePack::readFile(QString name, QTextStream& data)
{
  zip_file* zFile =
    zip_fopen(m_file, name.toStdString().c_str(), 0);
  if (zFile)
  {
    //int buffSize = 512;
    char buffer[2];//[buffSize];
    buffer[1]='\0';
    while( zip_fread(zFile, buffer, 1) > 0  )
      data << buffer[0];
  }
  zip_fclose(zFile);
}

//-----------------------------------------------------------------------------
int FilePack::addSource(QString fileName, QString& source)
{
  // zip_source_buffer corrupts the files, so the only way is to store the data
  // in a file of the file system and store it with addFile
  QFile f( fileName);
  f.open(QIODevice::WriteOnly | QIODevice::Truncate);
  f.write( source.toUtf8());
  f.close();

  m_TmpFilesToRemove.append( fileName );
  
  //qDebug() << "FilePack: addSource: " << source;
  return addFile( QFileInfo(fileName) );
}

//-----------------------------------------------------------------------------
bool FilePack::close()
{
  if(m_file)
  {
    bool ret = zip_close(m_file) == 0? true : false;
    if( ! ret)
      qDebug() << "FilePack: error while closing. " << zip_strerror(m_file);
    m_file = NULL;
    foreach(QString file, m_TmpFilesToRemove)
      QFile::remove(file);
    m_TmpFilesToRemove.clear();
    return ret;
  }  
  return false;
}

//-----------------------------------------------------------------------------
void FilePack::addDir ( QDir path )
{
  //qDebug() << "AddDir:" << path << path.dirName();
  if(int i = zip_add_dir(m_file, path.dirName().toUtf8()) != -1)
  {
    //foreach(
    //qDebug() << "ADDFILE:" << path.filePath(path.dirName());
    this->addFile(path.filePath(path.dirName().append("_0.vti")), path.dirName().append("/"+path.dirName().append("_0.vti")));
  }
  else
    qWarning() << "FilePacker: Error while adding dir\n" << zip_strerror(m_file);
}

//-----------------------------------------------------------------------------
int FilePack::addFile ( QFileInfo file, QString fileNameInPack )
{
  if( fileNameInPack == "" )
    fileNameInPack = file.fileName();
  int index = -2;
  struct zip_source* s_buffer =
    zip_source_file(m_file, file.filePath().toUtf8(), 0, 0);
    // zip_source_buffer corrupts the files ...
    //zip_source_buffer(m_file, data.toStdString().c_str(), data.size(), 0);
  if( s_buffer )
    index = zip_add(m_file, fileNameInPack.toUtf8(), s_buffer);
  if (index < 0)
    qDebug() << "FilePacker: Error while adding source:" << fileNameInPack << zip_strerror(m_file);

  return index;
}

//-----------------------------------------------------------------------------
void FilePack::ExtractFiles(QDir& filePath)
{
  int numFiles = zip_get_num_files(m_file);
  QTextStream stream;

  for(int i=0; i < numFiles; i++)
  {
    QString fileName(zip_get_name(m_file, i, 0));
    if( fileName != TAXONOMY && fileName != TRACE )
    {
      //TODO Review the method to diffs with files and dirs
      if( fileName.endsWith("/") )
        filePath.mkdir(fileName);
      else
      {
        QFile f(filePath.filePath(fileName)); //TODO change the path of the disk cache
        f.open(QFile::WriteOnly | QFile::Truncate);
        stream.setDevice(&f);
        qDebug() << "Unpacking" << fileName;
        this->readFile(fileName, stream);
        f.close();
      }
    }
  }
}

*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include <quazipfile.h>
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
bool IOEspinaFile::saveFile(QString& filePath,
                            QString& TraceContent,
                            QString& TaxonomyContent,
                            QStringList& segmentationPaths,
                            QString commonPathToRemove
                           )
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
