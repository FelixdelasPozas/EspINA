#include "helpers.h"
#include <QDebug>

int loadSimpleFile( int argc, char** argv )
{
  QDir fileDir(argv[1]);
  QString segFile = "seg3.seg";
  return loadSimpleFile(fileDir, segFile);
}
