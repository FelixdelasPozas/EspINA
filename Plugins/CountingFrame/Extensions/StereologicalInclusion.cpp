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

#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;

const ModelItem::ExtId StereologicalInclusion::ID = "CountingFrameExtension";

const Segmentation::InfoTag StereologicalInclusion::EXCLUDED = "Excluded from CF";

const QString StereologicalInclusion::EXTENSION_FILE = StereologicalInclusion::ID + "/StereologicalInclusion.csv";

QMap<SegmentationPtr, StereologicalInclusion::CacheEntry> StereologicalInclusion::s_cache;

const std::string FILE_VERSION = StereologicalInclusion::ID.toStdString() + " 1.0\n";
const char SEP = ';';

//------------------------------------------------------------------------
StereologicalInclusion::StereologicalInclusion()
: m_isOnEdge(false)
{
}

//------------------------------------------------------------------------
StereologicalInclusion::~StereologicalInclusion()
{
  qDebug() << "Deleting" << m_seg->data().toString() << ID;
}

//------------------------------------------------------------------------
ModelItem::ExtId StereologicalInclusion::id()
{
  return ID;
}

//------------------------------------------------------------------------
void StereologicalInclusion::initialize(ModelItem::Arguments args)
{
  qDebug() << "Initialize (ignore args)" << m_seg->data().toString() << ID;

  SampleSPtr sample = m_seg->sample();

  ModelItemSList relatedChannels = sample->relatedItems(EspINA::OUT, Channel::STAINLINK);
  Q_ASSERT(relatedChannels.size() > 0);

  CountingFrameList countingFrames;
  for (int i = 0; i < relatedChannels.size(); i++)
  {
    ChannelSPtr channel = channelPtr(relatedChannels[i]);
    Channel::ExtensionPtr ext = channel->extension(CountingFrameExtensionID);
    if (ext)
    {
      CountingFrameExtension *channelExt = dynamic_cast<CountingFrameExtension *>(ext);
      countingFrames << channelExt->countingFrames();
    }
  }

  m_isOnEdge = isOnEdge();

  setCountingFrames(countingFrames);

  connect(m_seg, SIGNAL(modified(ModelItem*)),
          this, SLOT(evaluateCountingFrames()));
}

//------------------------------------------------------------------------
ModelItem::ExtIdList StereologicalInclusion::dependencies() const
{
  ModelItem::ExtIdList deps;
  deps << EdgeDistance::ID;
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
QVariant StereologicalInclusion::information(const Segmentation::InfoTag &tag)
{
  if (EXCLUDED == tag)
  {
    bool cached = s_cache.contains(m_seg);
    if (!cached)
      evaluateCountingFrames();

    QStringList excludingCFs;
    foreach(int countingFrame, s_cache[m_seg])
    {
      excludingCFs << QString::number(countingFrame);
    }
    return excludingCFs.join(", ");
  }

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
void StereologicalInclusion::setCountingFrames(CountingFrameList countingFrames)
{
//   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
//   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
QSet<CountingFrame *> prevCF = m_isExcludedFrom.keys().toSet();
QSet<CountingFrame *> newCF  = countingFrames.toSet();

// Remove regions that doesn't exist anymore
foreach(CountingFrame *cf, prevCF.subtract(newCF))
{
  m_isExcludedFrom.remove(cf);
}

foreach(CountingFrame *countingFrame, newCF.subtract(prevCF))
{
  evaluateCountingFrame(countingFrame);
}
//   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

//------------------------------------------------------------------------
bool StereologicalInclusion::loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model)
{
  // TODO: 
  return false;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::saveCache(CacheList &cacheList)
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

    cache << SEP << segmentation->information(EXCLUDED).toString().toStdString();

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
bool StereologicalInclusion::isExcluded() const
{
  bool excluded = false;

  if (!m_isExcludedFrom.isEmpty())
  {
    excluded = true;

    int i = 0;
    CountingFrameList countingFrames = m_isExcludedFrom.keys();
    while (excluded && i < countingFrames.size())
    {
      excluded = excluded && m_isExcludedFrom[countingFrames[i]];
      i++;
    }
  }

  return excluded;
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrames()
{
  qDebug() << "Evaluate Counting Frames" << m_seg->data().toString() << ID;
  m_isOnEdge = isOnEdge();

  foreach(CountingFrame *cf, m_isExcludedFrom.keys())
    evaluateCountingFrame(cf);

}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrame(CountingFrame* countingFrame)
{
  bool excluded = m_isOnEdge || isExcludedFromCountingFrame(countingFrame);

  m_isExcludedFrom[countingFrame] = excluded;

  if (excluded)
    s_cache[m_seg].insert(countingFrame->id());
  else
    s_cache[m_seg].remove(countingFrame->id());

  QString tag       = "CountingFrameCondition %1";
  QString condition = excluded?
                      "<font color=\"red\">"   + tr("Excluded from Counting Frame %1").arg(countingFrame->id()) + "</font>":
                      "<font color=\"green\">" + tr("Included in Counting Frame %1"   ).arg(countingFrame->id()) + "</font>";
  m_seg->addCondition(tag.arg(countingFrame->id()), ":/apply.svg", condition);
}


//------------------------------------------------------------------------
bool StereologicalInclusion::isExcludedFromCountingFrame(CountingFrame* countingFrame)
{
  const TaxonomyElement *taxonomicalConstraint = countingFrame->taxonomicalConstraint();

  if (taxonomicalConstraint && m_seg->taxonomy() != taxonomicalConstraint)
    return true;

  EspinaRegion inputBB = m_seg->volume()->espinaRegion();

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

  Segmentation::InformationExtension ext = m_seg->informationExtension(EdgeDistance::ID);
  if (ext)
  {
    EdgeDistancePtr distanceExtension = dynamic_cast<EdgeDistancePtr>(ext);
    Segmentation::InfoTagList tags = distanceExtension->availableInformations();
    int i = 0;
    while (!excluded && i < tags.size())
    {
      bool ok = false;
      double dist = m_seg->information(tags[i++]).toDouble(&ok);
      excluded = ok && dist < threshold;
    }
  }

  return excluded;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isRealCollision(EspinaRegion interscetion)
{
  itkVolumeType::Pointer input = m_seg->volume()->toITK();
  for (int z = interscetion.zMin(); z <= interscetion.zMax(); z++)
    for (int y = interscetion.yMin(); y <= interscetion.yMax(); y++)
      for (int x = interscetion.xMin(); x <= interscetion.xMax(); x++)
      {
        itkVolumeType::IndexType index = m_seg->volume()->index(x, y, z);
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
