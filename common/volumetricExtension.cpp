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


#include "volumetricExtension.h"

#include "filter.h"

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


VolumetricRepresentation::VolumetricRepresentation(Segmentation* seg)
: ISegmentationRepresentation(seg)
{
}

QString VolumetricRepresentation::id()
{
  return m_seg->id();
}


void VolumetricRepresentation::render(pqView* view)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();

  pqDataRepresentation *dr = dp->setRepresentationVisibility(m_seg->outputPort(),view,m_seg->visible());
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  assert(rep);
  rep->setRepresentation(4);//VOLUME
    
  vtkSMProxy *repProxy = rep->getProxy();
  
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  pqServer *server =  pqApplicationCore::instance()->getActiveServer();
  m_LUT = pqApplicationCore::instance()->getLookupTableManager()->getLookupTable(server,m_seg->taxonomy()->getName(),4,0);
  if (m_LUT)
  {
    vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
      m_LUT->getProxy()->GetProperty("RGBPoints"));
    if (rgbs)
    {
      double rgba[4];
      m_seg->color(rgba);
      double colors[8] = {0,0,0,0,1,rgba[0],rgba[1],rgba[2]};
      rgbs->SetElements(colors);
      }
      m_LUT->getProxy()->UpdateVTKObjects();
    }
    
    vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(repProxy->GetProperty("LookupTable"));
    if (lut)
    {
      lut->SetProxy(0,m_LUT->getProxy());
    }
    
    rep->getProxy()->UpdateVTKObjects();
}

pqPipelineSource* VolumetricRepresentation::pipelineSource()
{
  qDebug() << "Volumetric Representation: Invalid pipeline (raw input).";
  return m_seg->creator()->pipelineSource();
}



const ExtensionId VolumetricExtension::ID  = "01_VolumetricExtension";

ExtensionId VolumetricExtension::id()
{
  return ID;
}

void VolumetricExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
}

void VolumetricExtension::addInformation(InformationMap& map)
{
//   qDebug() << ID << ": No extra information provided.";
}

void VolumetricExtension::addRepresentations(RepresentationMap& map)
{
   VolumetricRepresentation *rep = new VolumetricRepresentation(m_seg);
   map.insert("Volumetric", rep);
//    qDebug() << ID <<": Volumetric Representation Added";
}


ISegmentationExtension* VolumetricExtension::clone()
{
  return new VolumetricExtension();
}
