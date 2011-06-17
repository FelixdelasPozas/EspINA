#include "spatialExtension.h"
#include "cache/cachedObjectBuilder.h"
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineSource.h>

using namespace SpatialExtension;
//-----------------------------------------------------------------------------
SampleRepresentation::SampleRepresentation(Sample* sample): ISampleRepresentation(sample)
{
  //TODO create VTK filter
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  vtkFilter::Arguments volArgs;
  volArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, m_sample->id()));
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
  double spacing[3];
  pqPipelineSource* changeInformationFilter = m_rep->pipelineSource();
//   changeInformationFilter->updatePipeline();
  spacing[0] = x;
  spacing[1] = y;
  spacing[2] = z;
  
  vtkSMPropertyHelper(changeInformationFilter->getProxy(), "OutputSpacing" ).Set(spacing, 3);
  //   mutex.lock();
  changeInformationFilter->getProxy()->UpdateVTKObjects();
//   QString id(m_sample->id());
//   m_creator = CachedObjectBuilder::instance()->
//               registerProductCreator(id, changeInformationFilter);
//   
//   this->spacing(spacing);
//   qDebug() << "Spacing: " << spacing[0] << spacing[1] << spacing[2];
//   changeInformationFilter->updatePipeline();  
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
