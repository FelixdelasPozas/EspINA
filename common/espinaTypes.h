#ifndef ESPINATYPES_H
#define ESPINATYPES_H

// #include "data/cajalTypes.h"

#include <QString>
#include <QMap>
#include <QVariant>
//! A Trace node argument. It only has semantic meaning
typedef QString NodeArg;
//! The value of a node argument
typedef QString ParamValue;
typedef std::pair<NodeArg, ParamValue> NodeParam;
typedef std::vector<NodeParam> NodeParamList;

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

typedef QString ExtensionId;

enum ViewType
{
  VIEW_PLANE_FIRST = 0,
  VIEW_PLANE_XY    = 0,
  VIEW_PLANE_YZ    = 1,
  VIEW_PLANE_XZ    = 2,
  VIEW_PLANE_LAST  = 2,
  VIEW_3D = 3
};


#endif// ESPINATYPES_H