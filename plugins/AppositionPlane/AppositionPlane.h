#ifndef APPOSITIONPLANE_H
#define APPOSITIONPLANE_H

#include <common/pluginInterfaces/IFactoryExtension.h>

//! Apposition Plane Plugin
class AppositionPlane
: public QObject
, public IFactoryExtension
{
  Q_OBJECT
  Q_INTERFACES(IFactoryExtension)

public:
  explicit AppositionPlane();
  virtual ~AppositionPlane(){}

  virtual void initFactoryExtension(EspinaFactory* factory);
};

#endif// APPOSITIONPLANE_H