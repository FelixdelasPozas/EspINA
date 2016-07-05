/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "MorphologicalInformation.h"
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Segmentation.h>

// ITK
#include <itkShapeLabelObject.h>
#include <vtkMeshQuality.h>
#include <vtkSmartPointer.h>

// Qt
#include <QApplication>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkMeshQuality.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core;

const SegmentationExtension::Type EspinaExtensions_EXPORT MorphologicalInformation::TYPE = "MorphologicalInformation";

const SegmentationExtension::Key MORPHOLOGICAL_SIZE  = "Size";
const SegmentationExtension::Key MORPHOLOGICAL_PS    = "Physical Size";
const SegmentationExtension::Key MORPHOLOGICAL_Cx    = "Centroid X";
const SegmentationExtension::Key MORPHOLOGICAL_Cy    = "Centroid Y";
const SegmentationExtension::Key MORPHOLOGICAL_Cz    = "Centroid Z";
const SegmentationExtension::Key MORPHOLOGICAL_Rx    = "Region X";
const SegmentationExtension::Key MORPHOLOGICAL_Ry    = "Region Y";
const SegmentationExtension::Key MORPHOLOGICAL_Rz    = "Region Z";
const SegmentationExtension::Key MORPHOLOGICAL_BPMx  = "Binary Principal Moments X";
const SegmentationExtension::Key MORPHOLOGICAL_BPMy  = "Binary Principal Moments Y";
const SegmentationExtension::Key MORPHOLOGICAL_BPMz  = "Binary Principal Moments Z";
const SegmentationExtension::Key MORPHOLOGICAL_BPA00 = "Binary Principal Axes (0 0)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA01 = "Binary Principal Axes (0 1)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA02 = "Binary Principal Axes (0 2)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA10 = "Binary Principal Axes (1 0)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA11 = "Binary Principal Axes (1 1)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA12 = "Binary Principal Axes (1 2)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA20 = "Binary Principal Axes (2 0)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA21 = "Binary Principal Axes (2 1)";
const SegmentationExtension::Key MORPHOLOGICAL_BPA22 = "Binary Principal Axes (2 2)";
const SegmentationExtension::Key MORPHOLOGICAL_FD    = "Feret Diameter";
const SegmentationExtension::Key MORPHOLOGICAL_EEDx  = "Equivalent Ellipsoid Diameter X";
const SegmentationExtension::Key MORPHOLOGICAL_EEDy  = "Equivalent Ellipsoid Diameter Y";
const SegmentationExtension::Key MORPHOLOGICAL_EEDz  = "Equivalent Ellipsoid Diameter Z";
const SegmentationExtension::Key MORPHOLOGICAL_AREA  = "Surface Area";

//TODO: Review values to be used from new ITK version (Elongation & Flatness, Perimeter & Perimeter on border?)
//------------------------------------------------------------------------
MorphologicalInformation::MorphologicalInformation(const SegmentationExtension::InfoCache &cache,
                                                   const State &state)
: SegmentationExtension{cache}
, m_statistic          {nullptr}
{
  m_labelMap = Image2LabelFilterType::New();
  m_labelMap->SetComputeFeretDiameter(false);
}

//------------------------------------------------------------------------
MorphologicalInformation::~MorphologicalInformation()
{
}

//------------------------------------------------------------------------
State MorphologicalInformation::state() const
{
  return State();
}

//------------------------------------------------------------------------
Snapshot MorphologicalInformation::snapshot() const
{
  return Snapshot();
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList MorphologicalInformation::availableInformation() const
{
  InformationKeyList keys;

  for (auto value : {MORPHOLOGICAL_SIZE,
                     MORPHOLOGICAL_PS,
                     MORPHOLOGICAL_Cx, MORPHOLOGICAL_Cy, MORPHOLOGICAL_Cz,
                     MORPHOLOGICAL_BPMx, MORPHOLOGICAL_BPMy, MORPHOLOGICAL_BPMz,
                     MORPHOLOGICAL_BPA00, MORPHOLOGICAL_BPA01, MORPHOLOGICAL_BPA02,
                     MORPHOLOGICAL_BPA10, MORPHOLOGICAL_BPA11, MORPHOLOGICAL_BPA12,
                     MORPHOLOGICAL_BPA20, MORPHOLOGICAL_BPA21, MORPHOLOGICAL_BPA22,
                     MORPHOLOGICAL_FD,
                     MORPHOLOGICAL_EEDx, MORPHOLOGICAL_EEDy, MORPHOLOGICAL_EEDz,
                     MORPHOLOGICAL_AREA})
  {
    keys << createKey(value);
  }

  return keys;
}

//------------------------------------------------------------------------
void MorphologicalInformation::onExtendedItemSet(Segmentation* item)
{
}

//------------------------------------------------------------------------
QVariant MorphologicalInformation::cacheFail(const InformationKey& key) const
{
  if (key.value() == MORPHOLOGICAL_FD)
  {
    QWriteLocker lock(&m_mutex);
    m_labelMap->SetComputeFeretDiameter(true);
  }

  QVariant info;

  if (hasVolumetricData(m_extendedItem->output()) && hasMeshData(m_extendedItem->output()))
  {
    updateInformation();

    if (availableInformation().contains(key))
    {
      info = information(key);
    }
  }

  return info;
}

//------------------------------------------------------------------------
void MorphologicalInformation::updateInformation() const
{
  QWriteLocker lock(&m_mutex);
  Q_ASSERT(hasVolumetricData(m_extendedItem->output()) && hasMeshData(m_extendedItem->output()));

  itkVolumeType::Pointer segVolume = nullptr;

  {
    auto data = readLockVolume(m_extendedItem->output());
    segVolume = data->itkImage();
  }

  bool          validInfo = (segVolume != nullptr);
  LabelMapType *labelMap  = nullptr;

  if (validInfo)
  {
    m_labelMap->SetInput(segVolume);
    m_labelMap->Update();
    m_labelMap->ReleaseDataFlagOn();
    m_labelMap->Modified();

    labelMap = m_labelMap->GetOutput();
    labelMap->Update();

    validInfo = labelMap->GetNumberOfLabelObjects() == 1;
  }

  if (validInfo)
  {
    m_statistic = labelMap->GetNthLabelObject(0);

    updateInfoCache(MORPHOLOGICAL_SIZE, static_cast<int>(m_statistic->GetNumberOfPixels()));

    updateInfoCache(MORPHOLOGICAL_PS, m_statistic->GetPhysicalSize());

    updateInfoCache(MORPHOLOGICAL_Cx, m_statistic->GetCentroid()[0]);
    updateInfoCache(MORPHOLOGICAL_Cy, m_statistic->GetCentroid()[1]);
    updateInfoCache(MORPHOLOGICAL_Cz, m_statistic->GetCentroid()[2]);

    updateInfoCache(MORPHOLOGICAL_BPMx, m_statistic->GetPrincipalMoments()[0]);
    updateInfoCache(MORPHOLOGICAL_BPMy, m_statistic->GetPrincipalMoments()[1]);
    updateInfoCache(MORPHOLOGICAL_BPMz, m_statistic->GetPrincipalMoments()[2]);

    updateInfoCache(MORPHOLOGICAL_BPA00, m_statistic->GetPrincipalAxes()[0][0]);
    updateInfoCache(MORPHOLOGICAL_BPA01, m_statistic->GetPrincipalAxes()[0][1]);
    updateInfoCache(MORPHOLOGICAL_BPA02, m_statistic->GetPrincipalAxes()[0][2]);
    updateInfoCache(MORPHOLOGICAL_BPA10, m_statistic->GetPrincipalAxes()[1][0]);
    updateInfoCache(MORPHOLOGICAL_BPA11, m_statistic->GetPrincipalAxes()[1][1]);
    updateInfoCache(MORPHOLOGICAL_BPA12, m_statistic->GetPrincipalAxes()[1][2]);
    updateInfoCache(MORPHOLOGICAL_BPA20, m_statistic->GetPrincipalAxes()[2][0]);
    updateInfoCache(MORPHOLOGICAL_BPA21, m_statistic->GetPrincipalAxes()[2][1]);
    updateInfoCache(MORPHOLOGICAL_BPA22, m_statistic->GetPrincipalAxes()[2][2]);

    updateInfoCache(MORPHOLOGICAL_EEDx, m_statistic->GetEquivalentEllipsoidDiameter()[0]);
    updateInfoCache(MORPHOLOGICAL_EEDy, m_statistic->GetEquivalentEllipsoidDiameter()[1]);
    updateInfoCache(MORPHOLOGICAL_EEDz, m_statistic->GetEquivalentEllipsoidDiameter()[2]);

    if (m_labelMap->GetComputeFeretDiameter())
    {
      updateInfoCache(MORPHOLOGICAL_FD, m_statistic->GetFeretDiameter());
    }
  }

  vtkSmartPointer<vtkPolyData> mesh = nullptr;
  {
    auto data = readLockMesh(m_extendedItem->output());
    mesh = data->mesh();
  }

  if(mesh != nullptr)
  {
    double area = 0.0;

    for(long long i = 0; i < mesh->GetNumberOfCells(); ++i)
    {
      auto cell = mesh->GetCell(i);
      area += vtkMeshQuality::TriangleArea(cell);
    }

    updateInfoCache(MORPHOLOGICAL_AREA, area);
  }
}
