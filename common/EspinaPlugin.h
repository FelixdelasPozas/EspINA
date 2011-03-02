#ifndef ESPINAPLUGIN_H
#define ESPINAPLUGIN_H

#include "espinaTypes.h"

class EspinaPlugin
{
public:

  virtual void LoadAnalisys(EspinaParamList args) = 0;

};

#endif // ESPINAPLUGIN_H
