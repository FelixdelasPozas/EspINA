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


#include "labelMapExtension.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "sample.h"
#include "segmentation.h"
#include "cache/cachedObjectBuilder.h"

#include <vtkSMProperty.h>
#include <pqPipelineSource.h>
#include <vtkSMInputProperty.h>
#include <vtkSMProxy.h>
#include <QAction>
#include <vtkSMPropertyHelper.h>

#include <QElapsedTimer>
#include "spatialExtension.h"

using namespace LabelMapExtension;

//!-----------------------------------------------------------------------
//! LABELMAP SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Sample's LabelMap representation using vtkImageLabelMapBlend to
//! blend all segmentations into the sample

const ISegmentationRepresentation::RepresentationId SampleRepresentation::ID  = "LabelMap";

//------------------------------------------------------------------------
SampleRepresentation::SampleRepresentation(Sample* sample)
: ISampleRepresentation(sample)
, m_enabled(true)
, m_numberOfBlendedSeg(0)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments filterArgs;
  
  ISampleRepresentation *spatialRep;
  if ((spatialRep =  m_sample->representation(SpatialExtension::SampleRepresentation::ID)))
  {
    SpatialExtension::SampleRepresentation* rep = 
    dynamic_cast<SpatialExtension::SampleRepresentation*>(spatialRep);
    filterArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, spatialRep->id()));
  } else
  {
    filterArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, sample->id()));
  }
  m_rep = cob->createFilter("filters", "ImageLabelMapBlend", filterArgs);
  assert(m_rep);
}

//------------------------------------------------------------------------
SampleRepresentation::~SampleRepresentation()
{
  EXTENSION_DEBUG("Deleted " << ID << " Representation from " << m_sample->id());
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep);
}

//------------------------------------------------------------------------
QString SampleRepresentation::id()
{
  if (m_enabled)
    return m_rep->id()+":0";
  else
    return m_sample->id();
}

//------------------------------------------------------------------------
pqPipelineSource* SampleRepresentation::pipelineSource()
{
//   qDebug() << "[LabelMapExtension]: Pipeline requested";
  if (m_enabled)
    return m_rep->pipelineSource();
  else
    return m_sample->creator()->pipelineSource();
}

//------------------------------------------------------------------------
void SampleRepresentation::render(pqView* view, ViewType type)
{
  assert(false);
}

//------------------------------------------------------------------------
void SampleRepresentation::requestUpdate(bool force)
{
  vtkSMProperty* p;
  
  if (m_enabled && (m_numberOfBlendedSeg != m_sample->segmentations().size() || force)) 
  {
    qDebug() << "[LabelMapExtension]: Request Update";
    QElapsedTimer timer;
    timer.start();
    m_numberOfBlendedSeg = m_sample->segmentations().size();
    
    vtkstd::vector<vtkSMProxy *> inputs;
    vtkstd::vector<unsigned int> ports;
    
    // Ensure sample's mapper is the first input
    ISampleRepresentation *spatialRep;
    if ((spatialRep =  m_sample->representation(SpatialExtension::SampleRepresentation::ID)))
    {
      SpatialExtension::SampleRepresentation* rep = 
      dynamic_cast<SpatialExtension::SampleRepresentation*>(spatialRep);
      inputs.push_back(spatialRep->pipelineSource()->getProxy());
    } else
    {
      inputs.push_back(m_sample->creator()->pipelineSource()->getProxy());
    }
    ports.push_back(0);
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      if (seg->visible())
      {
// 	seg->creator()->pipelineSource()->updatePipeline();
	inputs.push_back(seg->creator()->pipelineSource()->getProxy());
	ports.push_back(seg->portNumber());
      }
    }
    
    p = m_rep->pipelineSource()->getProxy()->GetProperty("Input");
    vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
    if (input)
    {
//       input->RemoveAllProxies();
//       m_rep->pipelineSource()->getProxy()->UpdateVTKObjects();
      input->SetProxies(static_cast<unsigned int>(inputs.size())
      , &inputs[0]
      , &ports[0]);
      m_rep->pipelineSource()->getProxy()->UpdateVTKObjects();
    }
    
    int ci = 1;//colorinput
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      if (seg->visible())
      {
	double segColor[4];
	seg->color(segColor);
// 	std::cout << "Input " << ci << "_" << seg << " has COLOR: " << segColor[0] << segColor[1] << segColor[2] << std::endl;
	double labelColor[5] = {ci, segColor[0],segColor[1],segColor[2], seg->isSelected()};
	//TODO: NOTE: Each time we remove all inputs we have to re-assign colors
	// to labelmaps, nevertheless, paraview proxy somehow caches previous calls
	// to the Set method, and because parameters doesn't change, it doesn't update
	// the server component. Thus we change to a fake color and then to the good one.
	double fakeLabelColor[5] = {ci, -1, -1, -1, 0};
	vtkSMPropertyHelper(m_rep->pipelineSource()->getProxy(),"InputColor").Set(fakeLabelColor,5);
	vtkSMPropertyHelper(m_rep->pipelineSource()->getProxy(),"InputColor").Set(labelColor,5);
	ci++;
      }
    }
    m_rep->pipelineSource()->updatePipeline();
    qDebug() << "Updating Label Map took: " << timer.elapsed();
  }
  
  emit representationUpdated();
}

//------------------------------------------------------------------------
void SampleRepresentation::setEnable(bool value)
{
  if (m_enabled != value)
  {
    m_enabled = value;
    requestUpdate();
  }
}

//!-----------------------------------------------------------------------
//! LABELMAP EXTENSION
//!-----------------------------------------------------------------------
//! Provides:
//! - LabelMap Representation
//------------------------------------------------------------------------
SampleExtension::SampleExtension(QAction* toggleVisibility)
: m_toggleVisibility(toggleVisibility)
, m_labelRep(NULL)
{
    m_availableRepresentations << SampleRepresentation::ID;
}

//------------------------------------------------------------------------
SampleExtension::~SampleExtension()
{
  if (m_labelRep)
    delete m_labelRep;
}

//------------------------------------------------------------------------
void SampleExtension::initialize(Sample* sample)
{
  EXTENSION_DEBUG(ID << " Initializing");
  m_sample = sample;
  m_labelRep = new SampleRepresentation(m_sample);
  QObject::connect(m_toggleVisibility,SIGNAL(toggled(bool)),m_labelRep,SLOT(setEnable(bool)));
  EXTENSION_DEBUG(ID << " Initialized");
}

//------------------------------------------------------------------------
ISampleRepresentation* SampleExtension::representation(QString rep)
{
  if (rep == SampleRepresentation::ID)
    return m_labelRep;
  
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant SampleExtension::information(QString info)
{
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}


//------------------------------------------------------------------------
ISampleExtension* SampleExtension::clone()
{
  return new SampleExtension(m_toggleVisibility);
}