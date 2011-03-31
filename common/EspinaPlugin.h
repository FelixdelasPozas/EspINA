#ifndef ESPINAPLUGIN_H
#define ESPINAPLUGIN_H

#include "espinaTypes.h"

//! Interface to extend segmentation behaviour
class ISegmentationExtension {
public:
  
  virtual ExtensionId id() = 0;
  virtual void initialize() = 0;
  virtual void addInformation(InformationMap &map) = 0;
  virtual void addRepresentations(RepresentationMap &map) = 0;
  
  //! Prototype
  virtual ISegmentationExtension *clone() = 0;
  
protected:
  ISegmentationExtension() : m_init(false){}
  bool m_init;
};

class EspinaPlugin
{
public:

  virtual void LoadAnalisys(EspinaParamList args) = 0;

};

#endif // ESPINAPLUGIN_H
