#include <stdlib.h>
#include <FilePack.h>
#include <zip.h>

//QT
#include <QFile>
#include <QDebug>
#include <QDir>



void recursiveRemoveDir(char* dir)
{
  system(QString("rm -rf ").append(dir).toUtf8());
}

/**
 * Retrieve all the file paths inside a specific directory and recursively iterates
 * through the inside directories
 */
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

bool testSimpleSaveFile(QString fileName)
{
  QFile segFile(fileName);
  if( !segFile.exists() )
  {
    qDebug() <<"Error: " << segFile.fileName() << " does not exist";
    return false;
  }
  system(QString("unzip %1 -d test").arg(segFile.fileName()).toUtf8());
  
  QString filePath("save.seg");
  QFile::remove(filePath);
  QStringList segPaths;
  QString commonPath = QDir().currentPath();
  
  QFile fTrace("test/trace.dot");
  if( !fTrace.open(QIODevice::ReadOnly) )
  {
    qDebug("Error: trace.dot file could not be found");
    return false;
  }
  QTextStream traceContent(&fTrace);
  
  QFile fTax("test/taxonomy.xml");
  if( !fTax.open(QIODevice::ReadOnly) )
  {
    qDebug("Error: taxonomy.xml file could not be found");
    return false;
  }
  QTextStream taxContent(&fTax);

  QStringList paths = recursiveFilePaths(QDir("test"));
  paths.removeAll("trace.dot");
  paths.removeAll("taxonomy.xml");
  qDebug() << paths;
  QString trace(traceContent.readAll());
  QString tax(taxContent.readAll());
  bool res = IOEspinaFile::saveFile(filePath, trace, tax,
                                paths, QString("test/"));
  
  fTax.close();
  fTrace.close();
  recursiveRemoveDir("test");
  return res;
}



int main(int argc, char** argv)
{
  qDebug() << testSimpleSaveFile("test.seg");
  //qDebug() << system("zipcmp test.seg save.seg");
  return 0;
}