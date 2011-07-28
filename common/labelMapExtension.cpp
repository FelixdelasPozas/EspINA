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
  filterArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, sample->id()));
  
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
    m_numberOfBlendedSeg = m_sample->segmentations().size();
    
    vtkstd::vector<vtkSMProxy *> inputs;
    vtkstd::vector<unsigned int> ports;
    
    // Ensure sample's mapper is the first input
    inputs.push_back(m_sample->creator()->pipelineSource()->getProxy());
    ports.push_back(0);
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      if (seg->visible())
      {
	seg->creator()->pipelineSource()->updatePipeline();
	inputs.push_back(seg->creator()->pipelineSource()->getProxy());
	ports.push_back(0);
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
	double labelColor[4] = {ci, segColor[0],segColor[1],segColor[2]};
	//TODO: NOTE: Each time we remove all inputs we have to re-assign colors
	// to labelmaps, nevertheless, paraview proxy somehow caches previous calls
	// to the Set method, and because parameters doesn't change, it doesn't update
	// the server component. Thus we change to a fake color and then to the good one.
	double fakeLabelColor[4] = {ci, -1, -1, -1};
	vtkSMPropertyHelper(m_rep->pipelineSource()->getProxy(),"InputColor").Set(fakeLabelColor,4);
	vtkSMPropertyHelper(m_rep->pipelineSource()->getProxy(),"InputColor").Set(labelColor,4);
	ci++;
      }
    }
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