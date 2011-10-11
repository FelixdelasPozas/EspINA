#include <FilePack.h>
#include "helpers.h"
#include <QDebug>


int saveAndLoad(int argc, char** argv)
{
  QDir fileDir(argv[1]);
  QString destination = "save.seg";
  if( saveSimpleFile(fileDir.filePath("seg3.seg"), destination) != 0 )
  {
    qWarning("saveSimpleFile does not work propertly");
    return 1;
  } else if( loadSimpleFile(QDir::current(), destination) != 0 ) {
    qWarning("loadSimpleFile does not work propertly");
    return 1;
  }
  return 0;
}