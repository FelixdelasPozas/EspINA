#include "helpers.h"
#include <QDir>
#include <QDebug>
int loadWithoutTaxFile(int argc, char** argv)
{
  QDir fileDir(argv[1]);
    return loadWithOutRequiredFile(fileDir.filePath("withoutTaxFile.seg").toStdString().c_str());
}