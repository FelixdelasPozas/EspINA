#include "FilePack.h"
#include <QDebug>
#include <QFile>

//-----------------------------------------------------------------------------
FilePack::FilePack(QString FilePackName)
{
  if(QFile::exists(FilePackName))
  {
    qDebug() << "Deleting " << FilePackName;
    QFile::remove(FilePackName);
  }    
  m_file = zip_open(FilePackName.toStdString().c_str(), ZIP_CREATE, &m_error);
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
int FilePack::addTextSource(QString fileName, QString& data)
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
    index = zip_add(m_file, fileName.toLocal8Bit(), s_buffer);
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
