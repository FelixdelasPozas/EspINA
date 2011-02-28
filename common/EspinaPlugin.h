#ifndef ESPINAPLUGIN_H
#define ESPINAPLUGIN_H

#include "espinaTypes.h"

class EspinaPlugin
{
public:
  EspinaPlugin(){};
  virtual void LoadAnalaisys(EspinaParamList args) = 0;

};

#endif // ESPINAPLUGIN_H
