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

#include "StereologicalInclusion.h"

#include "CountingFrameExtension.h"
#include "CountingFrames/CountingFrame.h"

#include <Core/Extensions/EdgeDistances/EdgeDistance.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;


const Segmentation::InfoTag StereologicalInclusion::EXCLUDED = "Excluded from CF";

const QString StereologicalInclusion::EXTENSION_FILE = StereologicalInclusionID + "/StereologicalInclusion.csv";

StereologicalInclusion::ExtensionCache StereologicalInclusion::s_cache;

const std::string FILE_VERSION = StereologicalInclusionID.toStdString() + " 1.0\n";
const char SEP = ';';

//------------------------------------------------------------------------
StereologicalInclusion::StereologicalInclusion()
: m_isOnEdge(false)
{
}

//------------------------------------------------------------------------
StereologicalInclusion::~StereologicalInclusion()
{
  if (m_segmentation)
  {
    //qDebug() << "Deleting" << m_segmentation->data().toString() << StereologicalInclusionID;
    invalidate(m_segmentation);
  }
}

//------------------------------------------------------------------------
ModelItem::ExtId StereologicalInclusion::id()
{
  return StereologicalInclusionID;
}

//------------------------------------------------------------------------
ModelItem::ExtIdList StereologicalInclusion::dependencies() const
{
  ModelItem::ExtIdList deps;
  deps << EdgeDistanceID;
  return deps;
}

//------------------------------------------------------------------------
Segmentation::InfoTagList StereologicalInclusion::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << EXCLUDED;

  return tags;
}

//------------------------------------------------------------------------
void StereologicalInclusion::setSegmentation(SegmentationPtr seg)
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
QVariant StereologicalInclusion::information(const Segmentation::InfoTag &tag)
{
  if (EXCLUDED == tag)
  {
    if (!s_cache.isCached(m_segmentation))
      evaluateCountingFrames();

    ExtensionData &data = s_cache[m_segmentation].Data;

    QStringList excludingCFs;
    foreach(CountingFrame::Id id, data.ExclusionCFs.keys())
    {
      if (data.ExclusionCFs[id])
        excludingCFs << QString::number(id);
    }
    return excludingCFs.join(", ");
  }

  qWarning() << StereologicalInclusionID << ":"  << tag << " is not provided";
  return QVariant();
}

//------------------------------------------------------------------------
QString StereologicalInclusion::toolTipText() const
{
  QString tooltip;

  if (s_cache.contains(m_segmentation))
  {
    ExtensionData &data = s_cache[m_segmentation].Data;

    bool addBreakLine = false;
    foreach(CountingFrame::Id id, data.ExclusionCFs.keys())
    {
      if (addBreakLine) tooltip = tooltip.append("<br>");

      QString description = data.ExclusionCFs[id]?
                            "<font color=\"red\">"   + tr("Excluded from Counting Frame %1").arg(id) + "</font>":
                            "<font color=\"green\">" + tr("Included in Counting Frame %1"   ).arg(id) + "</font>";
      tooltip = tooltip.append(condition(":/apply.svg", description));

      addBreakLine = true;
    }
  }

  return tooltip;
}

//------------------------------------------------------------------------
void StereologicalInclusion::setCountingFrames(CountingFrameList countingFrames)
{
  Q_ASSERT(m_segmentation);
  //   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
  //   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
  QSet<CountingFrame *> prevCF = m_exclusionCFs.keys().toSet();
  QSet<CountingFrame *> newCF  = countingFrames.toSet();

  ExtensionData &data = s_cache[m_segmentation].Data;
  data.IsExcluded = false;
  // Remove regions that doesn't exist anymore
  foreach(CountingFrame *cf, prevCF.subtract(newCF))
  {
    m_exclusionCFs.remove(cf);
    data.ExclusionCFs.remove(cf->id());
  }

  foreach(CountingFrame *countingFrame, newCF.subtract(prevCF))
  {
    evaluateCountingFrame(countingFrame);
  }

  //   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

//------------------------------------------------------------------------
void StereologicalInclusion::loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model)
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

        for (int f = 2; f < fields.size(); ++f)
        {
          CountingFrame::Id id  = fields[f].toInt();
          bool excluded = id < 0;
          data.ExclusionCFs[abs(id)] = excluded;
          data.IsExcluded = data.IsExcluded || excluded;
        }
      } else
      {
        qWarning() << StereologicalInclusionID << "Invalid Cache Entry:" << line;
      }
    };
  }
}

//------------------------------------------------------------------------
// It's declared static to avoid collisions with other functions with same
// signature in different compilation units
static bool invalidData(SegmentationPtr seg)
{
  return !seg->hasInformationExtension(StereologicalInclusionID)
      && seg->outputIsModified();
}

//------------------------------------------------------------------------
bool StereologicalInclusion::saveCache(Snapshot &cacheList)
{
  s_cache.purge(invalidData);

  if (s_cache.isEmpty())
    return false;

  std::ostringstream cache;
  cache << FILE_VERSION;

  SegmentationPtr segmentation;
  foreach(segmentation, s_cache.keys())
  {
    ExtensionData &data = s_cache[segmentation].Data;

    cache << segmentation->filter()->id().toStdString();
    cache << SEP << segmentation->outputId();

    foreach(CountingFrame::Id id, data.ExclusionCFs.keys())
    {
      cache << SEP << (data.ExclusionCFs[id]?-id:id);
    }

    cache << std::endl;
  }

  cacheList << QPair<QString, QByteArray>(EXTENSION_FILE, cache.str().c_str());

  return true;
}

//------------------------------------------------------------------------
Segmentation::InformationExtension StereologicalInclusion::clone()
{
  return new StereologicalInclusion();
}

//------------------------------------------------------------------------
void StereologicalInclusion::initialize()
{
  if (s_cache.isCached(m_segmentation))
  {
    s_cache.markAsClean(m_segmentation);
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::invalidate(SegmentationPtr segmentation)
{
  if (!segmentation)
    segmentation = m_segmentation;

  if (segmentation)
  {
    //qDebug() << "Invalidate" << m_segmentation->data().toString() << StereologicalInclusionID;
    s_cache.markAsDirty(segmentation);
  }
}


//------------------------------------------------------------------------
bool StereologicalInclusion::isExcluded() const
{
  //qDebug() << m_segmentation->data().toString() << "is Cached:" << s_cache.isCached(m_segmentation);
  if (!s_cache.isCached(m_segmentation))
  {
    const_cast<StereologicalInclusion *>(this)->evaluateCountingFrames();
  }

  return s_cache[m_segmentation].Data.IsExcluded;
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrames()
{
  //qDebug() << "Evaluate Counting Frames" << m_segmentation->data().toString() << StereologicalInclusionID;
  SampleSPtr sample = m_segmentation->sample();
  if (!sample.isNull())
  {
    CountingFrameList countingFrames;
    foreach (ChannelPtr channel, sample->channels())
    {
      Channel::ExtensionPtr ext = channel->extension(CountingFrameExtensionID);
      if (ext)
      {
        CountingFrameExtension *channelExt = dynamic_cast<CountingFrameExtension *>(ext);
        countingFrames << channelExt->countingFrames();
      }
    }

    if (!countingFrames.isEmpty())
    {
      m_isOnEdge = isOnEdge();

      setCountingFrames(countingFrames);
    } else 
    {
      s_cache[m_segmentation].Data.IsExcluded = false;
    }
    s_cache.markAsClean(m_segmentation);
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrame(CountingFrame* countingFrame)
{
  // Compute CF's exclusion value
  bool excluded = m_isOnEdge || isExcludedFromCountingFrame(countingFrame);

  m_exclusionCFs[countingFrame] = excluded;

  ExtensionData &data = s_cache[m_segmentation].Data;
  data.ExclusionCFs[countingFrame->id()] = excluded;

  // Update segmentation's exclusion value
  excluded = true;

  int i = 0;
  CountingFrameList countingFrames = m_exclusionCFs.keys();
  while (excluded && i < countingFrames.size())
  {
    excluded = excluded && m_exclusionCFs[countingFrames[i]];
    i++;
  }
  data.IsExcluded = excluded;

  s_cache.markAsClean(m_segmentation);
}


//------------------------------------------------------------------------
bool StereologicalInclusion::isExcludedFromCountingFrame(CountingFrame* countingFrame)
{
  const TaxonomyElement *taxonomicalConstraint = countingFrame->taxonomicalConstraint();

  if (taxonomicalConstraint && m_segmentation->taxonomy() != taxonomicalConstraint)
    return true;

  EspinaRegion inputBB = m_segmentation->output()->region();

  vtkPolyData  *region       = countingFrame->region();
  vtkPoints    *regionPoints = region->GetPoints();
  vtkCellArray *regionFaces  = region->GetPolys();
  vtkCellData  *faceData     = region->GetCellData();

  double bounds[6];
  regionPoints->GetBounds(bounds);
  EspinaRegion regionBB(bounds);

  // If there is no intersection (nor is inside), then it is excluded
  if (!inputBB.intersect(regionBB))
    return true;

  bool collisionDected = false;
  // Otherwise, we have to test all faces collisions
  int numOfCells = regionFaces->GetNumberOfCells();
  regionFaces->InitTraversal();
  for(int f=0; f < numOfCells; f++)
  {
    vtkIdType npts, *pts;
    regionFaces->GetNextCell(npts, pts);

    vtkSmartPointer<vtkPoints> facePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < npts; i++)
      facePoints->InsertNextPoint(regionPoints->GetPoint(pts[i]));

    facePoints->GetBounds(bounds);
    EspinaRegion faceBB(bounds);
    if (inputBB.intersect(faceBB) && isRealCollision(inputBB.intersection(faceBB)))
    {
      if (faceData->GetScalars()->GetComponent(f,0) == 0)
        return true;
      collisionDected = true;
    }
  }

  if (collisionDected)
    return false;

  // If no collision was detected we have to check for inclusion
  for (int p=0; p + 7 < regionPoints->GetNumberOfPoints(); p +=4)
  {
    vtkSmartPointer<vtkPoints> slicePoints = vtkSmartPointer<vtkPoints>::New();
    for (int i=0; i < 8; i++)
      slicePoints->InsertNextPoint(regionPoints->GetPoint(p+i));

    slicePoints->GetBounds(bounds);
    EspinaRegion sliceBB(bounds);
    if (inputBB.intersect(sliceBB) && isRealCollision(inputBB.intersection(sliceBB)))
      return false;//;
  }

  // If no internal collision was detected, then the input was indeed outside our
  // bounding region
  return true;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isOnEdge()
{
  bool excluded  = false;
  Nm   threshold = 1.0;

  Segmentation::InformationExtension ext = m_segmentation->informationExtension(EdgeDistanceID);
  if (ext)
  {
    EdgeDistancePtr distanceExtension = dynamic_cast<EdgeDistancePtr>(ext);
    Segmentation::InfoTagList tags = distanceExtension->availableInformations();
    int i = 0;
    while (!excluded && i < tags.size())
    {
      bool ok = false;
      double dist = m_segmentation->information(tags[i++]).toDouble(&ok);
      excluded = ok && dist < threshold;
    }
  }

  return excluded;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isRealCollision(EspinaRegion interscetion)
{
  // TODO: esto tiene en cuenta el shift del origen de las imágenes de ITK?
  SegmentationVolumeSPtr volume = segmentationVolume(m_segmentation->output());

  itkVolumeType::Pointer input = volume->toITK();
  for (int z = interscetion.zMin(); z <= interscetion.zMax(); z++)
    for (int y = interscetion.yMin(); y <= interscetion.yMax(); y++)
      for (int x = interscetion.xMin(); x <= interscetion.xMax(); x++)
      {
        itkVolumeType::IndexType index = volume->index(x, y, z);
        if (input->GetLargestPossibleRegion().IsInside(index) && input->GetPixel(index))
          return true;
      }

  return false;
}

//------------------------------------------------------------------------
StereologicalInclusionPtr EspINA::stereologicalInclusionPtr(Segmentation::InformationExtension extension)
{
  return dynamic_cast<StereologicalInclusionPtr>(extension);
}
