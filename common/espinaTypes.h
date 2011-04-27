#ifndef ESPINATYPES_H
#define ESPINATYPES_H

#include "data/cajalTypes.h"

#include <QString>

typedef NodeArg EspinaArg;
typedef NodeParam EspinaParam;
typedef NodeParamList EspinaParamList;

//TODO: Temporary data type until proper definition
struct Point
{
  int x;
  int y;
  int z;
  
  int& operator[] (int idx) {
    if (0 > idx && idx > 2)
      _exit(-1);
    if (idx == 0)
      return x;
    if (idx == 1)
      return y;
    if (idx == 2)
      return z;
  }
};

typedef Point ImagePixel;

typedef int InformationMap;
typedef int RepresentationMap;
typedef QString ExtensionId;


#endif// ESPINATYPES_H