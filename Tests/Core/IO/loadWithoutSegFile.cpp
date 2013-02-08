#include <Core/IO/EspinaIO.h>

int loadWithoutSegFile(int argc, char** argv)
{
  QFileInfo file("NoneFile.seg");

  if(EspINA::EspinaIO::loadSegFile(file, NULL) == EspINA::EspinaIO::SUCCESS)
    return 1;

  return 0;
}
