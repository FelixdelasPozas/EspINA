#include "AppositionPlane.h"
#include "AppositionPlaneExtension.h"
#include "AppositionPlaneRenderer.h"
#include <common/extensions/SegmentationExtension.h>
#include <common/model/EspinaFactory.h>

#include <QDebug>

//-----------------------------------------------------------------------------
AppositionPlane::AppositionPlane()
{
  qDebug() << "Loading AP";
  SegmentationExtension::SPtr segExtension(new AppositionPlaneExtension());
  EspinaFactory::instance()->registerSegmentationExtension(segExtension);
  EspinaFactory::instance()->registerRenderer(new AppositionPlaneRenderer());
}

Q_EXPORT_PLUGIN2(AppositionPlanePlugin, AppositionPlane)