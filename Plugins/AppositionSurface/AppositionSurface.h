#ifndef APPOSITIONPLANE_H
#define APPOSITIONPLANE_H

#include <common/pluginInterfaces/IFactoryExtension.h>

//! Apposition Surface Plugin
class AppositionSurface
: public QObject
, public IFactoryExtension
{
  Q_OBJECT
  Q_INTERFACES(IFactoryExtension)

public:
  explicit AppositionSurface();
  virtual ~AppositionSurface(){}

  virtual void initFactoryExtension(EspinaFactory* factory);
};

#endif// APPOSITIONPLANE_H