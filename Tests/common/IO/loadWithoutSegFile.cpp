#include <Core/IO/EspinaIO.h>

int loadWithoutSegFile(int argc, char** argv)
{
  QFileInfo file("NoneFile.seg");
  QDir tmpDir;

  if(EspinaIO::loadSegFile(file, NULL, tmpDir) == EspinaIO::SUCCESS)
    return 1;

  return 0;
}