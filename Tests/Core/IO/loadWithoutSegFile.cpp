#include <Core/IO/SegFileReader.h>

using namespace EspINA;

int loadWithoutSegFile(int argc, char** argv)
{
  QFileInfo file("NoneFile.seg");

  if(SegFileReader::loadSegFile(file, NULL) == IOErrorHandler::SUCCESS)
    return 1;

  return 0;
}
