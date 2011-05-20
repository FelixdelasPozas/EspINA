#include "renderer.h"

#include "interfaces.h"
#include "products.h"

// ParaQ include files
#include "pqDisplayPolicy.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqRenderView.h"
#include "pqPipelineSource.h"
#include <pqPipelineRepresentation.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMPVLookupTableProxy.h>

#include <vtkSMProxyProperty.h>
#include "vtkSMProxyProperty.h"
#include "vtkImageMapToColors.h"
#include <pqPQLookupTableManager.h>
#include <pqScalarsToColors.h>

// Debug
#include <assert.h>
#include <QDebug>
#include <filter.h>

enum Rep3D 
{ POINTS  = 0
, SURFACE = 2
, MESH    = 2
, OUTLINE = 3
, VOLUME  = 4
, SLICE   = 6
, HIDEN   = 100
};


//------------------------------------------------------------------------
MeshRenderer *MeshRenderer::m_singleton(NULL);

QMap<EspinaProduct *,pqPipelineSource *> MeshRenderer::m_contours;

//------------------------------------------------------------------------
IRenderer *MeshRenderer::renderer()
{
  if (!m_singleton)
    m_singleton = new MeshRenderer();
  return m_singleton;
}

/*
void MeshRenderer::hide ( Segmentation* seg, pqRenderView* view )
{
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->outPut(),view,false);
}
*/

//------------------------------------------------------------------------
void MeshRenderer::render(EspinaProduct* actor, pqRenderView* view)
{
  pqPipelineSource *contour = NULL;
  if (m_contours.contains(actor))
    contour = m_contours[actor];
  else
  {
    pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
    contour = ob->createFilter("filters","Contour",actor->creator()->pipelineSource());
    vtkSMProperty *p = contour->getProxy()->GetProperty("ContourValues");
    vtkSMDoubleVectorProperty *values = vtkSMDoubleVectorProperty::SafeDownCast(p);
    values->SetElements1(255);
    m_contours.insert(actor,contour);
  }
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqDataRepresentation *dr = dp->setRepresentationVisibility(
    contour->getOutputPort(0)
    , view
    , actor->visible());
  
  if (!actor->visible())
    return;
  
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  assert(rep);
  
  rep->setRepresentation(SURFACE);
  
  vtkSMProxy *repProxy = rep->getProxy();
  
  
  vtkSMDoubleVectorProperty *color = vtkSMDoubleVectorProperty::SafeDownCast(
    repProxy->GetProperty("DiffuseColor"));
    if (color)
    {
      //TODO: Get colors from segmentation's property
      double rgba[4];
      actor->color(rgba);
      color->SetElements3(rgba[0],rgba[1],rgba[2]);
    }
    
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

/*
void MeshRenderer::renderSelection ( Segmentation* seg, pqRenderView* view )
{

}

void MeshRenderer::renderDiscarted ( Segmentation* seg, pqRenderView* view )
{

}
*/

//------------------------------------------------------------------------
VolumetricRenderer *VolumetricRenderer::m_singleton = NULL;

//------------------------------------------------------------------------
IRenderer *VolumetricRenderer::renderer()
{
  if (!m_singleton)
    m_singleton = new VolumetricRenderer();
  return m_singleton;
}

/*
void VolumeRenderer::hide ( Segmentation* seg, pqRenderView* view )
{
	pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
	pqDataRepresentation *dr = dp->setRepresentationVisibility(seg->outPut(),view,false);
}
*/

//------------------------------------------------------------------------
void VolumetricRenderer::render ( EspinaProduct *actor, pqRenderView * view )
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqDataRepresentation *dr = dp->setRepresentationVisibility(
    actor->outputPort()
    , view
    , actor->visible());
  
  if (!actor->visible())
    return;
  
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  //TODO: Manage style representations
  //qDebug() << "Total Rep: " << view->getRepresentations().size();
  assert(rep);
  rep->setRepresentation(VOLUME);
  
  vtkSMProxy *repProxy = rep->getProxy();
  
  // Get (or create if it doesn't exit) the lut for the segmentations' images
  pqServer *server =  pqApplicationCore::instance()->getActiveServer();
  pqScalarsToColors *segLUT = pqApplicationCore::instance()->getLookupTableManager()
  ->getLookupTable(server,QString("SegmentationsLUT"),4,0);
  if (segLUT)
  {
    //std::cout << "ScalarToColors\n";
    vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
      segLUT->getProxy()->GetProperty("RGBPoints"));
  if (rgbs)
  {
    double rgba[4];
    actor->color(rgba);
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
  
  /*
  // 	vtkSMPVLookupTableProxy *lut = vtkSMPVLookupTableProxy::SafeDownCast(
  // 		  rep->getLookupTableProxy());
  // 	
  // 	if (lut)
  // 	{
    // 		lut->UpdatePropertyInformation();
    // 		vtkSMDoubleVectorProperty *rgbs = vtkSMDoubleVectorProperty::SafeDownCast(
    // 		  lut->GetProperty("RGBPoints"));
    // 		
    // 		if (rgbs)
    // 		{
      // 		  // TODO: Use segmentation's information
      // 		  double colors[8] = {0,0,0,0,1,1,1,1}; 
      // 		  rgbs->SetElements(colors);
      // 		}
      // 	}
      */
  repProxy->UpdateVTKObjects();
}
      
/*
void VolumeRenderer::renderSelection ( Segmentation* seg, pqRenderView* view )
{

}

void VolumeRenderer::renderDiscarted ( Segmentation* seg, pqRenderView* view )
{

}
*/