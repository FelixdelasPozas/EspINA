#include "renderer.h"

#include "segmentation.h"

// Standard
#include <assert.h>

// ParaQ include files
#include "pqDisplayPolicy.h"
#include "pqApplicationCore.h"
#include "pqRenderView.h"
#include <pqPipelineRepresentation.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMPVLookupTableProxy.h>

#include <QDebug>

enum Rep3D 
{
	POINTS = 0
	, SURFACE = 2
	, MESH = 2
	, OUTLINE = 3
	, VOLUME = 4
	, SLICE = 6
	, HIDEN = 100
};

MeshRenderer *MeshRenderer::m_singleton = NULL;

void MeshRenderer::hide ( Segmentation* seg, pqRenderView* view )
{
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->data(),view,false);
}


void MeshRenderer::render( Segmentation* seg, pqRenderView* view)
{
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->data(),view,true);
	pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
	
	assert(rep);
	
	rep->setRepresentation(SURFACE);
	
	vtkSMProxy *repProxy = rep->getProxy();
	
	vtkSMDoubleVectorProperty *color = vtkSMDoubleVectorProperty::SafeDownCast(
	  repProxy->GetProperty("DiffuseColor"));
	if (color)
	{
	  //TODO: Get colors from segmentation's property
	  color->SetElements3(0,1,0);
	}
	
	//TODO: Create individual properties?
	// Opacity
	vtkSMDoubleVectorProperty *opacity = vtkSMDoubleVectorProperty::SafeDownCast(
	  rep->getProxy()->GetProperty("Opacity"));
	if (opacity)
	{
	  opacity->SetElements1(0.2); 
	}
	
	repProxy->UpdateVTKObjects();
}

void MeshRenderer::renderSelection ( Segmentation* seg, pqRenderView* view )
{

}

void MeshRenderer::renderDiscarted ( Segmentation* seg, pqRenderView* view )
{

}


Renderer *MeshRenderer::renderer()
{
	if (!m_singleton)
		m_singleton = new MeshRenderer();
	return m_singleton;
}



VolumeRenderer *VolumeRenderer::m_singleton = NULL;

Renderer *VolumeRenderer::renderer()
{
	if (!m_singleton)
		m_singleton = new VolumeRenderer();
	return m_singleton;
}


void VolumeRenderer::hide ( Segmentation* seg, pqRenderView* view )
{
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->data(),view,false);
}


void VolumeRenderer::render ( Segmentation* seg, pqRenderView* view )
{
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->data(),view,true);
	pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
	//qDebug() << "Total Rep: " << view->getRepresentations().size();
	
	assert(rep);
	
	rep->setRepresentation(VOLUME);
	
	vtkSMProxy *repProxy = rep->getProxy();
	
	vtkSMPVLookupTableProxy *lut = vtkSMPVLookupTableProxy::SafeDownCast(
		  rep->getLookupTableProxy());
	
	if (lut)
	{
		lut->UpdatePropertyInformation();
		vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
		  lut->GetProperty("RGBPoints"));
		
		if (rgbs)
		{
		  // TODO: Use segmentation's information
		  double colors[8] = {0,0,0,0,1,1,1,1}; 
		  rgbs->SetElements(colors);
		}
	}
	repProxy->UpdateVTKObjects();
}

void VolumeRenderer::renderSelection ( Segmentation* seg, pqRenderView* view )
{

}

void VolumeRenderer::renderDiscarted ( Segmentation* seg, pqRenderView* view )
{

}
