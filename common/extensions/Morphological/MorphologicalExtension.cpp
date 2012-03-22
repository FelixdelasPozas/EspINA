/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "MorphologicalExtension.h"

#include "common/cache/CachedObjectBuilder.h"
#include <common/model/Segmentation.h>

#include <QApplication>

#include <pqPipelineSource.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>


const QString MorphologicalExtension::ID = "MorphologicalExtension";

//------------------------------------------------------------------------
MorphologicalExtension::MorphologicalExtension()
: m_features(NULL)
, m_validFeret(0)
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
//     EXTENSION_DEBUG("Deleted " << ID << " Extension from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_features);
    m_features = NULL;
  }
}

//------------------------------------------------------------------------
QString MorphologicalExtension::id()
{
  return ID;
}


//------------------------------------------------------------------------
void MorphologicalExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  pqFilter::Arguments featuresArgs;
  featuresArgs << pqFilter::Argument("Input", pqFilter::Argument::INPUT, m_seg->volume().id());
  m_features = cob->createFilter("filters","MorphologicalFeatures", featuresArgs);
  Q_ASSERT(m_features);
  m_init = true;
}

//------------------------------------------------------------------------
SegmentationRepresentation* MorphologicalExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  Q_ASSERT(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant MorphologicalExtension::information(QString info) const
{
  if (!m_init)
    return QVariant();

  if (!m_validInfo)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_validInfo = true;
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
//     vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"FeretDiameter").UpdateValueFromServer();
//     vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"FeretDiameter").Get(&m_FeretDiameter,1);
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"EquivalentEllipsoidSize").UpdateValueFromServer();
    vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"EquivalentEllipsoidSize").Get(m_EquivalentEllipsoidSize,3);
    QApplication::restoreOverrideCursor();
  }

  //TODO: Get segmentation's spacing
  double spacing[3] = {1, 1, 1};
//   m_seg->origin()->spacing(spacing);

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
      return m_Region[0]*spacing[0];
  if (info == "Region Y")
      return m_Region[1]*spacing[1];
  if (info == "Region Z")
    return m_Region[2]*spacing[2];
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
  {
    if (!m_validFeret)
    {
      m_validFeret = true;
      int compute = 1;
      QApplication::setOverrideCursor(Qt::WaitCursor);
      vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"ComputeFeret").Set(compute);
      m_features->pipelineSource()->getProxy()->UpdateVTKObjects();
      m_features->pipelineSource()->updatePipeline();
      vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"FeretDiameter").UpdateValueFromServer();
      vtkSMPropertyHelper(m_features->pipelineSource()->getProxy(),"FeretDiameter").Get(&m_FeretDiameter,1);
      QApplication::restoreOverrideCursor();
    }
    return m_FeretDiameter;
  }
  if (info == "Equivalent Ellipsoid Size X")
      return m_EquivalentEllipsoidSize[0];
  if (info == "Equivalent Ellipsoid Size Y")
      return m_EquivalentEllipsoidSize[1];
  if (info == "Equivalent Ellipsoid Size Z")
      return m_EquivalentEllipsoidSize[2];

  qWarning() << ID << ":"  << info << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
SegmentationExtension* MorphologicalExtension::clone()
{
  return new MorphologicalExtension();
}

