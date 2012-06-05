#include <QProcess>
#include <QDir>
#include <QDebug>


//-----------------------------------------------------------------------------
bool fileDiff(QString filePath1, QString filePath2)
{
  QProcess diffProcess;
  diffProcess.start("diff", QStringList() << filePath1 << filePath2);
  if( diffProcess.waitForFinished() )
  {
    QByteArray output = diffProcess.readAllStandardOutput();
    if( output.size() != 0 )
    {
      qDebug() << "diff" << filePath1 << filePath2 << "\n" << output;
      return true;
    }
  } else
    return true;
  return false;
}

//-----------------------------------------------------------------------------
void recursiveRemoveDir(char* dir)
{
  system(QString("rm -rf ").append(dir).toUtf8());
}


//-----------------------------------------------------------------------------
QStringList recursiveFilePaths(QDir searchPath)
{
  QStringList paths;
  foreach(QFileInfo filePath, searchPath.entryInfoList(QDir::AllEntries))
  {
    QString path = searchPath.filePath(filePath.fileName());
    if( filePath.isDir() ) {
      if( !(filePath.fileName() == "." or filePath.fileName() == "..") ) {
        paths.append(recursiveFilePaths( path ));
      }
    } else {
      paths.append( path );
    }
  }
  return paths;
}
