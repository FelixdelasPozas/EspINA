#include "FilePack.h"

int loadWithoutSegFile(int argc, char** argv)
{
  QString taxContent, traceContent;
  QTextStream tax, trace;
  tax.setString(&taxContent);
  trace.setString(&traceContent);
  
  if( IOEspinaFile::loadFile("NoneFile.seg", tax, trace) )
    return 1;
  return 0;
}