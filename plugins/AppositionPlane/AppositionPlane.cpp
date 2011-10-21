#include "AppositionPlane.h"

// Debug
#include <espina_debug.h>

// EspINA
#include <espina.h>
#include <sample.h>

#include <espINAFactory.h>
#include "AppositionPlaneExtension.h"

//-----------------------------------------------------------------------------
AppositionPlane::AppositionPlane(QObject* parent)
{
}

//-----------------------------------------------------------------------------
void AppositionPlane::onStartup()
{
  AppositionPlaneExtension appPlane;
  EspINAFactory::instance()->addSegmentationExtension(&appPlane);
}