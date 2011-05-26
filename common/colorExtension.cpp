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

#include <vtkSMProperty.h>
#include <vtkSMProxyProperty.h>
#include <pqPipelineSource.h>

//DEBUG
#include <QDebug>
#include <assert.h>

ColorRepresentation::ColorRepresentation(Segmentation* seg)
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
}

ColorRepresentation::~ColorRepresentation()
{
  delete m_rep;
}



pqPipelineSource* ColorRepresentation::pipelineSource()
{
  double rgba[4];
  m_seg->color(rgba);
  //TODO: change to binary segmentation images
  m_LUT->SetTableValue(255, rgba[0], rgba[1], rgba[2], 0.6);
  m_LUT->UpdateVTKObjects();
  return m_rep->pipelineSource();
}

void ColorRepresentation::render(pqView* view)
{
  qDebug() << "Color Representation: Invalid rendering";
}


const ExtensionId ColorExtension::ID = "ColorExtension";

ExtensionId ColorExtension::id()
{
  return ID;
}

void ColorExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
}


void ColorExtension::addInformation(InformationMap& map)
{
  qDebug() << "Color Extension: No extra information provided.";
}

void ColorExtension::addRepresentations(RepresentationMap& map)
{
  ColorRepresentation *rep = new ColorRepresentation(m_seg);
  map.insert("Color", rep);
  qDebug() << "Color Extension: Color Representation Added";
}

ISegmentationExtension* ColorExtension::clone()
{
  return new ColorExtension();
}

