#include <Core/IO/EspinaIO.h>

int loadWithoutSegFile(int argc, char** argv)
{
  QFileInfo file("NoneFile.seg");
  EspinaModel *model;
  QDir tmpDir;

  if(EspinaIO::loadSegFile(file, model, tmpDir) == EspinaIO::SUCCESS)
    return 1;

  return 0;
}