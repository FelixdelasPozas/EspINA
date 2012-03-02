#include "AppositionPlaneExtension.h"

// Debug
#include <espina_debug.h>

// EspINA
#include <cache/cachedObjectBuilder.h>
#include <segmentation.h>
#include <sample.h>


#include <vtkSMPropertyHelper.h>
#include <pqPipelineSource.h>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <QApplication>
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineRepresentation.h>
#include <vtkSMPVRepresentationProxy.h>

///-----------------------------------------------------------------------
/// APPOSITION PLANE REPRESENTATION
///-----------------------------------------------------------------------
/// Segmentation's Apposition Plane representation using 
///  vtkAppositionPlane

const ISegmentationRepresentation::RepresentationId 
  AppositionPlaneRepresentation::ID = "AppositionPlane";

//------------------------------------------------------------------------
AppositionPlaneRepresentation::AppositionPlaneRepresentation(Segmentation* seg)
: ISegmentationRepresentation(seg)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments appPlaneArgs;
  appPlaneArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  vtkFilter *appPlane = cob->createFilter("filters","AppositionPlane",appPlaneArgs);
  
  assert(appPlane->numProducts() == 1);
  
  m_rep = new vtkProduct(appPlane->product(0).creator(),appPlane->product(0).portNumber());
}

  
//------------------------------------------------------------------------
AppositionPlaneRepresentation::~AppositionPlaneRepresentation()
{
  EXTENSION_DEBUG("Deleted " << ID << " Representation from " << m_seg->id());
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_rep->creator());//vtkProduct default beheaviour doesn't delete its filter
  delete m_rep;
}

//------------------------------------------------------------------------
QString AppositionPlaneRepresentation::id()
{
  return m_rep->creator()->id() + ":0";
}

//------------------------------------------------------------------------
void AppositionPlaneRepresentation::render(pqView* view)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  
  pqDataRepresentation *dr = 
    dp->setRepresentationVisibility(m_rep->outputPort(),view,m_seg->visible());
    
  if (!dr)
    return;
  
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  assert(rep);
  rep->setRepresentation("Surface");
  
  vtkSMProxy *repProxy = rep->getProxy();
  
  double color[4] = {1,0,0,1};
  double rgba[4];
  rgba[3] = 1;
//   m_seg->color(color);
  bool isSelected = m_seg->isSelected();
  for(int c=0; c<3; c++)
  {
    rgba[c] = color[c]*(isSelected?1:0.7);
  }
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

//------------------------------------------------------------------------
pqPipelineSource* AppositionPlaneRepresentation::pipelineSource()
{
  return m_rep->creator()->pipelineSource();
}

//------------------------------------------------------------------------
void AppositionPlaneRepresentation::requestUpdate(bool force)
{

}



///-----------------------------------------------------------------------
/// APPOSITION PLANE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - AS Area
/// - AS Perimeter

const ExtensionId AppositionPlaneExtension::ID = "AppositionPlaneExtension";

//------------------------------------------------------------------------
AppositionPlaneExtension::AppositionPlaneExtension()
: m_features(NULL)
, m_init(false)
{
  m_availableRepresentations << AppositionPlaneRepresentation::ID;
  m_availableInformations << "AS Area" << "AS Perimeter";
}

//------------------------------------------------------------------------
AppositionPlaneExtension::~AppositionPlaneExtension()
{
  if (m_features)
  {
    EXTENSION_DEBUG("Deleted " << ID << " from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_features);
    m_features = NULL;
  }
}

//------------------------------------------------------------------------
ExtensionId AppositionPlaneExtension::id()
{
  return ID;
}


//------------------------------------------------------------------------
void AppositionPlaneExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  seg->creator()->pipelineSource()->updatePipeline();
  
  m_planeRep = new AppositionPlaneRepresentation(seg);
  // m_planeRep->pipelineSource()->updatePipeline();
  
  vtkFilter::Arguments featuresArgs;
  featuresArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_planeRep->id()));
  m_features = cob->createFilter("filters","AppositionPlaneFeatures", featuresArgs);
  assert(m_features);
}

//------------------------------------------------------------------------
ISegmentationRepresentation* AppositionPlaneExtension::representation(QString rep)
{
  if (rep == AppositionPlaneRepresentation::ID)
    return m_planeRep;
  
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant AppositionPlaneExtension::information(QString info)
{
  if (!m_init)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_init = true;
    m_features->pipelineSource()->updatePipeline();
    
    vtkSMProxy *proxy = m_features->pipelineSource()->getProxy();
    
    vtkSMPropertyHelper(proxy, "Area").UpdateValueFromServer();
    vtkSMPropertyHelper(proxy, "Area").Get(&m_Area, 1);
    vtkSMPropertyHelper(proxy, "Perimeter").UpdateValueFromServer();
    vtkSMPropertyHelper(proxy, "Perimeter").Get(&m_Perimeter, 1);
    QApplication::restoreOverrideCursor();
  }

  double spacing[3];
  m_seg->origin()->spacing(spacing);
  
  if (info == "AS Area")
      return m_Area;
  if (info == "AS Perimeter")
      return m_Perimeter;
 
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
ISegmentationExtension* AppositionPlaneExtension::clone()
{
  return new AppositionPlaneExtension();
}