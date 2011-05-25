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


#include "volumetricRenderer.h"
#include "products.h"

// Para View
#include <pqApplicationCore.h>
#include <pqDisplayPolicy.h>
#include <pqPipelineRepresentation.h>
#include <vtkSMProxy.h>
#include <pqServer.h>
#include <pqScalarsToColors.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMProxyProperty.h>
#include <pqLookupTableManager.h>

VolumetricRenderer::VolumetricRenderer(QWidget* parent)
: IViewWidget(parent)
{

}


void VolumetricRenderer::updateState(bool checked)
{

}

IViewWidget* VolumetricRenderer::clone()
{

}

void VolumetricRenderer::renderInView(QModelIndex index, pqView* view)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();

  for (int row = 0; row < index.model()->rowCount(index); row++)
  {
    IModelItem *item = static_cast<IModelItem *>(index.child(row,0).internalPointer());
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
     
    /** TO 1UNDO:
    pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->representation("Volumetric")->outputPort(),view,true);
    pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
    assert(rep);
    rep->setRepresentation(4);
    rep->getProxy()->UpdateVTKObjects();
    */
    
    continue;
    /*
    pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->outputPort(),view,true);
    pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
    assert(rep);
    rep->setRepresentation(4);
    
    vtkSMProxy *repProxy = rep->getProxy();
  
    // Get (or create if it doesn't exit) the lut for the segmentations' images
    pqServer *server =  pqApplicationCore::instance()->getActiveServer();
    pqScalarsToColors *segLUT = pqApplicationCore::instance()->getLookupTableManager()->getLookupTable(server,seg->taxonomy()->getName(),4,0);
    if (segLUT)
    {
      //std::cout << "ScalarToColors\n";
      vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
	segLUT->getProxy()->GetProperty("RGBPoints"));
      if (rgbs)
      {
	double rgba[4];
	seg->color(rgba);
	double colors[8] = {0,0,0,0,1,rgba[0],rgba[1],rgba[2]};
	rgbs->SetElements(colors);
      }
      segLUT->getProxy()->UpdateVTKObjects();
    }
    vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(repProxy->GetProperty("LookupTable"));
    if (lut)
    {
      lut->SetProxy(0,segLUT->getProxy());
    }
    
    
    rep->getProxy()->UpdateVTKObjects();
    */
  }
}

