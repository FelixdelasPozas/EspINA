#include "AppositionPlaneExtension.h"

// Debug
#include <espina_debug.h>

// EspINA
#include <cache/cachedObjectBuilder.h>
#include <segmentation.h>
#include <sample.h>

#include <vtkSMPropertyHelper.h>
#include <pqPipelineSource.h>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <QApplication>

//!-----------------------------------------------------------------------
//! MORPHOLOGICAL EXTENSION--------------------------------------
//! Information Provided:
//! - Centroid

const ExtensionId AppositionPlaneExtension::ID = "AppositionPlaneExtension";

//------------------------------------------------------------------------
AppositionPlaneExtension::AppositionPlaneExtension()
: m_features(NULL)
, m_init(false)
{
  m_availableInformations << "AS Area" << "AS Perimeter";
}

//------------------------------------------------------------------------
AppositionPlaneExtension::~AppositionPlaneExtension()
{
  if (m_features)
  {
    EXTENSION_DEBUG("Deleted " << ID << " from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_features);
    m_features = NULL;
  }
}

//------------------------------------------------------------------------
ExtensionId AppositionPlaneExtension::id()
{
  return ID;
}


//------------------------------------------------------------------------
void AppositionPlaneExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments featuresArgs;
  featuresArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  m_features = cob->createFilter("filters","MorphologicalFeatures", featuresArgs);
  assert(m_features);
}

//------------------------------------------------------------------------
ISegmentationRepresentation* AppositionPlaneExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant AppositionPlaneExtension::information(QString info)
{
  if (!m_init)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_init = true;
    m_features->pipelineSource()->updatePipeline();
    
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Size").UpdateValueFromServer();
    QApplication::restoreOverrideCursor();
  }

  double spacing[3];
  m_seg->origin()->spacing(spacing);
  
  if (info == "AS Area")
      return 5;
  if (info == "AS Perimeter")
      return 7;
 
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
ISegmentationExtension* AppositionPlaneExtension::clone()
{
  return new AppositionPlaneExtension();
}