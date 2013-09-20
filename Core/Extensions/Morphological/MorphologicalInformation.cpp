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
#include <Core/Model/Filter.h>

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
const Segmentation::InfoTag MORPHOLOGICAL_SIZE  = "Size";
const Segmentation::InfoTag MORPHOLOGICAL_PS    = "Physical Size";
const Segmentation::InfoTag MORPHOLOGICAL_Cx    = "Centroid X";
const Segmentation::InfoTag MORPHOLOGICAL_Cy    = "Centroid Y";
const Segmentation::InfoTag MORPHOLOGICAL_Cz    = "Centroid Z";
const Segmentation::InfoTag MORPHOLOGICAL_Rx    = "Region X";
const Segmentation::InfoTag MORPHOLOGICAL_Ry    = "Region Y";
const Segmentation::InfoTag MORPHOLOGICAL_Rz    = "Region Z";
const Segmentation::InfoTag MORPHOLOGICAL_BPMx  = "Binary Principal Moments X";
const Segmentation::InfoTag MORPHOLOGICAL_BPMy  = "Binary Principal Moments Y";
const Segmentation::InfoTag MORPHOLOGICAL_BPMz  = "Binary Principal Moments Z";
const Segmentation::InfoTag MORPHOLOGICAL_BPA00 = "Binary Principal Axes (0 0)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA01 = "Binary Principal Axes (0 1)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA02 = "Binary Principal Axes (0 2)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA10 = "Binary Principal Axes (1 0)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA11 = "Binary Principal Axes (1 1)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA12 = "Binary Principal Axes (1 2)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA20 = "Binary Principal Axes (2 0)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA21 = "Binary Principal Axes (2 1)";
const Segmentation::InfoTag MORPHOLOGICAL_BPA22 = "Binary Principal Axes (2 2)";
const Segmentation::InfoTag MORPHOLOGICAL_FD    = "Feret Diameter";
const Segmentation::InfoTag MORPHOLOGICAL_EEDx  = "Equivalent Ellipsoid Diameter X";
const Segmentation::InfoTag MORPHOLOGICAL_EEDy  = "Equivalent Ellipsoid Diameter Y";
const Segmentation::InfoTag MORPHOLOGICAL_EEDz  = "Equivalent Ellipsoid Diameter Z";

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

  tags << MORPHOLOGICAL_SIZE;
  tags << MORPHOLOGICAL_PS;
  tags << MORPHOLOGICAL_Cx << MORPHOLOGICAL_Cy << MORPHOLOGICAL_Cz;
//   tags << MORPHOLOGICAL_Rx << MORPHOLOGICAL_Ry << MORPHOLOGICAL_Rz;
  tags << MORPHOLOGICAL_BPMx << MORPHOLOGICAL_BPMy << MORPHOLOGICAL_BPMz;
  tags << MORPHOLOGICAL_BPA00 << MORPHOLOGICAL_BPA01 << MORPHOLOGICAL_BPA02;
  tags << MORPHOLOGICAL_BPA10 << MORPHOLOGICAL_BPA11 << MORPHOLOGICAL_BPA12;
  tags << MORPHOLOGICAL_BPA20 << MORPHOLOGICAL_BPA21 << MORPHOLOGICAL_BPA22;
  tags << MORPHOLOGICAL_FD;
  tags << MORPHOLOGICAL_EEDx << MORPHOLOGICAL_EEDy << MORPHOLOGICAL_EEDz;

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
  bool requestedInvalidFeret = cached && (tag == MORPHOLOGICAL_FD && s_cache[m_segmentation].Data.FeretDiameter == -1);

  if (!cached || requestedInvalidFeret)
  {
    if (tag == MORPHOLOGICAL_FD)
      m_labelMap->SetComputeFeretDiameter(true);

    updateInformation();
  }

  ExtensionData &data = s_cache[m_segmentation].Data;

  if (tag == MORPHOLOGICAL_SIZE)
    return data.Size;
  if (tag == MORPHOLOGICAL_PS)
    return data.PhysicalSize;
  if (tag == MORPHOLOGICAL_Cx)
    return data.Centroid[0];
  if (tag == MORPHOLOGICAL_Cy)
    return data.Centroid[1];
  if (tag == MORPHOLOGICAL_Cz)
    return data.Centroid[2];
//   if (info == Rx)
//       return m_Region[0]*spacing[0];
//   if (info == "Region Y")
//       return m_Region[1]*spacing[1];
//   if (info == "Region Z")
//     return m_Region[2]*spacing[2];
  if (tag == MORPHOLOGICAL_BPMx)
    return data.BinaryPrincipalMoments[0];
  if (tag == MORPHOLOGICAL_BPMy)
    return data.BinaryPrincipalMoments[1];
  if (tag == MORPHOLOGICAL_BPMz)
    return data.BinaryPrincipalMoments[2];
  if (tag == MORPHOLOGICAL_BPA00)
    return data.BinaryPrincipalAxes[0][0];
  if (tag == MORPHOLOGICAL_BPA01)
    return data.BinaryPrincipalAxes[0][1];
  if (tag == MORPHOLOGICAL_BPA02)
    return data.BinaryPrincipalAxes[0][2];
  if (tag == MORPHOLOGICAL_BPA10)
    return data.BinaryPrincipalAxes[1][0];
  if (tag == MORPHOLOGICAL_BPA11)
    return data.BinaryPrincipalAxes[1][1];
  if (tag == MORPHOLOGICAL_BPA12)
    return data.BinaryPrincipalAxes[1][2];
  if (tag == MORPHOLOGICAL_BPA20)
    return data.BinaryPrincipalAxes[2][0];
  if (tag == MORPHOLOGICAL_BPA21)
    return data.BinaryPrincipalAxes[2][1];
  if (tag == MORPHOLOGICAL_BPA22)
    return data.BinaryPrincipalAxes[2][2];
  if (tag == MORPHOLOGICAL_FD)
    return data.FeretDiameter;
  if (tag == MORPHOLOGICAL_EEDx)
    return data.EquivalentEllipsoidSize[0];
  if (tag == MORPHOLOGICAL_EEDy)
    return data.EquivalentEllipsoidSize[1];
  if (tag == MORPHOLOGICAL_EEDz)
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
          extensionSegmentation = segmentation.get();
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
  bool invalid = false;
  if (seg->hasInformationExtension(MorphologicalInformationID))
  {
    invalid = !seg->informationExtension(MorphologicalInformationID)->isEnabled();
  } else 
  {
    invalid = seg->outputIsModified();
  }
  return invalid;
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
  SegmentationVolumeSPtr segVolume = segmentationVolume(m_segmentation->output());

  bool          validInfo = segVolume != NULL;
  LabelMapType *labelMap  = NULL;

  if (validInfo)
  {
    m_labelMap->SetInput(segVolume->toITK());
    m_labelMap->Update();
    m_labelMap->Modified();

    labelMap = m_labelMap->GetOutput();
    labelMap->Update();

    validInfo = labelMap->GetNumberOfLabelObjects() == 1;
  }

  if (validInfo)
  {
    m_statistic = labelMap->GetNthLabelObject(0);

    s_cache.markAsClean(m_segmentation);

    ExtensionData &data = s_cache[m_segmentation].Data;

    data.Size = static_cast<unsigned int>(m_statistic->GetNumberOfPixels());

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
