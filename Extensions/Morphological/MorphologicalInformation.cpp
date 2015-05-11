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
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Segmentation.h>

// ITK
#include <itkShapeLabelObject.h>

// Qt
#include <QApplication>
#include <QDebug>

using namespace ESPINA;

const SegmentationExtension::Type EspinaExtensions_EXPORT MorphologicalInformation::TYPE = "MorphologicalInformation";

// NOTE: Should it be public?
const SegmentationExtension::InfoTag MORPHOLOGICAL_SIZE  = "Size";
const SegmentationExtension::InfoTag MORPHOLOGICAL_PS    = "Physical Size";
const SegmentationExtension::InfoTag MORPHOLOGICAL_Cx    = "Centroid X";
const SegmentationExtension::InfoTag MORPHOLOGICAL_Cy    = "Centroid Y";
const SegmentationExtension::InfoTag MORPHOLOGICAL_Cz    = "Centroid Z";
const SegmentationExtension::InfoTag MORPHOLOGICAL_Rx    = "Region X";
const SegmentationExtension::InfoTag MORPHOLOGICAL_Ry    = "Region Y";
const SegmentationExtension::InfoTag MORPHOLOGICAL_Rz    = "Region Z";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPMx  = "Binary Principal Moments X";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPMy  = "Binary Principal Moments Y";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPMz  = "Binary Principal Moments Z";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA00 = "Binary Principal Axes (0 0)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA01 = "Binary Principal Axes (0 1)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA02 = "Binary Principal Axes (0 2)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA10 = "Binary Principal Axes (1 0)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA11 = "Binary Principal Axes (1 1)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA12 = "Binary Principal Axes (1 2)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA20 = "Binary Principal Axes (2 0)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA21 = "Binary Principal Axes (2 1)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_BPA22 = "Binary Principal Axes (2 2)";
const SegmentationExtension::InfoTag MORPHOLOGICAL_FD    = "Feret Diameter";
const SegmentationExtension::InfoTag MORPHOLOGICAL_EEDx  = "Equivalent Ellipsoid Diameter X";
const SegmentationExtension::InfoTag MORPHOLOGICAL_EEDy  = "Equivalent Ellipsoid Diameter Y";
const SegmentationExtension::InfoTag MORPHOLOGICAL_EEDz  = "Equivalent Ellipsoid Diameter Z";

//TODO: Review values to be used from new ITK version
//TODO: Make thread safe
//------------------------------------------------------------------------
MorphologicalInformation::MorphologicalInformation(const SegmentationExtension::InfoCache &cache,
                                                   const State &state)
: SegmentationExtension{cache}
, m_statistic          {nullptr}
, m_validFeret         {false}
, Size                 {-1}
, PhysicalSize         {-1}
, FeretDiameter        {-1}
{
  m_labelMap = Image2LabelFilterType::New();
  m_labelMap->SetComputeFeretDiameter(false);

  for(int i=0; i<3; i++)
  {
    Centroid[i] = -1;
    //Region[i]   = -1;
    BinaryPrincipalMoments[i]  = -1;
    EquivalentEllipsoidSize[i] = -1;
  }
  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
      BinaryPrincipalAxes[i][j] = -1;
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
SegmentationExtension::InfoTagList MorphologicalInformation::availableInformations() const
{
  InfoTagList tags;

  tags << MORPHOLOGICAL_SIZE;
  tags << MORPHOLOGICAL_PS;
  tags << MORPHOLOGICAL_Cx << MORPHOLOGICAL_Cy << MORPHOLOGICAL_Cz;
  tags << MORPHOLOGICAL_BPMx << MORPHOLOGICAL_BPMy << MORPHOLOGICAL_BPMz;
  tags << MORPHOLOGICAL_BPA00 << MORPHOLOGICAL_BPA01 << MORPHOLOGICAL_BPA02;
  tags << MORPHOLOGICAL_BPA10 << MORPHOLOGICAL_BPA11 << MORPHOLOGICAL_BPA12;
  tags << MORPHOLOGICAL_BPA20 << MORPHOLOGICAL_BPA21 << MORPHOLOGICAL_BPA22;
  tags << MORPHOLOGICAL_FD;
  tags << MORPHOLOGICAL_EEDx << MORPHOLOGICAL_EEDy << MORPHOLOGICAL_EEDz;

  return tags;
}

//------------------------------------------------------------------------
void MorphologicalInformation::onExtendedItemSet(Segmentation* item)
{
}

//------------------------------------------------------------------------
QVariant MorphologicalInformation::cacheFail(const QString& tag) const
{
  if (tag == MORPHOLOGICAL_FD)
  {
    m_labelMap->SetComputeFeretDiameter(true);
  }

  QVariant info;

  if (hasVolumetricData(m_extendedItem->output()))
  {
    updateInformation();

    if (availableInformations().contains(tag))
    {
      info = information(tag);
    }
  }

  return info;
}

//------------------------------------------------------------------------
void MorphologicalInformation::updateInformation() const
{
//   qDebug() << "Updating" << m_seg->data().toString() << ID;
  Q_ASSERT(hasVolumetricData(m_extendedItem->output()));

  auto segVolume = readLockVolume(m_extendedItem->output());

  bool          validInfo = !segVolume.isNull();
  LabelMapType *labelMap  = nullptr;

  if (validInfo)
  {
    m_labelMap->SetInput(segVolume->itkImage());
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
}
