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

// Debug
#include "espina_debug.h"

// EspINA
#include "filter.h"
#include "segmentation.h"

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

//!-----------------------------------------------------------------------
//! VOLUMETRIC REPRESENTATION
//!-----------------------------------------------------------------------
//! Segmentation's Volumetric representation using LUT 

const ISegmentationRepresentation::RepresentationId VolumetricRepresentation::ID  = "Volumetric";

//------------------------------------------------------------------------
VolumetricRepresentation::VolumetricRepresentation(Segmentation* seg)
: ISegmentationRepresentation(seg)
{
}

//------------------------------------------------------------------------
VolumetricRepresentation::~VolumetricRepresentation()
{
}


//------------------------------------------------------------------------
QString VolumetricRepresentation::id()
{
  return m_seg->id();
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
pqPipelineSource* VolumetricRepresentation::pipelineSource()
{
  return m_seg->creator()->pipelineSource();
}


//!-----------------------------------------------------------------------
//! VOLUMETRIC EXTENSION
//!-----------------------------------------------------------------------
//! Segmentation's Volume representation using LUT

const ExtensionId VolumetricExtension::ID  = "VolumetricExtension";

//------------------------------------------------------------------------
VolumetricExtension::VolumetricExtension()
: m_volRep(NULL)
{
  m_availableRepresentations << VolumetricRepresentation::ID;
}

//------------------------------------------------------------------------
VolumetricExtension::~VolumetricExtension()
{
  if (m_volRep)
    delete m_volRep;
}

//------------------------------------------------------------------------
ExtensionId VolumetricExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void VolumetricExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  m_volRep = new VolumetricRepresentation(seg);
}

//------------------------------------------------------------------------
ISegmentationRepresentation* VolumetricExtension::representation(QString rep)
{
  if (rep == VolumetricRepresentation::ID)
    return m_volRep;

  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant VolumetricExtension::information(QString info)
{
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
ISegmentationExtension* VolumetricExtension::clone()
{
  return new VolumetricExtension();
}
