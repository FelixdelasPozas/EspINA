/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "CountingRegionExtension.h"

// Debug
#include "espina_debug.h"

#include "filter.h"
#include "sample.h"
#include "segmentation.h"
#include "cache/cachedObjectBuilder.h"

#include <pqPipelineSource.h>
#include <pqOutputPort.h>
#include <vtkSMOutputPort.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineRepresentation.h>
#include <pqObjectBuilder.h>
#include <pq3DWidget.h>
#include <vtkBoxWidget2.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <pqScalarsToColors.h>

//!-----------------------------------------------------------------------
//! COUNTING REGION SEGMENTATION EXTENSION
//!-----------------------------------------------------------------------
//! Information Provided:
//! - Discarted
//------------------------------------------------------------------------
CountingRegion::SegmentationExtension::SegmentationExtension()
: m_discarted(NULL)
{
  m_availableInformations << "Discarted";
}

//------------------------------------------------------------------------
CountingRegion::SegmentationExtension::~SegmentationExtension()
{
  if (m_discarted)
  {
    EXTENSION_DEBUG("Deleted " << ID << " from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_discarted);
    m_discarted = NULL;
  }
}

//------------------------------------------------------------------------
ExtensionId CountingRegion::SegmentationExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingRegion::SegmentationExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  m_seg->creator()->pipelineSource()->updatePipeline();
  vtkFilter::Arguments countingArgs;
  countingArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  m_discarted = cob->createFilter("filters","CountingRegion", countingArgs);
  assert(m_discarted);
  
  SampleExtension *sampleExt = dynamic_cast<SampleExtension *>(m_seg->origin()->extension(CountingRegion::ID));
  updateRegions(sampleExt->regions());
}

//------------------------------------------------------------------------
ISegmentationRepresentation* CountingRegion::SegmentationExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant CountingRegion::SegmentationExtension::information(QString info)
{
  if (info == "Discarted")
  {
    int isDiscarted = 0;
    m_discarted->pipelineSource()->updatePipeline();
    vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").UpdateValueFromServer();
    vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").Get(&isDiscarted,1);
    m_seg->setVisible(!isDiscarted);
    return (bool)isDiscarted;
  }
  
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
void CountingRegion::SegmentationExtension::updateRegions(QMap<QString, BoundingRegion *>& regions)
{
  EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
  EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
  
  vtkSMProperty *p;
  
  vtkstd::vector<vtkSMProxy *> inputs;
  vtkstd::vector<unsigned int> ports;
  
  // First input is the segmentation object
  inputs.push_back(m_seg->creator()->pipelineSource()->getProxy());
  ports.push_back(0);
    
  foreach(BoundingRegion *region, regions)
  {
    region->pipelineSource()->updatePipeline();
    inputs.push_back(region->pipelineSource()->getProxy());
    ports.push_back(0);
  }
    
  p = m_discarted->pipelineSource()->getProxy()->GetProperty("Input");
  vtkSMInputProperty *inputRegions = vtkSMInputProperty::SafeDownCast(p);
  assert(inputRegions);
  if (inputRegions)
  {
    inputRegions->SetProxies(static_cast<unsigned int>(inputs.size())
    , &inputs[0]
    , &ports[0]);
  }
  
  m_discarted->pipelineSource()->updatePipeline();
  
  int isDiscarted = 0;
  vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").UpdateValueFromServer();
  vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").Get(&isDiscarted,1);
  m_seg->setVisible(!isDiscarted);
  m_seg->notifyInternalUpdate();
}

//------------------------------------------------------------------------
ISegmentationExtension* CountingRegion::SegmentationExtension::clone()
{
  return new SegmentationExtension();
}

//!-----------------------------------------------------------------------
//! BOUNDING REGION SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Base class for counting region's Bounding Region representations

const ISampleRepresentation::RepresentationId CountingRegion::BoundingRegion::ID  = "CountingRegion";

//------------------------------------------------------------------------
CountingRegion::BoundingRegion::BoundingRegion(Sample* sample)
: ISampleRepresentation(sample)
, m_boundigRegion(NULL)
{
  m_sample = sample;
}

//------------------------------------------------------------------------
CountingRegion::BoundingRegion::~BoundingRegion()
{
  if (m_boundigRegion)
  {
    EXTENSION_DEBUG("Deleted " << " bounded area " << " from " << m_sample->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_boundigRegion);
    m_boundigRegion = NULL;
  }

}

//------------------------------------------------------------------------
QString CountingRegion::BoundingRegion::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingRegion::BoundingRegion::render(pqView* view, ViewType type)
{

}

//------------------------------------------------------------------------
pqPipelineSource* CountingRegion::BoundingRegion::pipelineSource()
{
  assert(m_boundigRegion);
  return  m_boundigRegion->pipelineSource();
}

//!-----------------------------------------------------------------------
//! RECTANGULAR REGION SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Represent a Bounding Region applied to the sample

//------------------------------------------------------------------------
RectangularRegion::RectangularRegion(Sample* sample,
  int left, int top, int upper,
  int right, int bottom, int lower) 
: BoundingRegion(sample)
{
  EXTENSION_DEBUG("Rectangular Region: (" <<  
  left << "," <<
  top << "," <<
  upper << "," <<
  right << "," <<
  bottom << "," <<
  lower << ") Initialized");
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  double spacing[3]; 
  m_sample->spacing(spacing);
  // Configuration of Bounding Region interface
  vtkFilter::Arguments regionArgs;
  regionArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,""));
  regionArgs.push_back(vtkFilter::Argument("Inclusion",vtkFilter::INTVECT, QString("%1,%2,%3")
					   .arg(int(left*spacing[0])).arg(int(top*spacing[1])).arg(int(upper*spacing[2]))));
  regionArgs.push_back(vtkFilter::Argument("Exclusion",vtkFilter::INTVECT, QString("%1,%2,%3")
					   .arg(int(right*spacing[0])).arg(int(bottom*spacing[1])).arg(int(lower*spacing[2]))));
  m_boundigRegion = cob->createFilter("filters","RectangularBoundingRegion", regionArgs);
  
  if (!m_boundigRegion)
  {
    qDebug() << "Couldn't create Bounding Region Filter";
    assert(false);
  }
  pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
  m_box =  builder->createProxy("implicit_functions","Box",pqApplicationCore::instance()->getActiveServer(),"widgets");
  
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(m_sample->creator()->pipelineSource()->getProxy(), m_box);
  m_widget = widgets.first();
}

//------------------------------------------------------------------------
RectangularRegion::~RectangularRegion()
{
}

//------------------------------------------------------------------------
void RectangularRegion::render(pqView* view, ViewType type)
{
//   m_widget->setView(view);
//   m_widget->setWidgetVisible(true);
//   m_widget->select();
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqDataRepresentation *dr = dp->setRepresentationVisibility(pipelineSource()->getOutputPort(0),view,true);
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  rep->setColorField("Type");
  
  vtkSMProxy * lut = rep->getLookupTableProxy();
  double colors[8] = {0,1,0,0,255,0,1,0};
  vtkSMPropertyHelper(lut,"RGBPoints").Set(colors,8);
  lut->UpdateVTKObjects();
  
  double opacity = 0.7;
  vtkSMPropertyHelper(rep->getProxy(),"Opacity").Set(opacity);
  rep->getProxy()->UpdateVTKObjects();
}

//------------------------------------------------------------------------
void RectangularRegion::setInclusive(int left, int top, int upper)
{

}


//------------------------------------------------------------------------
void RectangularRegion::setExclusive(int right, int bottom, int lower)
{

}

//!-----------------------------------------------------------------------
//! ADAPTIVE REGION SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Represent a Bounding Region applied to the sample
//------------------------------------------------------------------------
AdaptiveRegion::AdaptiveRegion(Sample* sample, int left, int top, int upper, int right, int bottom, int lower)
: BoundingRegion(sample)
{
  EXTENSION_DEBUG("Adaptive Region: (" <<  
  left << "," <<
  top << "," <<
  upper << "," <<
  right << "," <<
  bottom << "," <<
  lower << ") Initialized");
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  // Configuration of Bounding Region interface
  vtkFilter::Arguments regionArgs;
  regionArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_sample->id()));
  regionArgs.push_back(vtkFilter::Argument("Inclusion",vtkFilter::INTVECT,QString("%1,%2,%3").arg(left).arg(top).arg(upper)));
  regionArgs.push_back(vtkFilter::Argument("Exclusion",vtkFilter::INTVECT,QString("%1,%2,%3").arg(right).arg(bottom).arg(lower)));
  m_boundigRegion = cob->createFilter("filters","AdaptiveBoundingRegion", regionArgs);
  
  if (!m_boundigRegion)
  {
    qDebug() << "Couldn't create Bounding Region Filter";
    assert(false);
  }
}


//------------------------------------------------------------------------
AdaptiveRegion::~AdaptiveRegion()
{
}

void AdaptiveRegion::render(pqView* view, ViewType type)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqDataRepresentation *dr = dp->setRepresentationVisibility(pipelineSource()->getOutputPort(0),view,true);
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  
  vtkSMProxy * lut = rep->getLookupTableProxy();
  double colors[8] = {0,1,0,0,255,0,1,0};
  vtkSMPropertyHelper(lut,"RGBPoints").Set(colors,8);
  lut->UpdateVTKObjects();
  
  
  double opacity = 0.5;
  vtkSMPropertyHelper(rep->getProxy(),"Opacity").Set(opacity);
  rep->getProxy()->UpdateVTKObjects();
}

//------------------------------------------------------------------------
void AdaptiveRegion::setInclusive(int left, int top, int upper)
{

}

//------------------------------------------------------------------------
void AdaptiveRegion::setExclusive(int right, int bottom, int lower)
{

}


//!-----------------------------------------------------------------------
//! CROSSHAIR EXTENSION
//!-----------------------------------------------------------------------
//! Provides:
//! - CountingRegion Representation

//------------------------------------------------------------------------
CountingRegion::SampleExtension::SampleExtension()
: m_numRepresentations(0)
{
}

//------------------------------------------------------------------------
CountingRegion::SampleExtension::~SampleExtension()
{
  foreach(BoundingRegion *region, m_regions)
    delete region;
  
  m_regions.clear();
}

//------------------------------------------------------------------------
void CountingRegion::SampleExtension::initialize(Sample* sample)
{
  EXTENSION_DEBUG(ID << " Initialized");
  m_sample = sample;
}

//------------------------------------------------------------------------
ISampleRepresentation* CountingRegion::SampleExtension::representation(QString rep)
{
  if (m_regions.contains(rep))
    return m_regions[rep];
  
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant CountingRegion::SampleExtension::information(QString info)
{
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
QString CountingRegion::SampleExtension::createAdaptiveRegion(int left, int top, int upper, int right, int bottom, int lower)
{
  AdaptiveRegion *region = new AdaptiveRegion(m_sample, left, top, upper, right, bottom, lower);
  assert(region);
  
  QString repName = QString("Adaptative Region (%1,%2,%3,%4,%5,%6)") 
    .arg(left).arg(top).arg(upper).arg(right).arg(bottom).arg(lower);
    
  if (!m_regions.contains(repName))
  {
    m_regions[repName] = region;
    m_numRepresentations++;
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
      if (!ext)
      {
	qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
	assert(false);
      }
      ext->updateRegions(m_regions);
    }
  }
  return repName;
}


//------------------------------------------------------------------------
QString CountingRegion::SampleExtension::createRectangularRegion(int left, int top, int upper, int right, int bottom, int lower)
{
  RectangularRegion *region = new RectangularRegion(m_sample, left, top, upper, right, bottom, lower);
  assert(region);
  
  QString repName = QString("Rectangular Region (%1,%2,%3,%4,%5,%6)") 
    .arg(left).arg(top).arg(upper).arg(right).arg(bottom).arg(lower);
    
    if (!m_regions.contains(repName))
    {
      m_regions[repName] = region;
      m_numRepresentations++;
      
      foreach(Segmentation *seg, m_sample->segmentations())
      {
	SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
	if (!ext)
	{
	  qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
	  assert(false);
	}
	ext->updateRegions(m_regions);
      }
    }
  return repName;
}

//------------------------------------------------------------------------
void CountingRegion::SampleExtension::removeRegion(QString& name)
{
  if (m_regions.contains(name))
  {
    BoundingRegion *region = m_regions[name];
    m_regions.remove(name);
    m_numRepresentations--;
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
      if (!ext)
      {
	qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
	assert(false);
      }
      ext->updateRegions(m_regions);
    }
    delete region;
  }
}


//------------------------------------------------------------------------
ISampleExtension* CountingRegion::SampleExtension::clone()
{
  return new SampleExtension();
}



