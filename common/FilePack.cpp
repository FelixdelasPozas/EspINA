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
  /*
  QFile f("/tmp/"+fileName);
  f.open(QIODevice::WriteOnly | QIODevice::Truncate);
  f.write(data.toUtf8());
  f.close();
  */
  int index = -2;
  struct zip_source* s_buffer =
    zip_source_buffer(m_file, data.toLocal8Bit(), data.size(), 0);
  if( s_buffer )
    index = zip_add(m_file, getRealName(name).toLocal8Bit(), s_buffer);
  else
    qDebug() << "FilePacker: Error while adding source:\n" << zip_strerror(m_file);
  return index;
}

//-----------------------------------------------------------------------------
bool FilePack::close()
{
  if(m_file)
  {
    bool ret = zip_close(m_file) == 0? true : false;
    m_file = NULL;
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

