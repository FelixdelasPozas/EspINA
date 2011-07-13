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

#include "products.h"
#include "cache/cachedObjectBuilder.h"

//DEBUG
#include <QDebug>
#include <vtkSMProperty.h>
#include <pqPipelineSource.h>
#include <vtkSMInputProperty.h>
#include <vtkSMProxy.h>
#include <QAction>
#include <vtkSMPropertyHelper.h>

using namespace LabelMapExtension;


SampleRepresentation::SampleRepresentation(Sample* sample)
: ISampleRepresentation(sample)
, m_enable(true)
, m_numberOfBlendedSeg(0)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments filterArgs;
  filterArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, sample->id()));
  
  m_rep = cob->createFilter("filters", "ImageLabelMapBlend", filterArgs);
  assert(m_rep);
}

SampleRepresentation::~SampleRepresentation()
{
  //qDebug() << "Deleted LabelMap Representation from " << m_sample->id();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep);
}

QString SampleRepresentation::id()
{
  if (m_enable)
    return m_rep->id()+":0";
  else
    return m_sample->id();
}


pqPipelineSource* SampleRepresentation::pipelineSource()
{
  if (m_enable)
    return m_rep->pipelineSource();
  else
    return m_sample->creator()->pipelineSource();
}

void SampleRepresentation::render(pqView* view, ViewType type)
{
}

void SampleRepresentation::requestUpdate(bool force)
{
  vtkSMProperty* p;
//   p = m_rep->pipelineSource()->getProxy()->GetProperty("Input");
//   vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
//   foreach(Segmentation *seg, m_sample->segmentations())
//   {
//     if (seg->visible())
//     {
//       input->AddInputConnection(seg->creator()->pipelineSource()->getProxy(),0);
//     }
//   }
//   emit representationUpdated();//DEBUG
//   return;//DEBUG
  
  if (m_numberOfBlendedSeg != m_sample->segmentations().size() || force) 
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
	inputs.push_back(seg->creator()->pipelineSource()->getProxy());
	ports.push_back(0);
      }
    }
    
    p = m_rep->pipelineSource()->getProxy()->GetProperty("Input");
    vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
    if (input)
    {
      input->RemoveAllProxies();
      m_rep->pipelineSource()->getProxy()->UpdateVTKObjects();
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
	double c[4] = {ci, segColor[0],segColor[1],segColor[2]};
	vtkSMPropertyHelper(m_rep->pipelineSource()->getProxy(),"InputColor").Set(c,4);
	ci++;
      }
    }
  }
  
  emit representationUpdated();
}

void SampleRepresentation::setEnable(bool value)
{
  if (m_enable != value)
  {
    m_enable = value;
    requestUpdate();
  }
}

SampleExtension::SampleExtension(QAction* toggleVisibility)
: m_toggleVisibility(toggleVisibility)
{
}

SampleExtension::~SampleExtension()
{
}


void SampleExtension::initialize(Sample* sample)
{
  m_sample = sample;
}

void SampleExtension::addInformation(ISampleExtension::InformationMap& map)
{
  qDebug() << ID << "No extra information provided.";
}

void SampleExtension::addRepresentations(ISampleExtension::RepresentationMap& map)
{
  SampleRepresentation *rep = new SampleRepresentation(m_sample);
  map.insert("02_LabelMap", rep);
  QObject::connect(m_toggleVisibility,SIGNAL(toggled(bool)),rep,SLOT(setEnable(bool)));
  //qDebug() << ID << "Label Map Representation Added";
}

ISampleExtension* SampleExtension::clone()
{
  return new SampleExtension(m_toggleVisibility);
}
