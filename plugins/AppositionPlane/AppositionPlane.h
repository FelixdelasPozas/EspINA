#ifndef APPOSITIONPLANE_H
#define APPOSITIONPLANE_H

#include <common/pluginInterfaces/IExtensionProvider.h>

//! Apposition Plane Plugin
class AppositionPlane
: public QObject
, public IExtensionProvider
{
  Q_OBJECT
  Q_INTERFACES(IExtensionProvider)

public:
  explicit AppositionPlane();
  virtual ~AppositionPlane(){}

  virtual void initExtensionProvider(EspinaFactory* factory);
};

#endif// APPOSITIONPLANE_H