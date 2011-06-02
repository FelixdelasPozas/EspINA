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


#include "crosshairExtension.h"

#include "filter.h"
#include "cache/cachedObjectBuilder.h"

#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
#include <pqPipelineSource.h>

//DEBUG
#include <QDebug>
#include <assert.h>
#include <vtkSMPropertyHelper.h>


CrosshairRepresentation::CrosshairRepresentation(Sample* sample): ISampleRepresentation(sample)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  assert(m_sample->representation("02_LabelMap"));

  for(ViewType plane = VIEW_PLANE_FIRST; plane <= VIEW_PLANE_LAST; plane = ViewType(plane+1))
  {
    vtkFilter::Arguments filterArgs;
    filterArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, m_sample->representation("02_LabelMap")->id()));
    QString mode = QString("%1").arg(5+ plane);
    filterArgs.push_back(vtkFilter::Argument("SliceMode",vtkFilter::INTVECT,mode));
    m_planes[plane] = cob->createFilter("filters", "ImageSlicer", filterArgs);
  }
}

CrosshairRepresentation::~CrosshairRepresentation()
{
  qDebug() << "Deleting Crosshair Representation from " << m_sample->id();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  for(ViewType plane = VIEW_PLANE_FIRST; plane <= VIEW_PLANE_LAST; plane = ViewType(plane+1))
    cob->removeFilter(m_planes[plane]);
}

QString CrosshairRepresentation::id()
{
  assert(false); // We can use a crosshair as input for another filter
  return "";
}

void CrosshairRepresentation::render(pqView* view, ViewType type)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  if (type == VIEW_3D)
  {
    dp->setRepresentationVisibility(m_planes[VIEW_PLANE_XY]->pipelineSource()->getOutputPort(0),view,true);
    dp->setRepresentationVisibility(m_planes[VIEW_PLANE_YZ]->pipelineSource()->getOutputPort(0),view,true);
    dp->setRepresentationVisibility(m_planes[VIEW_PLANE_XZ]->pipelineSource()->getOutputPort(0),view,true);
  }
  else
  {
    for(ViewType plane = VIEW_PLANE_FIRST; plane <= VIEW_PLANE_LAST; plane = ViewType(plane+1))
    {
      pqDataRepresentation *dr = dp->setRepresentationVisibility(m_planes[plane]->pipelineSource()->getOutputPort(0),view,true);
      if (type == plane)
	continue;
      pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
      assert(rep);
      rep->setRepresentation(3);
    }
    
  }
}

pqPipelineSource* CrosshairRepresentation::pipelineSource()
{
  assert(false); // We can use a crosshair as input for another filter
  return NULL;
}

void CrosshairRepresentation::setSlice(int slice, ViewType type)
{
  if (type != VIEW_3D)
  {
    vtkSMPropertyHelper(m_planes[type]->pipelineSource()->getProxy(),"Slice").Set(slice);
    emit representationUpdated();
  }
}

void CrosshairRepresentation::centerOn(int x, int y, int z)
{
  setSlice(x,VIEW_PLANE_YZ);
  setSlice(y,VIEW_PLANE_XZ);
  setSlice(z,VIEW_PLANE_XY);
}




const ExtensionId CrosshairExtension::ID = "03_CrosshairExtension";

void CrosshairExtension::initialize(Sample* sample)
{
  m_sample = sample;
}

void CrosshairExtension::addInformation(ISampleExtension::InformationMap& map)
{
  qDebug() << ID << ": No extra information provided.";
}

void CrosshairExtension::addRepresentations(ISampleExtension::RepresentationMap& map)
{
   CrosshairRepresentation *rep = new CrosshairRepresentation(m_sample);
   map.insert("03_Crosshair", rep);
   qDebug() << ID <<": Crosshair Representation Added";
}

ISampleExtension* CrosshairExtension::clone()
{
  return new CrosshairExtension();
}

