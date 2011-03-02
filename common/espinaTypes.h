#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include "data/cajalTypes.h"

typedef NodeArg EspinaArg;
typedef NodeParam EspinaParam;
typedef NodeParamList EspinaParamList;

//TODO: Temporary data type until proper definition
struct Point
{
  int x;
  int y;
  int z;
};

typedef Point ImagePixel;




#endif// ESPINATYPES_H