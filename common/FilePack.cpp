#include "FilePack.h"
#include <QDebug>
#include <QFile>

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
void FilePack::readFile(FilePack::fileNames name, QTextStream& data)
{
  zip_file* zFile =
    zip_fopen(m_file, getRealName(name).toStdString().c_str(), 0);
  if (zFile)
  {
    //int buffSize = 512;
    char buffer[2];//[buffSize];
    buffer[1]='\0';
    while( zip_fread(zFile, buffer, 1) > 0  )
      data << buffer[0];
  }
}

//-----------------------------------------------------------------------------
int FilePack::addSource(FilePack::fileNames name, QString& data)
{
  // zip_source_buffer corrupts the files ...
  QString stdFileName = getRealName(name);
  QString tmpFileName = "."+stdFileName;
  QFile f(tmpFileName);
  f.open(QIODevice::WriteOnly | QIODevice::Truncate);
  f.write(data.toUtf8());
  f.close();

  m_TmpFilesToRemove.append( tmpFileName );
  
  qDebug() << "FilePack: addSource: " << data;
  int index = -2;
  struct zip_source* s_buffer =
    zip_source_file(m_file, tmpFileName.toUtf8(), 0, 0);
    // zip_source_buffer corrupts the files ...
    //zip_source_buffer(m_file, data.toStdString().c_str(), data.size(), 0);
  if( s_buffer )
    index = zip_add(m_file, stdFileName.toUtf8(), s_buffer);
  if (index < 0)
    qDebug() << "FilePacker: Error while adding source:\n" << zip_strerror(m_file);

  return index;
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
QString FilePack::getRealName(FilePack::fileNames name)
{
  switch(name)
  {
    case TRACE:
      return "trace.dot";
    case TAXONOMY:
      return "taxonomy.xml";
    default:
      qDebug() << "FilePack: Error in " << __FILE__ << ":"<< __LINE__ << ". Unknown name";
      return "";
  }
}


//-----------------------------------------------------------------------------
void IOEspinaFile::loadFile(QString filePath,
                            QTextStream& TraceContent,
                            QTextStream& TaxonomyContent)
{
  FilePack zipFile( filePath, FilePack::READ );
  // Read Taxonomy
  zipFile.readFile(FilePack::TAXONOMY, TaxonomyContent);
  qDebug() << "Tax: " << *TaxonomyContent.string();


  // Read Trace
  zipFile.readFile(FilePack::TRACE, TraceContent);
  qDebug() << "Trace: " << *TraceContent.string();

  zipFile.close();

}

//-----------------------------------------------------------------------------
void IOEspinaFile::saveFile(QString& filePath,
                            QString& TraceContent,
                            QString& TaxonomyContent)
{
  FilePack pack( filePath, FilePack::WRITE );
  pack.addSource(FilePack::TRACE, TraceContent);
  pack.addSource(FilePack::TAXONOMY, TaxonomyContent);
  pack.close();
}
