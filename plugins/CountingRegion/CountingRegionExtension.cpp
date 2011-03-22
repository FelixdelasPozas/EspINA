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

#include "traceNodes.h"
#include "CountingRegion.h"

#include <cache/cachedObjectBuilder.h>
#include <pqPipelineSource.h>

#include <QDebug>
#include <assert.h>
#include <pqOutputPort.h>
#include <vtkSMOutputPort.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMStringVectorProperty.h>


const ExtensionId CountingRegionExtension::ID = "CountinRegionExtension";

void CountingRegionExtension::initialize(Segmentation *seg)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  // Create counting region filter
  assert(!m_init);
  qDebug() << "Creating a new counting region filter";
  m_seg = seg;
  // Configuration of Counting Region interface //TODO: This is too messy...
  VtkParamList args;
  VtkArg arg;
  arg.name = "Input";
  arg.type = INPUT;
  VtkParam param;
  param.first = arg;
  param.second = m_seg->id();
  args.push_back(param);
  m_countingRegion = cob->createFilter("filters", "CountingRegion", args);
  if (!m_countingRegion)
  {
    qDebug() << "Couldn't create Bounding Region Filter";
    assert(false);
  }
  
  m_manager->initializeExtension(this);
  
  m_init = true;
}

void CountingRegionExtension::addInformation(InformationMap& map)
{
  qDebug() << "No extra information provided. This extension modifies visibity property";
}


void CountingRegionExtension::addRepresentations(RepresentationMap& map)
{
  qDebug() << "No extra representation provided";
}

ISegmentationExtension* CountingRegionExtension::clone()
{
  return new CountingRegionExtension(m_manager);
}


void CountingRegionExtension::updateRegions(QList< pqPipelineSource* >& regions)
{
  vtkSMProperty *p;
  vtkSMInputProperty *input;
  m_countingRegion->updatePipeline();
  m_countingRegion->getProxy()->UpdatePropertyInformation();
  qDebug() << regions.size() << "regions updated";
  foreach (pqPipelineSource *region, regions)
  {
    p = m_countingRegion->getProxy()->GetProperty("Regions");
    input = vtkSMInputProperty::SafeDownCast(p);
    input->SetProxy(0,region->getProxy());
  }
  m_countingRegion->updatePipeline();
  m_countingRegion->getProxy()->UpdatePropertyInformation();
  p = m_countingRegion->getProxy()->GetProperty("Discarted");
  assert(p);
  vtkSMIntVectorProperty *discarted = vtkSMIntVectorProperty::SafeDownCast(p);
  int isDiscarted = discarted->GetElement(0);
  m_seg->setVisible(!isDiscarted);
}

