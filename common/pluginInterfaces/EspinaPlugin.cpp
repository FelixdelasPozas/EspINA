#include "EspinaPlugin.h"


QStringList ISegmentationExtension::dependencies()
{
  return QStringList();
}

QStringList ISegmentationExtension::availableRepresentations()
{
  return m_availableRepresentations;
}

QStringList ISegmentationExtension::availableInformations()
{
  return m_availableInformations;
}
