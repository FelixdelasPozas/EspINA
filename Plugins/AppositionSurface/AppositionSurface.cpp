#include "AppositionSurface.h"
#include "AppositionSurfaceExtension.h"
#include "AppositionSurfaceRenderer.h"
#include <common/extensions/SegmentationExtension.h>
#include <common/model/EspinaFactory.h>

#include <QDebug>

//-----------------------------------------------------------------------------
AppositionSurface::AppositionSurface()
{
}

//-----------------------------------------------------------------------------
void AppositionSurface::initFactoryExtension(EspinaFactory* factory)
{
  SegmentationExtension::SPtr segExtension(new AppositionSurfaceExtension());
  factory->registerSegmentationExtension(segExtension);
  factory->registerRenderer(new AppositionSurfaceRenderer());
}

Q_EXPORT_PLUGIN2(AppositionSurfacePlugin, AppositionSurface)
