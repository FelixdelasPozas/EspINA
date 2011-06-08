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


#include "colorExtension.h"

#include "cache/cachedObjectBuilder.h"
#include "proxies/vtkSMRGBALookupTableProxy.h"

#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <vtkSMProperty.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMPropertyHelper.h>
#include <pqPipelineSource.h>
#include <pqScalarsToColors.h>
#include <pqLookupTableManager.h>

//DEBUG
#include <QDebug>
#include <assert.h>

using namespace ColorExtension;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SampleRepresentation::SampleRepresentation(Sample* sample): ISampleRepresentation(sample)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  pqServer *server =  pqApplicationCore::instance()->getActiveServer();
  pqLookupTableManager *lutManager = pqApplicationCore::instance()->getLookupTableManager();
  
  vtkFilter::Arguments volArgs;
  volArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, m_sample->id()));
  m_rep = cob->createFilter("filters", "ImageMapToColors", volArgs);
  assert(m_rep->numProducts() == 1);
  
  vtkSMProperty* p;
  // Get (or create if it doesn't exit) the lut for the background image
  m_LUT = lutManager->getLookupTable(server, "Greyscale", 4, 0);
  assert(m_LUT);
  double gray[8] = {0, 0, 0, 0, 255, 1, 1, 1};
  vtkSMPropertyHelper(m_LUT->getProxy(),"RGBPoints").Set(gray,8);
  m_LUT->getProxy()->UpdateVTKObjects();
  
  // Set the greyLUT for the mapper
  p = m_rep->pipelineSource()->getProxy()->GetProperty("LookupTable");
  vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
  if (lut)
  {
    lut->SetProxy(0, m_LUT->getProxy());
  }
  
  m_rep->pipelineSource()->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
SampleRepresentation::~SampleRepresentation()
{
  qDebug() << "Deleted Color Representation from " << m_sample->id();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep);
//   m_LUT->Delete();
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
//-----------------------------------------------------------------------------
void SampleExtension::initialize(Sample* sample)
{
  m_sample = sample;
}

//-----------------------------------------------------------------------------
void SampleExtension::addInformation(ISampleExtension::InformationMap& map)
{
  qDebug() << "Sample" << ID << ": No extra information provided.";
}

//-----------------------------------------------------------------------------
void SampleExtension::addRepresentations(ISampleExtension::RepresentationMap& map)
{
  SampleRepresentation *rep = new SampleRepresentation(m_sample);
  map.insert("01_Color", rep);
  qDebug() << "Sample"<< ID << ": Color Representation Added";
}

//-----------------------------------------------------------------------------
ISampleExtension* SampleExtension::clone()
{
  return new SampleExtension();
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SegmentationRepresentation::SegmentationRepresentation(Segmentation* seg)
: ISegmentationRepresentation(seg)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments volArgs;
  volArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, m_seg->id()));
  m_rep = cob->createFilter("filters", "ImageMapToColors", volArgs);
  assert(m_rep->numProducts() == 1);
  
  vtkSMProperty* p;

  //TODO: Use smart pointers
  m_LUT = vtkSMRGBALookupTableProxy::New();
  m_LUT->SetTableValue(0,0,0,0,0);
  double rgba[4];
  m_seg->color(rgba);
  rgba[0] = 1;
  rgba[1] = 0;
  rgba[2] = 0;
  //TODO: change to binary segmentation images
  m_LUT->SetTableValue(255, rgba[0], rgba[1], rgba[2], 0.6);
  m_LUT->UpdateVTKObjects();

  // Set the greyLUT for the slicemapper
  p = m_rep->pipelineSource()->getProxy()->GetProperty("LookupTable");
  vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
  if (lut)
  {
    lut->SetProxy(0, m_LUT);
  }
  m_rep->pipelineSource()->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
SegmentationRepresentation::~SegmentationRepresentation()
{
  qDebug() << "Deleted Color Representation from " << m_seg->id();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep);
  m_LUT->Delete();
}


//-----------------------------------------------------------------------------
QString SegmentationRepresentation::id()
{
  return m_rep->id()+":0";
}

//-----------------------------------------------------------------------------
void SegmentationRepresentation::render(pqView* view)
{
  qDebug() << "Color Representation: Invalid rendering";
}

//-----------------------------------------------------------------------------
pqPipelineSource* SegmentationRepresentation::pipelineSource()
{
  double rgba[4];
  m_seg->color(rgba);
  //TODO: change to binary segmentation images
  m_LUT->SetTableValue(255, rgba[0], rgba[1], rgba[2], 0.6);
  m_LUT->UpdateVTKObjects();
  //m_rep->pipelineSource()->updatePipeline();
  return m_rep->pipelineSource();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SegmentationExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
}

//-----------------------------------------------------------------------------
void SegmentationExtension::addInformation(InformationMap& map)
{
  qDebug() << "Color Extension: No extra information provided.";
}

//-----------------------------------------------------------------------------
void SegmentationExtension::addRepresentations(RepresentationMap& map)
{
  SegmentationRepresentation *rep = new SegmentationRepresentation(m_seg);
  map.insert("01_Color", rep);
  qDebug() << "Color Extension: Color Representation Added";
}

//-----------------------------------------------------------------------------
ISegmentationExtension* SegmentationExtension::clone()
{
  return new SegmentationExtension();
}

