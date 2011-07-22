#include "spatialExtension.h"

#include "sample.h"
#include "cache/cachedObjectBuilder.h"

// Debug
#include "espina_debug.h"

// Paraview
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>


using namespace SpatialExtension;
//-----------------------------------------------------------------------------
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

#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>
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


/*
//-----------------------------------------------------------------------------
void SampleRepresentation::requestUpdate(bool force)
{
  emit representationUpdated();
}
*/
//-----------------------------------------------------------------------------
void SampleExtension::addInformation(ISampleExtension::InformationMap& map)
{

}

//-----------------------------------------------------------------------------
void SampleExtension::addRepresentations(ISampleExtension::RepresentationMap& map)
{
  SampleRepresentation *rep = new SampleRepresentation(m_sample);
  map.insert("00_Spatial", rep);
}

//-----------------------------------------------------------------------------
ISampleExtension* SampleExtension::clone()
{
  return new SampleExtension();
}

//-----------------------------------------------------------------------------
void SampleExtension::initialize(Sample* sample)
{
  m_sample = sample;
}
