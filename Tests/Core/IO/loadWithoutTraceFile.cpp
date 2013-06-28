#include "helpers.h"
#include <QDir>

int loadWithoutTraceFile(int argc, char** argv)
{
  QDir fileDir(argv[1]);
  return loadWithOutRequiredFile(fileDir.filePath("withoutTraceFile.seg").toStdString().c_str());
}
