#include <FilePack.h>
#include "helpers.h"
#include <QDebug>


int saveExistingFile( int argc, char** argv)
{
  QDir fileDir(argv[1]);
  QFile segFile(fileDir.filePath("seg3.seg"));
  if( !segFile.exists() )
  {
    qDebug() <<"Error: " << segFile.fileName() << " does not exist";
    return 1;
  }
  system(QString("unzip -u %1 -d test").arg(segFile.fileName()).toUtf8());
  
  QString filePath("save.seg");
  
  QFile repeatedFile(filePath);
  if( !repeatedFile.open(QIODevice::WriteOnly) )
  {
    qWarning("Could not save the repeated file");
    return 1;
  }
  repeatedFile.write("Nothing important ....");
  repeatedFile.close();
  
  QStringList segPaths;
  QString commonPath = QDir().currentPath();
  // Opens trace file
  QFile fTrace("test/trace.dot");
  if( !fTrace.open(QIODevice::ReadOnly) )
  {
    qDebug("Error: trace.dot file could not be found");
    return 1;
  }
  QTextStream traceContent(&fTrace);
  // Opens taxonomy file
  QFile fTax("test/taxonomy.xml");
  if( !fTax.open(QIODevice::ReadOnly) )
  {
    qDebug("Error: taxonomy.xml file could not be found");
    return 1;
  }
  QTextStream taxContent(&fTax);

  QStringList paths = recursiveFilePaths(QDir("test"));
  paths.removeAll("trace.dot");
  paths.removeAll("taxonomy.xml");

  QString trace(traceContent.readAll());
  QString tax(taxContent.readAll());
  if( !IOEspinaFile::saveFile(filePath, trace, tax,
                              paths, QString("test/")) )
  {
    qWarning("Error while saving file");
    return 1;
  }
  
  fTax.close();
  fTrace.close();
  recursiveRemoveDir("test");
  QFile::remove(filePath);
 
  return 0;
}