#include <FilePack.h>
#include "helpers.h"
#include <QDebug>


int saveSimpleFile(int argc, char** argv)
{
  QDir fileDir(argv[1]);
  QString fileName("seg3.seg");
  if( argc >= 3 )
    fileName = argv[2];
  return saveSimpleFile(fileDir.filePath(fileName));
}