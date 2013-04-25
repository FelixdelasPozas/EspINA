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


const QString MorphologicalInformation::EXTENSION_FILE = "MorphologicalInformation/MorphologicalInformation.csv";

const std::string FILE_VERSION = MorphologicalInformationID.toStdString() + " 1.0\n";
const char SEP = ',';

MorphologicalInformation::ExtensionCache MorphologicalInformation::s_cache;

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
MorphologicalInformation::ExtensionData::ExtensionData()
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
, m_validFeret(false)
{
  m_labelMap = Image2LabelFilterType::New();
  m_labelMap->SetComputeFeretDiameter(false);
}

//------------------------------------------------------------------------
MorphologicalInformation::~MorphologicalInformation()
{
  if (m_segmentation)
  {
    //qDebug() << m_seg->data().toString() << ": Deleting" << MorphologicalInformationID;
    invalidate(m_segmentation);
  }
}

//------------------------------------------------------------------------
ModelItem::ExtId MorphologicalInformation::id()
{
  return MorphologicalInformationID;
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
void MorphologicalInformation::setSegmentation(SegmentationPtr seg)
{
  EspINA::Segmentation::Information::setSegmentation(seg);

  connect(m_segmentation, SIGNAL(outputModified()),
          this, SLOT(invalidate()));

  if (m_segmentation->outputIsModified())
    invalidate();
  else
    initialize();
}

//------------------------------------------------------------------------
QVariant MorphologicalInformation::information(const Segmentation::InfoTag &tag)
{
  bool cached = s_cache.isCached(m_segmentation);
  bool requestedInvalidFeret = cached && (tag == FD && s_cache[m_segmentation].Data.FeretDiameter == -1);

  if (!cached || requestedInvalidFeret)
  {
    if (tag == FD)
      m_labelMap->SetComputeFeretDiameter(true);

    updateInformation();
  }

  ExtensionData &data = s_cache[m_segmentation].Data;

  if (tag == SIZE)
    return data.Size;
  if (tag == PS)
    return data.PhysicalSize;
  if (tag == Cx)
    return data.Centroid[0];
  if (tag == Cy)
    return data.Centroid[1];
  if (tag == Cz)
    return data.Centroid[2];
//   if (info == Rx)
//       return m_Region[0]*spacing[0];
//   if (info == "Region Y")
//       return m_Region[1]*spacing[1];
//   if (info == "Region Z")
//     return m_Region[2]*spacing[2];
  if (tag == BPMx)
    return data.BinaryPrincipalMoments[0];
  if (tag == BPMy)
    return data.BinaryPrincipalMoments[1];
  if (tag == BPMz)
    return data.BinaryPrincipalMoments[2];
  if (tag == BPA00)
    return data.BinaryPrincipalAxes[0][0];
  if (tag == BPA01)
    return data.BinaryPrincipalAxes[0][1];
  if (tag == BPA02)
    return data.BinaryPrincipalAxes[0][2];
  if (tag == BPA10)
    return data.BinaryPrincipalAxes[1][0];
  if (tag == BPA11)
    return data.BinaryPrincipalAxes[1][1];
  if (tag == BPA12)
    return data.BinaryPrincipalAxes[1][2];
  if (tag == BPA20)
    return data.BinaryPrincipalAxes[2][0];
  if (tag == BPA21)
    return data.BinaryPrincipalAxes[2][1];
  if (tag == BPA22)
    return data.BinaryPrincipalAxes[2][2];
  if (tag == FD)
    return data.FeretDiameter;
  if (tag == EEDx)
    return data.EquivalentEllipsoidSize[0];
  if (tag == EEDy)
    return data.EquivalentEllipsoidSize[1];
  if (tag == EEDz)
    return data.EquivalentEllipsoidSize[2];

  qWarning() << MorphologicalInformationID << ":"  << tag << " is not provided";
  return QVariant();
}

//------------------------------------------------------------------------
void MorphologicalInformation::loadCache(QuaZipFile  &file,
                                         const QDir  &tmpDir,
                                         IEspinaModel *model)
{
  QString header(file.readLine());
  if (header.toStdString() == FILE_VERSION)
  {
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
        if ( segmentation->filter()->id()       == fields[0]
          && segmentation->outputId()           == fields[1].toInt()
          && segmentation->filter()->cacheDir() == tmpDir)
        {
          extensionSegmentation = segmentation.data();
        }
        i++;
      }
      if (extensionSegmentation)
      {
        ExtensionData &data = s_cache[extensionSegmentation].Data;

        data.Size = fields[2].toDouble();
        data.PhysicalSize = fields[3].toDouble();

        data.Centroid[0] = fields[4].toDouble();
        data.Centroid[1] = fields[5].toDouble();
        data.Centroid[2] = fields[6].toDouble();

        data.BinaryPrincipalMoments[0] = fields[7].toDouble();
        data.BinaryPrincipalMoments[1] = fields[8].toDouble();
        data.BinaryPrincipalMoments[2] = fields[9].toDouble();

        data.BinaryPrincipalAxes[0][0] = fields[10].toDouble();
        data.BinaryPrincipalAxes[0][1] = fields[11].toDouble();
        data.BinaryPrincipalAxes[0][2] = fields[12].toDouble();
        data.BinaryPrincipalAxes[1][0] = fields[13].toDouble();
        data.BinaryPrincipalAxes[1][1] = fields[14].toDouble();
        data.BinaryPrincipalAxes[1][2] = fields[15].toDouble();
        data.BinaryPrincipalAxes[2][0] = fields[16].toDouble();
        data.BinaryPrincipalAxes[2][1] = fields[17].toDouble();
        data.BinaryPrincipalAxes[2][2] = fields[18].toDouble();

        data.FeretDiameter = fields[19].toDouble();

        data.EquivalentEllipsoidSize[0] = fields[20].toDouble();
        data.EquivalentEllipsoidSize[1] = fields[21].toDouble();
        data.EquivalentEllipsoidSize[2] = fields[22].toDouble();
      } else
      {
        qWarning() << MorphologicalInformationID << "Invalid Cache Entry:" << line;
      }
    };
  }
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  return !seg->hasInformationExtension(MorphologicalInformationID)
      && seg->outputIsModified();
}

//------------------------------------------------------------------------
bool MorphologicalInformation::saveCache(Snapshot &snapshot)
{
  s_cache.purge(invalidData);

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  foreach(SegmentationPtr segmentation, s_cache.keys())
  {
    ExtensionData &data = s_cache[segmentation].Data;

    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    cache << SEP << data.Size;

    cache << SEP << data.PhysicalSize;

    cache << SEP << data.Centroid[0];
    cache << SEP << data.Centroid[1];
    cache << SEP << data.Centroid[2];

    //cache << SEP << s_cache[segmentation].Region[0];
    //cache << SEP << s_cache[segmentation].Region[1];
    //cache << SEP << s_cache[segmentation].Region[2];

    cache << SEP << data.BinaryPrincipalMoments[0];
    cache << SEP << data.BinaryPrincipalMoments[1];
    cache << SEP << data.BinaryPrincipalMoments[2];

    cache << SEP << data.BinaryPrincipalAxes[0][0];
    cache << SEP << data.BinaryPrincipalAxes[0][1];
    cache << SEP << data.BinaryPrincipalAxes[0][2];
    cache << SEP << data.BinaryPrincipalAxes[1][0];
    cache << SEP << data.BinaryPrincipalAxes[1][1];
    cache << SEP << data.BinaryPrincipalAxes[1][2];
    cache << SEP << data.BinaryPrincipalAxes[2][0];
    cache << SEP << data.BinaryPrincipalAxes[2][1];
    cache << SEP << data.BinaryPrincipalAxes[2][2];

    cache << SEP << data.FeretDiameter;

    cache << SEP << data.EquivalentEllipsoidSize[0];
    cache << SEP << data.EquivalentEllipsoidSize[1];
    cache << SEP << data.EquivalentEllipsoidSize[2];

    cache << std::endl;
  }

  snapshot << SnapshotEntry(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//------------------------------------------------------------------------
Segmentation::InformationExtension MorphologicalInformation::clone()
{
  return new MorphologicalInformation();
}

//------------------------------------------------------------------------
void MorphologicalInformation::initialize()
{
  s_cache.markAsClean(m_segmentation);
}

//------------------------------------------------------------------------
void MorphologicalInformation::invalidate(SegmentationPtr segmentation)
{
  if (!segmentation)
    segmentation = m_segmentation;

  if (segmentation)
  {
    //qDebug() << "Invalidate" << m_seg->data().toString() << MorphologicalInformationID;
    s_cache.markAsDirty(segmentation);
  }
}

//------------------------------------------------------------------------
void MorphologicalInformation::updateInformation()
{
  //qDebug() << "Updating" << m_seg->data().toString() << ID;
  //FIXME: devolver NAN m_labelMap->SetInput(m_segmentation->volume()->toITK());
  m_labelMap->Update();
  m_labelMap->Modified();

  LabelMapType *labelMap = m_labelMap->GetOutput();
  labelMap->Update();

  bool validInfo = labelMap->GetNumberOfLabelObjects() == 1;

  if (validInfo)
  {
    m_statistic = labelMap->GetNthLabelObject(0);

    s_cache.markAsClean(m_segmentation);

    ExtensionData &data = s_cache[m_segmentation].Data;

    data.Size         = static_cast<unsigned int>(m_statistic->GetNumberOfPixels());

    data.PhysicalSize = m_statistic->GetPhysicalSize();

    for(int i=0; i<3; i++)
      data.Centroid[i] = m_statistic->GetCentroid()[i];

    for(int i=0; i<3; i++)
      data.BinaryPrincipalMoments[i] = m_statistic->GetPrincipalMoments()[i];

    for(int i=0; i<3; i++)
    {
      for(int j=0; j<3; j++)
        data.BinaryPrincipalAxes[i][j] = m_statistic->GetPrincipalAxes()[i][j];
    }

    for(int i=0; i<3; i++)
      data.EquivalentEllipsoidSize[i] = m_statistic->GetEquivalentEllipsoidDiameter()[i];

    if (m_labelMap->GetComputeFeretDiameter())
      data.FeretDiameter = m_statistic->GetFeretDiameter();
  }
}
