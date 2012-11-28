#include "FilePack.h"
#include <QProcess>
#include <QDebug>

//-----------------------------------------------------------------------------
int loadWithOutRequiredFile(const char* fileName)
{
  QDir fileDir(fileName);
  
  QString taxContent, traceContent;
  QTextStream tax, trace;
  tax.setString(&taxContent);
  trace.setString(&traceContent);
  if( IOEspinaFile::loadFile(fileDir.filePath(fileName), tax, trace) )
    return 1;

  return 0;
} 

//-----------------------------------------------------------------------------
bool differentFiles(QString filePath1, QString filePath2)
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


//-----------------------------------------------------------------------------
int saveSimpleFile(QString referenceSegFile, QString destination)
{
  QFile segFile(referenceSegFile);
  if( !segFile.exists() )
  {
    qDebug() <<"Error: " << segFile.fileName() << " does not exist";
    return 1;
  }
  system(QString("unzip -u %1 -d test").arg(segFile.fileName()).toUtf8());
  
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
  if( !IOEspinaFile::saveFile(destination, trace, tax,
                              paths, QString("test/")) )
  {
    qWarning("Error while saving file");
    return 1;
  }
  
  fTax.close();
  fTrace.close();
  recursiveRemoveDir("test");
//   QFile::remove(destination);

  return 0;
}

//-----------------------------------------------------------------------------
int loadSimpleFile( QDir fileDir, QString referenceFileName )
{
  int result = 0;
  //QDir fileDir(argv[1]);
  
  QString taxContent, traceContent;
  QTextStream tax, trace;
  tax.setString(&taxContent);
  trace.setString(&traceContent);
  
  //QString referenceFileName = "seg3.seg";
  if( !IOEspinaFile::loadFile(fileDir.filePath(referenceFileName), trace, tax) )
  {
    qWarning("An error ocurre while openning seg file");
    return 1;
  }
  // compare trace.dot and taxonomy.xml ... TODO and pvd files
  recursiveRemoveDir("test");
  system(QString("unzip -u %1 -d test").arg(fileDir.filePath(referenceFileName)).toUtf8());
  
  // trace
  QFile traceFile("trace.dot");
  if( !traceFile.open(QIODevice::WriteOnly) )
  {
    qWarning("trace.dot file could not be opened");
    return 1;
  }
  traceFile.write(traceContent.toUtf8());
  traceFile.close();
  
  if( differentFiles("trace.dot", "test/trace.dot") )
  {
    qWarning("trace files are different");
    return 1;
  }
  QFile::remove("trace.dot");
  
  // taxonomy
  QFile taxFile("taxonomy.xml");
  if( !taxFile.open(QIODevice::WriteOnly) )
  {
    qWarning("taxonomy.xml file could not be opened");
    return 1;
  }
  taxFile.write(taxContent.toUtf8());
  taxFile.close();
  if( differentFiles("taxonomy.xml", "test/taxonomy.xml") )
  {
    qWarning("taxonomy files are different");
    return 1;
  }
  QFile::remove("taxonomy.xml");
  
  // Compare all the files to test if the uncompress was correctly
  QStringList testFiles = recursiveFilePaths(QDir("./test"));
  testFiles.removeAll("./test/trace.dot");
  testFiles.removeAll("./test/taxonomy.xml");
  QStringList originalFiles = 
    recursiveFilePaths(QDir(fileDir.filePath(QString(referenceFileName).remove(QRegExp("\\..*$")))));
  
  qDebug() << testFiles << "\n" << originalFiles;
  
  if( testFiles.size() != originalFiles.size()) // less taxonomy.xml and trace.xml
  {
    qWarning("Seg files does not contain the same number of files");
    return 1;
  }
  for(int i=0; i < testFiles.size(); i++)
  {
    if(differentFiles(testFiles.at(i), originalFiles.at(i)) )
    {
      qWarning() << testFiles.at(i) << "and" << originalFiles.at(i) << "are different";
      return 1;
    }
  }
  recursiveRemoveDir("test");
  return 0;
}
