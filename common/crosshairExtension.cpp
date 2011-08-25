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

// Debug
#include "espina_debug.h"

// EspINA
#include "sample.h"
#include "filter.h"
#include "cache/cachedObjectBuilder.h"
#include "labelMapExtension.h"

#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
#include <pqPipelineSource.h>

#include <vtkSMPropertyHelper.h>
#include <vtkSMInputProperty.h>
#include <vtkSMProxy.h>
#include <pqView.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSMPVRepresentationProxy.h>

using namespace CrosshairExtension;

//!-----------------------------------------------------------------------
//! CROSSHAIR SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Represent a sample as 3-crossing planes.
//! It supports both 3D and 2D representations

const ISampleRepresentation::RepresentationId SampleRepresentation::ID  = "Crosshairs";

//------------------------------------------------------------------------
SampleRepresentation::SampleRepresentation(Sample* sample)
: ISampleRepresentation(sample)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  assert(m_sample->representation(LabelMapExtension::SampleRepresentation::ID));
  
  m_internalRep = dynamic_cast<LabelMapExtension::SampleRepresentation *>(m_sample->representation(LabelMapExtension::SampleRepresentation::ID));

  for(ViewType plane = VIEW_PLANE_FIRST; plane <= VIEW_PLANE_LAST; plane = ViewType(plane+1))
  {
    vtkFilter::Arguments filterArgs;
    filterArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT, m_internalRep->id()));
    QString mode = QString("%1").arg(5+ plane);
    filterArgs.push_back(vtkFilter::Argument("SliceMode",vtkFilter::INTVECT,mode));
    m_planes[plane] = cob->createFilter("filters", "ImageSlicer", filterArgs);
    m_center[plane] = 0;
  }
  m_center[VIEW_3D] = -1; // there is no single coordinate to refer all 3D image
  
  connect(m_internalRep,SIGNAL(representationUpdated()),this,SLOT(internalRepresentationUpdated()));
}

//------------------------------------------------------------------------
SampleRepresentation::~SampleRepresentation()
{
  //qDebug() << "Deleting Crosshair Representation from " << m_sample->id();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  for(ViewType plane = VIEW_PLANE_FIRST; plane <= VIEW_PLANE_LAST; plane = ViewType(plane+1))
    cob->removeFilter(m_planes[plane]);
}

//------------------------------------------------------------------------
QString SampleRepresentation::id()
{
  assert(false); // We can't use a crosshair as input for another filter
  return "";
}

//------------------------------------------------------------------------
bool exist(vtkActor *actor, vtkActorCollection *collection)
{
  collection->InitTraversal();
  while (vtkActor *oldActor = collection->GetNextActor())
  {
    if (oldActor == actor)
      return true;
  }
  return false;
}

//------------------------------------------------------------------------
vtkActorCollection *findNewActors(vtkActorCollection *before, vtkActorCollection *after)
{
  vtkActorCollection *newActors = vtkActorCollection::New();
  //qDebug() << "Actors before adding rep:" << before->GetNumberOfItems();
  //qDebug() << "Actors after adding rep:" << after->GetNumberOfItems();
  after->InitTraversal();
  while (vtkActor *actor = after->GetNextActor())
  {
    if (!exist(actor, before))
    {
      newActors->AddItem(actor);
    }
  }
  return newActors;
}

//------------------------------------------------------------------------
void copyActors(vtkActorCollection *source, vtkActorCollection *destination)
{
  //qDebug() << "Actors before copying:" << destination->GetNumberOfItems();
  source->InitTraversal();
  while(vtkActor *actor = source->GetNextActor())
  {
    destination->AddItem(actor);
  }
  //qDebug() << "Actors after copying:" << destination->GetNumberOfItems();
  
}

//------------------------------------------------------------------------
void createRepresentation(pqView *view, pqOutputPort *port, int crossHairPlane)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  vtkSMRenderViewProxy* viewProxy = vtkSMRenderViewProxy::SafeDownCast(view->getProxy());
//   vtkActorCollection *rendererActors =   viewProxy->GetRenderer()->GetActors();
//   
//   vtkActorCollection *actorsBeforeRep = vtkActorCollection::New();
//   //qDebug() << "Actors before adding rep:" << rendererActors->GetNumberOfItems();
//   copyActors(rendererActors, actorsBeforeRep);
  
  pqDataRepresentation *dr = dp->setRepresentationVisibility(port,view,true);
  if (crossHairPlane >= 0 && crossHairPlane <= 2)
  {
      pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
      assert(rep);
      rep->setRepresentation(vtkSMPVRepresentationProxy::OUTLINE);
          
      vtkSMProxy *repProxy = rep->getProxy();
      
      double color[4] = {0,0,0.0,1.0};
      color[crossHairPlane] = 1.0;
      vtkSMPropertyHelper(repProxy,"AmbientColor").Set(color,3);
      repProxy->UpdateVTKObjects();
  }
/*
  vtkActorCollection *actorsAfterRep = vtkActorCollection::New();
  rendererActors = viewProxy->GetRenderer()->GetActors();
  //qDebug() << "Actors after adding rep:" << rendererActors->GetNumberOfItems();
  copyActors(rendererActors, actorsAfterRep);
  
  vtkActorCollection *newRendererActors = findNewActors(actorsBeforeRep, actorsAfterRep);
  
  if (isCrossHair)
  {
      newRendererActors->InitTraversal();
      while(vtkActor *actor = newRendererActors->GetNextActor())
      {
	actor->SetPickable(false);
      }
  }
  
  actorsBeforeRep->Delete();
  actorsAfterRep->Delete();
  newRendererActors->Delete();*/
}

//------------------------------------------------------------------------
void SampleRepresentation::render(pqView* view, ViewType type)
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
      bool isCrosshair = (type != plane);
      createRepresentation(view,m_planes[plane]->pipelineSource()->getOutputPort(0),isCrosshair?plane:-1);
    }
  }
}

//------------------------------------------------------------------------
pqPipelineSource* SampleRepresentation::pipelineSource()
{
  assert(false); // We can use a crosshair as input for another filter
  return NULL;
}

//------------------------------------------------------------------------
void SampleRepresentation::setSlice(int slice, ViewType type, bool update)
{
  if (slice == m_center[type])
    return; //Update is not needed
  if (type != VIEW_3D)
  {
    vtkSMPropertyHelper(m_planes[type]->pipelineSource()->getProxy(),"Slice").Set(slice);
    m_planes[type]->pipelineSource()->getProxy()->UpdateVTKObjects();
    m_center[type] = slice;
    if (update)
      emit representationUpdated();
  }
}

//------------------------------------------------------------------------
int SampleRepresentation::slice ( ViewType type )
{
  return m_center[type];
  
  if (type != VIEW_3D)
  {
    int numSlice;
    m_planes[type]->pipelineSource()->getProxy()->UpdateVTKObjects();
    vtkSMPropertyHelper(m_planes[type]->pipelineSource()->getProxy(),"Slice").Get(&numSlice, 1);
    return numSlice;
  }
  return -1; 
}


//------------------------------------------------------------------------
void SampleRepresentation::centerOn(int x, int y, int z)
{
  int extent[6];
  m_sample->extent(extent);
  if (x < extent[0] || extent[1] < x)
    return;
  if (y < extent[2] || extent[3] < y)
    return;
  if (z < extent[4] || extent[5] < z)
    return;
  setSlice(x,VIEW_PLANE_YZ,false);
  setSlice(y,VIEW_PLANE_XZ,false);
  setSlice(z,VIEW_PLANE_XY,false);
  emit representationUpdated();
}

//------------------------------------------------------------------------
void SampleRepresentation::internalRepresentationUpdated()
{
  vtkSMProperty* p;
  vtkSMInputProperty *inputProp;
  
  for(ViewType plane = VIEW_PLANE_FIRST; plane <= VIEW_PLANE_LAST; plane = ViewType(plane+1))
  {
    p = m_planes[plane]->pipelineSource()->getProxy()->GetProperty("Input");
    inputProp = vtkSMInputProperty::SafeDownCast(p);
    inputProp->SetInputConnection(0, m_internalRep->pipelineSource()->getProxy(), 0);
    m_planes[plane]->pipelineSource()->getProxy()->UpdateVTKObjects();
  }
}

//!-----------------------------------------------------------------------
//! CROSSHAIR EXTENSION
//!-----------------------------------------------------------------------
//! Provides:
//! - Crosshair Representation
//------------------------------------------------------------------------
SampleExtension::SampleExtension()
: m_crossRep(NULL)
{
  m_availableRepresentations << SampleRepresentation::ID;
}

//------------------------------------------------------------------------
SampleExtension::~SampleExtension()
{
  if (m_crossRep)
    delete m_crossRep;
}

//------------------------------------------------------------------------
void SampleExtension::initialize(Sample* sample)
{
   EXTENSION_DEBUG(ID << " Initialized");
  m_sample = sample;
  m_crossRep = new SampleRepresentation(m_sample);
}

//------------------------------------------------------------------------
QStringList SampleExtension::dependencies()
{
  QStringList deps;
  deps << LabelMapExtension::ID;
  
  return deps;
}

//------------------------------------------------------------------------
ISampleRepresentation* SampleExtension::representation(QString rep)
{
  if (rep == SampleRepresentation::ID)
    return m_crossRep;
  
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
  return new SampleExtension();
}

//!-----------------------------------------------------------------------
// SampleRepresentation *SampleRepresentation(Sample *sample)
// {
//   return dynamic_cast<SampleRepresentation *>(sample->representation(SampleRepresentation::ID));
// }
