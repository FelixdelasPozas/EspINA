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


#include "MorphologicalInformation.h"

#include "Core/Model/Segmentation.h"
#include <Core/Model/EspinaModel.h>

// ITK
#include <itkShapeLabelObject.h>

// Qt
#include <QApplication>
#include <QDebug>

using namespace EspINA;

const ModelItem::ExtId MorphologicalInformation::ID = "MorphologicalExtension";

const QString MorphologicalInformation::EXTENSION_FILE = "MorphologicalInformation/MorphologicalInformation.csv";

const std::string FILE_VERSION = MorphologicalInformation::ID.toStdString() + " 1.0\n";
const char SEP = ',';

QMap<SegmentationPtr, MorphologicalInformation::CacheEntry> MorphologicalInformation::s_cache;

// NOTE: Should it be public?
const Segmentation::InfoTag SIZE  = "Size";
const Segmentation::InfoTag PS    = "Physical Size";
const Segmentation::InfoTag Cx    = "Centroid X";
const Segmentation::InfoTag Cy    = "Centroid Y";
const Segmentation::InfoTag Cz    = "Centroid Z";
const Segmentation::InfoTag Rx    = "Region X";
const Segmentation::InfoTag Ry    = "Region Y";
const Segmentation::InfoTag Rz    = "Region Z";
const Segmentation::InfoTag BPMx  = "Binary Principal Moments X";
const Segmentation::InfoTag BPMy  = "Binary Principal Moments Y";
const Segmentation::InfoTag BPMz  = "Binary Principal Moments Z";
const Segmentation::InfoTag BPA00 = "Binary Principal Axes (0 0)";
const Segmentation::InfoTag BPA01 = "Binary Principal Axes (0 1)";
const Segmentation::InfoTag BPA02 = "Binary Principal Axes (0 2)";
const Segmentation::InfoTag BPA10 = "Binary Principal Axes (1 0)";
const Segmentation::InfoTag BPA11 = "Binary Principal Axes (1 1)";
const Segmentation::InfoTag BPA12 = "Binary Principal Axes (1 2)";
const Segmentation::InfoTag BPA20 = "Binary Principal Axes (2 0)";
const Segmentation::InfoTag BPA21 = "Binary Principal Axes (2 1)";
const Segmentation::InfoTag BPA22 = "Binary Principal Axes (2 2)";
const Segmentation::InfoTag FD    = "Feret Diameter";
const Segmentation::InfoTag EEDx  = "Equivalent Ellipsoid Diameter X";
const Segmentation::InfoTag EEDy  = "Equivalent Ellipsoid Diameter Y";
const Segmentation::InfoTag EEDz  = "Equivalent Ellipsoid Diameter Z";

//------------------------------------------------------------------------
MorphologicalInformation::CacheEntry::CacheEntry()
: Size(-1)
, PhysicalSize(-1)
, FeretDiameter(-1)
{
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

//TODO: Review values to be used from new ITK version
//------------------------------------------------------------------------
MorphologicalInformation::MorphologicalInformation()
: m_statistic(NULL)
, m_validInfo(false)
{
  m_labelMap = Image2LabelFilterType::New();
  m_labelMap->SetComputeFeretDiameter(false);
}

//------------------------------------------------------------------------
MorphologicalInformation::~MorphologicalInformation()
{
  qDebug() << "Deleting Morphological Extension";
}

//------------------------------------------------------------------------
ModelItem::ExtId MorphologicalInformation::id()
{
  return ID;
}

//------------------------------------------------------------------------
void MorphologicalInformation::initialize(ModelItem::Arguments args)
{
  //qDebug() << "Initialize" << m_seg->data().toString() << ID;

}

//------------------------------------------------------------------------
Segmentation::InfoTagList MorphologicalInformation::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << SIZE;
  tags << PS;
  tags << Cx << Cy << Cz;
//   tags << Rx << Ry << Rz;
  tags << BPMx << BPMy << BPMz;
  tags << BPA00 << BPA01 << BPA02;
  tags << BPA10 << BPA11 << BPA12;
  tags << BPA20 << BPA21 << BPA22;
  tags << FD;
  tags << EEDx << EEDy << EEDz;

  return tags;
}

//------------------------------------------------------------------------
QVariant MorphologicalInformation::information(const Segmentation::InfoTag &tag)
{
  bool cached = s_cache.contains(m_seg);
  bool outdatedVolume = (m_seg->volume()->toITK()->GetTimeStamp() > m_labelMap->GetTimeStamp());
  bool requestedInvalidFeret = cached && (tag == FD && s_cache[m_seg].FeretDiameter == -1);
  //TODO 2013-01-22: Doesn't work with undo/redo 
  if (!cached || outdatedVolume || requestedInvalidFeret)
  {
    if (tag == FD)
      m_labelMap->SetComputeFeretDiameter(true);

    updateInformation();
  }

  if (tag == SIZE)
    return s_cache[m_seg].Size;
  if (tag == PS)
    return s_cache[m_seg].PhysicalSize;
  if (tag == Cx)
    return s_cache[m_seg].Centroid[0];
  if (tag == Cy)
    return s_cache[m_seg].Centroid[1];
  if (tag == Cz)
    return s_cache[m_seg].Centroid[2];
//   if (info == Rx)
//       return m_Region[0]*spacing[0];
//   if (info == "Region Y")
//       return m_Region[1]*spacing[1];
//   if (info == "Region Z")
//     return m_Region[2]*spacing[2];
  if (tag == BPMx)
    return s_cache[m_seg].BinaryPrincipalMoments[0];
  if (tag == BPMy)
    return s_cache[m_seg].BinaryPrincipalMoments[1];
  if (tag == BPMz)
    return s_cache[m_seg].BinaryPrincipalMoments[2];
  if (tag == BPA00)
    return s_cache[m_seg].BinaryPrincipalAxes[0][0];
  if (tag == BPA01)
    return s_cache[m_seg].BinaryPrincipalAxes[0][1];
  if (tag == BPA02)
    return s_cache[m_seg].BinaryPrincipalAxes[0][2];
  if (tag == BPA10)
    return s_cache[m_seg].BinaryPrincipalAxes[1][0];
  if (tag == BPA11)
    return s_cache[m_seg].BinaryPrincipalAxes[1][1];
  if (tag == BPA12)
    return s_cache[m_seg].BinaryPrincipalAxes[1][2];
  if (tag == BPA20)
    return s_cache[m_seg].BinaryPrincipalAxes[2][0];
  if (tag == BPA21)
    return s_cache[m_seg].BinaryPrincipalAxes[2][1];
  if (tag == BPA22)
    return s_cache[m_seg].BinaryPrincipalAxes[2][2];
  if (tag == FD)
    return s_cache[m_seg].FeretDiameter;
  if (tag == EEDx)
    return s_cache[m_seg].EquivalentEllipsoidSize[0];
  if (tag == EEDy)
    return s_cache[m_seg].EquivalentEllipsoidSize[1];
  if (tag == EEDz)
    return s_cache[m_seg].EquivalentEllipsoidSize[2];

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
bool MorphologicalInformation::loadCache(QuaZipFile  &file,
                                         const QDir  &tmpDir,
                                         EspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() != FILE_VERSION)
    return false;

  char buffer[1024];
  while (file.readLine(buffer, sizeof(buffer)) > 0)
  {
    QString line(buffer);
    QStringList fields = line.split(SEP);

    SegmentationPtr extensionSegmentation = NULL;
    int i = 0;
    while (!extensionSegmentation && i < model->segmentations().size())
    {
      SegmentationSPtr segmentation = model->segmentations()[i];
      if ( segmentation->filter()->id()  == fields[0]
        && segmentation->outputId()         == fields[1].toInt()
        && segmentation->filter()->tmpDir() == tmpDir)
      {
        extensionSegmentation = segmentation.data();
      }
      i++;
    }
    //TODO: Remove extensions when removed inside undo commands
    //Q_ASSERT(extensionSegmentation);

    if (extensionSegmentation)
    {
      s_cache[extensionSegmentation].Size = fields[2].toDouble();

      s_cache[extensionSegmentation].PhysicalSize = fields[3].toDouble();

      s_cache[extensionSegmentation].Centroid[0] = fields[4].toDouble();
      s_cache[extensionSegmentation].Centroid[1] = fields[5].toDouble();
      s_cache[extensionSegmentation].Centroid[2] = fields[6].toDouble();

      s_cache[extensionSegmentation].BinaryPrincipalMoments[0] = fields[7].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalMoments[1] = fields[8].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalMoments[2] = fields[9].toDouble();

      s_cache[extensionSegmentation].BinaryPrincipalAxes[0][0] = fields[10].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[0][1] = fields[11].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[0][2] = fields[12].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[1][0] = fields[13].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[1][1] = fields[14].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[1][2] = fields[15].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[2][0] = fields[16].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[2][1] = fields[17].toDouble();
      s_cache[extensionSegmentation].BinaryPrincipalAxes[2][2] = fields[18].toDouble();

      s_cache[extensionSegmentation].FeretDiameter = fields[19].toDouble();

      s_cache[extensionSegmentation].EquivalentEllipsoidSize[0] = fields[20].toDouble();
      s_cache[extensionSegmentation].EquivalentEllipsoidSize[1] = fields[21].toDouble();
      s_cache[extensionSegmentation].EquivalentEllipsoidSize[2] = fields[22].toDouble();
    }
  };


  return true;
}

//------------------------------------------------------------------------
bool MorphologicalInformation::saveCache(CacheList &cacheList)
{
  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  SegmentationPtr segmentation;
  foreach(segmentation, s_cache.keys())
  {
    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    cache << SEP << s_cache[segmentation].Size;

    cache << SEP << s_cache[segmentation].PhysicalSize;

    cache << SEP << s_cache[segmentation].Centroid[0];
    cache << SEP << s_cache[segmentation].Centroid[1];
    cache << SEP << s_cache[segmentation].Centroid[2];

    //cache << SEP << s_cache[segmentation].Region[0];
    //cache << SEP << s_cache[segmentation].Region[1];
    //cache << SEP << s_cache[segmentation].Region[2];

    cache << SEP << s_cache[segmentation].BinaryPrincipalMoments[0];
    cache << SEP << s_cache[segmentation].BinaryPrincipalMoments[1];
    cache << SEP << s_cache[segmentation].BinaryPrincipalMoments[2];

    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[0][0];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[0][1];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[0][2];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[1][0];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[1][1];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[1][2];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[2][0];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[2][1];
    cache << SEP << s_cache[segmentation].BinaryPrincipalAxes[2][2];

    cache << SEP << s_cache[segmentation].FeretDiameter;

    cache << SEP << s_cache[segmentation].EquivalentEllipsoidSize[0];
    cache << SEP << s_cache[segmentation].EquivalentEllipsoidSize[1];
    cache << SEP << s_cache[segmentation].EquivalentEllipsoidSize[2];

    cache << std::endl;
  }

  cacheList << QPair<QString, QByteArray>(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//------------------------------------------------------------------------
Segmentation::InformationExtension MorphologicalInformation::clone()
{
  return new MorphologicalInformation();
}

//------------------------------------------------------------------------
void MorphologicalInformation::updateInformation()
{
  qDebug() << "Updating" << m_seg->data().toString() << ID;
  m_labelMap->SetInput(m_seg->volume()->toITK());
  m_labelMap->Update();
  m_labelMap->Modified();

  LabelMapType *labelMap = m_labelMap->GetOutput();
  labelMap->Update();

  m_validInfo = labelMap->GetNumberOfLabelObjects() == 1;

  if (m_validInfo)
  {
    m_statistic = labelMap->GetNthLabelObject(0);

    s_cache[m_seg].Size         = static_cast<unsigned int>(m_statistic->GetNumberOfPixels());

    s_cache[m_seg].PhysicalSize = m_statistic->GetPhysicalSize();

    for(int i=0; i<3; i++)
      s_cache[m_seg].Centroid[i] = m_statistic->GetCentroid()[i];

    for(int i=0; i<3; i++)
      s_cache[m_seg].BinaryPrincipalMoments[i] = m_statistic->GetPrincipalMoments()[i];

    for(int i=0; i<3; i++)
    {
      for(int j=0; j<3; j++)
        s_cache[m_seg].BinaryPrincipalAxes[i][j] = m_statistic->GetPrincipalAxes()[i][j];
    }

    for(int i=0; i<3; i++)
      s_cache[m_seg].EquivalentEllipsoidSize[i] = m_statistic->GetEquivalentEllipsoidDiameter()[i];

    if (m_labelMap->GetComputeFeretDiameter())
      s_cache[m_seg].FeretDiameter = m_statistic->GetFeretDiameter();
  }
}