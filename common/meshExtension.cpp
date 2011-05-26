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


#include "meshExtension.h"

#include "cache/cachedObjectBuilder.h"

//DEBUG
#include <QDebug>
#include <assert.h>

//ParaView
#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
#include <vtkSMProxy.h>
#include <pqServer.h>
#include <pqScalarsToColors.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMProxyProperty.h>
#include <pqLookupTableManager.h>
#include <vtkSMPropertyHelper.h>


MeshRepresentation::MeshRepresentation(Segmentation* seg)
: ISegmentationRepresentation(seg)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments contourArgs;
  contourArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  contourArgs.push_back(vtkFilter::Argument("ContourValues",vtkFilter::DOUBLEVECT,"255"));
  vtkFilter *m_contour = cob->createFilter("filters","Contour",contourArgs);
  
  assert(m_contour->numProducts() == 1);
  
  m_rep = new vtkProduct(m_contour->product(0).creator(),m_contour->product(0).portNumber());
}

MeshRepresentation::~MeshRepresentation()
{
  qDebug() << "Deleted Mesh Representation from " << m_seg->id();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep->creator());//vtkProduct default beheaviour doesn't delete its filter
  delete m_rep;
}


pqPipelineSource* MeshRepresentation::pipelineSource()
{
  qDebug() << "Mesh Representation: Invalid pipeline (raw input).";
  return m_rep->creator()->pipelineSource();
}

void MeshRepresentation::render(pqView* view)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();

  pqDataRepresentation *dr = dp->setRepresentationVisibility(m_rep->outputPort(),view,m_seg->visible());
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  assert(rep);
  rep->setRepresentation(2);//SURFACE
    
  vtkSMProxy *repProxy = rep->getProxy();
  
  double rgba[4];
  m_seg->color(rgba);
  vtkSMPropertyHelper(repProxy,"DiffuseColor").Set(rgba,3);
    
  // 	//TODO: Create individual properties?
  // 	// Opacity
  // 	vtkSMDoubleVectorProperty *opacity = vtkSMDoubleVectorProperty::SafeDownCast(
  // 	  rep->getProxy()->GetProperty("Opacity"));
  // 	if (opacity)
  // 	{
    // 	  opacity->SetElements1(0.2); 
  // 	}
      
  repProxy->UpdateVTKObjects();
}


const ExtensionId MeshExtension::ID  = "MeshExtension";

ExtensionId MeshExtension::id()
{
  return ID;
}

void MeshExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
}

void MeshExtension::addInformation(InformationMap& map)
{
  qDebug() << ID << ": No extra information provided.";
}

void MeshExtension::addRepresentations(RepresentationMap& map)
{
   MeshRepresentation *rep = new MeshRepresentation(m_seg);
   map.insert("Mesh", rep);
   qDebug() << ID <<": Mesh Representation Added";
}


ISegmentationExtension* MeshExtension::clone()
{
  return new MeshExtension();
}
