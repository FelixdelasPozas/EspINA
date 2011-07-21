#include "morphologicalExtension.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "cachedObjectBuilder.h"
#include "segmentation.h"

#include <vtkSMPropertyHelper.h>
#include <pqPipelineSource.h>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <QApplication>

//!-----------------------------------------------------------------------
//! MORPHOLOGICAL EXTENSION
//!-----------------------------------------------------------------------
//! Information Provided:
//! - Centroid

const ExtensionId MorphologicalExtension::ID  = "Morphological";

//------------------------------------------------------------------------
MorphologicalExtension::MorphologicalExtension()
: m_features(NULL)
, m_init(false)
{
  m_availableInformations << "Size";
  m_availableInformations << "Physical Size";
  m_availableInformations << "Centroid X" << "Centroid Y" << "Centroid Z";
  m_availableInformations << "Region X" << "Region Y" << "Region Z"; 
  m_availableInformations << "Binary Principal Moments X" << "Binary Principal Moments Y" << "Binary Principal Moments Z";
  m_availableInformations << "Binary Principal Axes (0 0)" << "Binary Principal Axes (0 1)" << "Binary Principal Axes (0 2)";
  m_availableInformations << "Binary Principal Axes (1 0)" << "Binary Principal Axes (1 1)" << "Binary Principal Axes (1 2)";
  m_availableInformations << "Binary Principal Axes (2 0)" << "Binary Principal Axes (2 1)" << "Binary Principal Axes (2 2)";
  m_availableInformations << "Feret Diameter";
  m_availableInformations << "Equivalent Ellipsoid Size X" << "Equivalent Ellipsoid Size Y" << "Equivalent Ellipsoid Size Z";
}

//------------------------------------------------------------------------
MorphologicalExtension::~MorphologicalExtension()
{
  if (m_features)
  {
    EXTENSION_DEBUG("Deleted " << ID << " Representation from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_features);
    m_features = NULL;
  }
}

//------------------------------------------------------------------------
ExtensionId MorphologicalExtension::id()
{
  return ID;
}


//------------------------------------------------------------------------
void MorphologicalExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments featuresArgs;
  featuresArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  m_features = cob->createFilter("filters","MorphologicalFeatures", featuresArgs);
  assert(m_features);
}

//------------------------------------------------------------------------
ISegmentationRepresentation* MorphologicalExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant MorphologicalExtension::information(QString info)
{
  if (!m_init)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_init = true;
    m_features->pipelineSource()->updatePipeline();
    
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Size").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Size").Get(&m_Size,1);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"PhysicalSize").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"PhysicalSize").Get(&m_PhysicalSize,1);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Centroid").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Centroid").Get(m_Centroid,3);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Region").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"Region").Get(m_Region,3);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"BinaryPrincipalMoments").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"BinaryPrincipalMoments").Get(m_BinaryPrincipalMoments,3);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"BinaryPrincipalAxes").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"BinaryPrincipalAxes").Get(m_BinaryPrincipalAxes,9);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"FeretDiameter").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"FeretDiameter").Get(&m_FeretDiameter,1);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"EquivalentEllipsoidSize").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"EquivalentEllipsoidSize").Get(m_EquivalentEllipsoidSize,3);
    QApplication::restoreOverrideCursor();
  }
  
  if (info == "Size")
      return m_Size;
  if (info == "Physical Size")
      return m_PhysicalSize;
  if (info == "Centroid X")
      return m_Centroid[0];
  if (info == "Centroid Y")
      return m_Centroid[1];
  if (info == "Centroid Z")
      return m_Centroid[2];
  if (info == "Region X")
      return m_Region[0];
  if (info == "Region Y")
      return m_Region[1];
  if (info == "Region Z")
      return m_Region[2];
  if (info == "Binary Principal Moments X")
      return m_BinaryPrincipalMoments[0];
  if (info == "Binary Principal Moments Y")
      return m_BinaryPrincipalMoments[1];
  if (info == "Binary Principal Moments Z")
      return m_BinaryPrincipalMoments[2];
  if (info == "Binary Principal Axes (0 0)")
      return m_BinaryPrincipalAxes[0];
  if (info == "Binary Principal Axes (0 1)")
      return m_BinaryPrincipalAxes[1];
  if (info == "Binary Principal Axes (0 2)")
      return m_BinaryPrincipalAxes[2];
  if (info == "Binary Principal Axes (1 0)")
      return m_BinaryPrincipalAxes[3];
  if (info == "Binary Principal Axes (1 1)")
      return m_BinaryPrincipalAxes[4];
  if (info == "Binary Principal Axes (1 2)")
      return m_BinaryPrincipalAxes[5];
  if (info == "Binary Principal Axes (2 0)")
      return m_BinaryPrincipalAxes[6];
  if (info == "Binary Principal Axes (2 1)")
      return m_BinaryPrincipalAxes[7];
  if (info == "Binary Principal Axes (2 2)")
      return m_BinaryPrincipalAxes[8];
  if (info == "Feret Diameter")
      return m_FeretDiameter;
  if (info == "Equivalent Ellipsoid Size X")
      return m_EquivalentEllipsoidSize[0];
  if (info == "Equivalent Ellipsoid Size Y")
      return m_EquivalentEllipsoidSize[1];
  if (info == "Equivalent Ellipsoid Size Z")
      return m_EquivalentEllipsoidSize[2];
  
 
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
ISegmentationExtension* MorphologicalExtension::clone()
{
  return new MorphologicalExtension();
}

