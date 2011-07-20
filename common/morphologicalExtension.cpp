#include "morphologicalExtension.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "cachedObjectBuilder.h"
#include "segmentation.h"

#include <vtkSMPropertyHelper.h>
#include <pqPipelineSource.h>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMDoubleVectorProperty.h>

//!-----------------------------------------------------------------------
//! MORPHOLOGICAL EXTENSION
//!-----------------------------------------------------------------------
//! Information Provided:
//! - Centroid

const ExtensionId MorphologicalExtension::ID  = "Morphological";

//------------------------------------------------------------------------
MorphologicalExtension::MorphologicalExtension()
: m_features(NULL)
, m_init(false)
{
  m_availableInformations << "Centroid_X" << "Centroid_Y" << "Centroid_Z";
}

//------------------------------------------------------------------------
MorphologicalExtension::~MorphologicalExtension()
{
}

//------------------------------------------------------------------------
ExtensionId MorphologicalExtension::id()
{
  return ID;
}


//------------------------------------------------------------------------
void MorphologicalExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments featuresArgs;
  featuresArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  m_features = cob->createFilter("filters","MorphologicalFeatures", featuresArgs);
  assert(m_features);
}

//------------------------------------------------------------------------
ISegmentationRepresentation* MorphologicalExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant MorphologicalExtension::information(QString info)
{
  if (!m_init)
  {
    m_init = true;
    m_features->pipelineSource()->updatePipeline();
  }
  
  vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Centroid").UpdateValueFromServer();
  vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Centroid").Get(m_centroid,3);
  if (info == "Centroid_X")
      return m_centroid[0];
  if (info == "Centroid_Y")
      return m_centroid[1];
  if (info == "Centroid_Z")
      return m_centroid[2];
  
 
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
ISegmentationExtension* MorphologicalExtension::clone()
{
  return new MorphologicalExtension();
}

