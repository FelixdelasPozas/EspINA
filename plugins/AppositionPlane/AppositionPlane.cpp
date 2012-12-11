#include "AppositionPlane.h"
#include "AppositionPlaneExtension.h"
#include "AppositionPlaneRenderer.h"
#include <common/extensions/SegmentationExtension.h>
#include <common/model/EspinaFactory.h>

#include <QDebug>

//-----------------------------------------------------------------------------
AppositionPlane::AppositionPlane()
{
}

//-----------------------------------------------------------------------------
void AppositionPlane::initFactoryExtension(EspinaFactory* factory)
{
  SegmentationExtension::SPtr segExtension(new AppositionPlaneExtension());
  factory->registerSegmentationExtension(segExtension);
  factory->registerRenderer(new AppositionPlaneRenderer());
}

Q_EXPORT_PLUGIN2(AppositionPlanePlugin, AppositionPlane)
