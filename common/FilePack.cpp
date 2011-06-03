#include "FilePack.h"
#include <QDebug>
#include <QFile>

#define TRACE "trace.dot"
#define TAXONOMY "taxonomy.xml"

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
  
  qDebug() << "FilePack: addSource: " << source;
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
int FilePack::addFile ( QFileInfo file )
{
  int index = -2;
  struct zip_source* s_buffer =
    zip_source_file(m_file, file.filePath().toUtf8(), 0, 0);
    // zip_source_buffer corrupts the files ...
    //zip_source_buffer(m_file, data.toStdString().c_str(), data.size(), 0);
  if( s_buffer )
    index = zip_add(m_file, file.fileName().toUtf8(), s_buffer);
  if (index < 0)
    qDebug() << "FilePacker: Error while adding source:\n" << zip_strerror(m_file);

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
      QFile f(filePath.filePath(fileName)); //TODO change the path of the disk cache
      f.open(QFile::WriteOnly | QFile::Truncate);
      stream.setDevice(&f);
      qDebug() << "Unpacking" << fileName;
      this->readFile(fileName, stream);
      f.close();
    }
  }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void IOEspinaFile::loadFile(QString filePath,
                            QTextStream& TraceContent,
                            QTextStream& TaxonomyContent)
{
  FilePack zipFile( filePath, FilePack::READ );
  // Read Taxonomy
  zipFile.readFile(TAXONOMY, TaxonomyContent);
  // Read Trace
  zipFile.readFile(TRACE, TraceContent);
  QDir path(filePath.remove(QRegExp("\\..*$")));
  // If the directory does not exist, it must be created
  if( !path.exists() )
    path.mkpath(path.absolutePath());
  zipFile.ExtractFiles(path);
  zipFile.close();
}

//-----------------------------------------------------------------------------
void IOEspinaFile::saveFile(QString& filePath,
                            QString& TraceContent,
                            QString& TaxonomyContent,
                            QStringList& segmentationPaths)
{
  FilePack pack( filePath, FilePack::WRITE );
  pack.addSource(TRACE, TraceContent);
  pack.addSource(TAXONOMY, TaxonomyContent);
  foreach(QString fileName, segmentationPaths)
  {
    pack.addFile(QFileInfo(fileName));
  }
  pack.close();
}
