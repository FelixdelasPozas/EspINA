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

#include "Core/Model/Segmentation.h"

// ITK
#include <itkShapeLabelObject.h>

// Qt
#include <QApplication>
#include <QDebug>

const ModelItemExtension::ExtId MorphologicalExtension::ID = "MorphologicalExtension";

// NOTE: Should it be public?
const ModelItemExtension::InfoTag SIZE = "Size";
const ModelItemExtension::InfoTag PS = "Physical Size";
const ModelItemExtension::InfoTag Cx = "Centroid X";
const ModelItemExtension::InfoTag Cy = "Centroid Y";
const ModelItemExtension::InfoTag Cz = "Centroid Z";
const ModelItemExtension::InfoTag Rx = "Region X";
const ModelItemExtension::InfoTag Ry = "Region Y";
const ModelItemExtension::InfoTag Rz = "Region Z";
const ModelItemExtension::InfoTag BPMx = "Binary Principal Moments X";
const ModelItemExtension::InfoTag BPMy = "Binary Principal Moments Y";
const ModelItemExtension::InfoTag BPMz = "Binary Principal Moments Z";
const ModelItemExtension::InfoTag BPA00 = "Binary Principal Axes (0 0)";
const ModelItemExtension::InfoTag BPA01 = "Binary Principal Axes (0 1)";
const ModelItemExtension::InfoTag BPA02 = "Binary Principal Axes (0 2)";
const ModelItemExtension::InfoTag BPA10 = "Binary Principal Axes (1 0)";
const ModelItemExtension::InfoTag BPA11 = "Binary Principal Axes (1 1)";
const ModelItemExtension::InfoTag BPA12 = "Binary Principal Axes (1 2)";
const ModelItemExtension::InfoTag BPA20 = "Binary Principal Axes (2 0)";
const ModelItemExtension::InfoTag BPA21 = "Binary Principal Axes (2 1)";
const ModelItemExtension::InfoTag BPA22 = "Binary Principal Axes (2 2)";
const ModelItemExtension::InfoTag FD = "Feret Diameter";
const ModelItemExtension::InfoTag EEDx = "Equivalent Ellipsoid Diameter X";
const ModelItemExtension::InfoTag EEDy = "Equivalent Ellipsoid Diameter Y";
const ModelItemExtension::InfoTag EEDz = "Equivalent Ellipsoid Diameter Z";

//TODO: Review values to be used from new ITK version
//------------------------------------------------------------------------
MorphologicalExtension::MorphologicalExtension()
: m_statistic(NULL)
, m_validInfo(false)
, m_Size(-1)
, m_PhysicalSize(-1)
, m_validFeret(false)
, m_FeretDiameter(-1)
{
  m_availableInformations << SIZE;
  m_availableInformations << PS;
  m_availableInformations << Cx << Cy << Cz;
//   m_availableInformations << Rx << Ry << Rz;
  m_availableInformations << BPMx << BPMy << BPMz;
  m_availableInformations << BPA00 << BPA01 << BPA02;
  m_availableInformations << BPA10 << BPA11 << BPA12;
  m_availableInformations << BPA20 << BPA21 << BPA22;
  m_availableInformations << FD;
  m_availableInformations << EEDx << EEDy << EEDz;

  for(int i=0; i<3; i++)
  {
    m_Centroid[i] = -1;
    m_Region[i]   = -1;
    m_BinaryPrincipalMoments[i]  = -1;
    m_EquivalentEllipsoidSize[i] = -1;
  }
  for(int i=0; i<9; i++)
    m_BinaryPrincipalAxes[i] = -1;
}

//------------------------------------------------------------------------
MorphologicalExtension::~MorphologicalExtension()
{
}

//------------------------------------------------------------------------
ModelItemExtension::ExtId MorphologicalExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void MorphologicalExtension::initialize(ModelItem::Arguments args)
{
//   qDebug() << "Converting from ITK to LabelMap";
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_labelMap = Image2LabelFilterType::New();
  m_labelMap->SetComputeFeretDiameter(false);

  QApplication::restoreOverrideCursor();
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

  if (NULL == m_statistic
      || m_seg->volume()->toITK()->GetTimeStamp() > m_labelMap->GetTimeStamp()
      || (info == FD && !m_validFeret)
     )
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
//     qDebug() << "Updating morphological extension";
//     qDebug() << "Volume TS:" << m_seg->volume()->GetTimeStamp().GetMTime();
//     qDebug() << "LabelMap TS:" << m_labelMap->GetTimeStamp().GetMTime();
    if (info == FD)
    {
      m_labelMap->SetComputeFeretDiameter(true);
      m_validFeret = true;
    }
    m_labelMap->SetInput(m_seg->volume()->toITK());
    m_labelMap->Update();
    m_labelMap->Modified();

    LabelMapType *labelMap = m_labelMap->GetOutput();
    labelMap->Update();

    QApplication::restoreOverrideCursor();
    m_validInfo = labelMap->GetNumberOfLabelObjects() == 1;

    if (m_validInfo)
      m_statistic = labelMap->GetNthLabelObject(0);
  }

  if (!m_validInfo)
    return QVariant(-1);

//   EspinaVolume::SpacingType spacing = m_seg->volume()->GetSpacing();
  if (info == SIZE)
      return static_cast<unsigned int>(m_statistic->GetNumberOfPixels());
  if (info == PS)
    return m_statistic->GetPhysicalSize();
  if (info == Cx)
    return m_statistic->GetCentroid()[0];
  if (info == Cy)
    return m_statistic->GetCentroid()[1];
  if (info == Cz)
    return m_statistic->GetCentroid()[2];
//   if (info == Rx)
//       return m_Region[0]*spacing[0];
//   if (info == "Region Y")
//       return m_Region[1]*spacing[1];
//   if (info == "Region Z")
//     return m_Region[2]*spacing[2];
  if (info == BPMx)
      return m_statistic->GetPrincipalMoments()[0];
  if (info == BPMy)
      return m_statistic->GetPrincipalMoments()[1];
  if (info == BPMz)
      return m_statistic->GetPrincipalMoments()[2];
  if (info == BPA00)
      return m_statistic->GetPrincipalAxes()[0][0];
  if (info == BPA01)
      return m_statistic->GetPrincipalAxes()[0][1];
  if (info == BPA02)
      return m_statistic->GetPrincipalAxes()[0][2];
  if (info == BPA10)
      return m_statistic->GetPrincipalAxes()[1][0];
  if (info == BPA11)
      return m_statistic->GetPrincipalAxes()[1][1];
  if (info == BPA12)
      return m_statistic->GetPrincipalAxes()[1][2];
  if (info == BPA20)
      return m_statistic->GetPrincipalAxes()[2][0];
  if (info == BPA21)
      return m_statistic->GetPrincipalAxes()[2][1];
  if (info == BPA22)
      return m_statistic->GetPrincipalAxes()[2][2];
  if (info == FD)
    return m_statistic->GetFeretDiameter();
  if (info == EEDx)
      return m_statistic->GetEquivalentEllipsoidDiameter()[0];
  if (info == EEDy)
      return m_statistic->GetEquivalentEllipsoidDiameter()[1];
  if (info == EEDz)
      return m_statistic->GetEquivalentEllipsoidDiameter()[2];

  qWarning() << ID << ":"  << info << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
SegmentationExtension* MorphologicalExtension::clone()
{
  return new MorphologicalExtension();
}
