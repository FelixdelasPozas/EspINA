#include "spatialExtension.h"

// Debug
#include "espina_debug.h"

#include "sample.h"
#include "cache/cachedObjectBuilder.h"

// Paraview
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>

#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>


using namespace SpatialExtension;

//!-----------------------------------------------------------------------
//! SPATIAL SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Sample's Spatial representation using vtkImagechangeInformation to
//! change input image spacing to the one specified by the user
//-----------------------------------------------------------------------------

const ISampleRepresentation::RepresentationId SampleRepresentation::ID  = "Spatial";

SampleRepresentation::SampleRepresentation(Sample* sample): ISampleRepresentation(sample)
{
  //TODO create VTK filter
  m_sample->spacing(m_spacing);
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  vtkFilter::Arguments volArgs;
  volArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, m_sample->id()));
  QString spacing = QString("%1,%2,%3").arg(m_spacing[0]).arg(m_spacing[1]).arg(m_spacing[2]);
  volArgs.push_back(vtkFilter::Argument("OutputSpacing",vtkFilter::DOUBLEVECT, spacing));
  m_rep = cob->createFilter("filters", "ImageChangeInformation", volArgs);
  assert(m_rep->numProducts() == 1);
}

//-----------------------------------------------------------------------------
SampleRepresentation::~SampleRepresentation()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep);
}

//-----------------------------------------------------------------------------
QString SampleRepresentation::id()
{
  return m_rep->id()+":0";
}

//-----------------------------------------------------------------------------
void SampleRepresentation::render(pqView* view, ViewType type)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  dp->setRepresentationVisibility(pipelineSource()->getOutputPort(0),view,true);
}

//-----------------------------------------------------------------------------
pqPipelineSource* SampleRepresentation::pipelineSource()
{
  return m_rep->pipelineSource();
}

//-----------------------------------------------------------------------------
void SampleRepresentation::setSpacing(double x, double y, double z)
{
  pqPipelineSource* changeInformationFilter = m_rep->pipelineSource();
//   changeInformationFilter->updatePipeline();
  m_spacing[0] = x;
  m_spacing[1] = y;
  m_spacing[2] = z;
  
  vtkSMPropertyHelper(changeInformationFilter->getProxy(), "OutputSpacing" ).Set(m_spacing, 3);
  //   mutex.lock();
  changeInformationFilter->getProxy()->UpdateVTKObjects();
}

void SampleRepresentation::spacing(double value[3])
{
  value[0] = m_spacing[0];
  value[1] = m_spacing[1];
  value[2] = m_spacing[2];
}

//!-----------------------------------------------------------------------
//! SPATIAL EXTENSION
//!-----------------------------------------------------------------------
//! Provides:
//! - Spatial Representation
//-----------------------------------------------------------------------------
SampleExtension::SampleExtension()
: m_spatialRep(NULL)
{
  m_availableRepresentations << SampleRepresentation::ID;
}

//-----------------------------------------------------------------------------
SampleExtension::~SampleExtension()
{
  if (m_spatialRep)
    delete m_spatialRep;
}

//-----------------------------------------------------------------------------
void SampleExtension::initialize(Sample* sample)
{
  EXTENSION_DEBUG(ID << " Initialized");
  m_sample = sample;
  m_spatialRep = new SampleRepresentation(m_sample);
}

//-----------------------------------------------------------------------------
ISampleRepresentation* SampleExtension::representation(QString rep)
{
  if (rep == SampleRepresentation::ID)
    return m_spatialRep;
  
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//-----------------------------------------------------------------------------
QVariant SampleExtension::information(QString info)
{
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//-----------------------------------------------------------------------------
ISampleExtension* SampleExtension::clone()
{
  return new SampleExtension();
}

