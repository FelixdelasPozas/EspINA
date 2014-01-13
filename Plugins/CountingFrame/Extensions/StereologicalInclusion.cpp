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
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <GUI/Utils/Conditions.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Segmentation.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;
using namespace EspINA::CF;

const SegmentationExtension::Type    StereologicalInclusion::TYPE     = "StereologicalInclusion";
const SegmentationExtension::InfoTag StereologicalInclusion::EXCLUDED = "Excluded from CF";

const QString StereologicalInclusion::FILE = StereologicalInclusion::TYPE + "/StereologicalInclusion.csv";

const std::string FILE_VERSION = StereologicalInclusion::TYPE.toStdString() + " 1.0\n";
const char SEP = ';';

//------------------------------------------------------------------------
StereologicalInclusion::StereologicalInclusion()
: m_isOnEdge(false)
, m_isInitialized(false)
, m_isExcluded(false)
{
}

//------------------------------------------------------------------------
StereologicalInclusion::~StereologicalInclusion()
{
}

//------------------------------------------------------------------------
State StereologicalInclusion::state() const
{
  State state;

  return state;
}

//------------------------------------------------------------------------
Snapshot StereologicalInclusion::snapshot() const
{
  Snapshot snapshot;

  return snapshot;
}

//------------------------------------------------------------------------
SegmentationExtension::TypeList StereologicalInclusion::dependencies() const
{
  TypeList dependencies;

  dependencies << EdgeDistance::TYPE;

  return dependencies;
}

//------------------------------------------------------------------------
SegmentationExtension::InfoTagList StereologicalInclusion::availableInformations() const
{
  InfoTagList tags;

  tags << EXCLUDED;

  return tags;
}

//------------------------------------------------------------------------
void StereologicalInclusion::onSegmentationSet(SegmentationPtr segmentation)
{
//   connect(segmentation, SIGNAL(outputModified()),
//           this,         SLOT(invalidate()));
//
//   if (m_segmentation->outputIsModified())
//     invalidate();
//   else
//     initialize();
}

//------------------------------------------------------------------------
QVariant StereologicalInclusion::information ( const SegmentationExtension::InfoTag& tag ) const
{
  if (EXCLUDED == tag)
  {
    evaluateCountingFrames();

    QStringList excludingCFs;
    for(auto cf : m_excludedByCF.keys())
    {
      if (m_excludedByCF[cf])
      {
        excludingCFs << QString::number(cf);
      }
    }
    return excludingCFs.join(", ");
  }

  qWarning() << StereologicalInclusion::TYPE << ":"  << tag << " is not provided";
  return QVariant();
}

//------------------------------------------------------------------------
QString StereologicalInclusion::toolTipText() const
{
  QString tooltip;

  bool addBreakLine = false;
  for(auto id : m_excludedByCF.keys())
  {
    if (addBreakLine) tooltip = tooltip.append("<br>");

    QString description = m_excludedByCF[id]?
    "<font color=\"red\">"   + tr("Excluded from Counting Frame %1").arg(id) + "</font>":
    "<font color=\"green\">" + tr("Included in Counting Frame %1"  ).arg(id) + "</font>";
    tooltip = tooltip.append(condition(":/apply.svg", description));

    addBreakLine = true;
  }

  return tooltip;
}

//------------------------------------------------------------------------
void StereologicalInclusion::setCountingFrames(CountingFrameList countingFrames)
{
  Q_ASSERT(m_segmentation);
//   //   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
//   //   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
//   QSet<CountingFrame *> prevCF = m_exclusionCFs.keys().toSet();
//   QSet<CountingFrame *> newCF  = countingFrames.toSet();
//
//   ExtensionData &data = s_cache[m_segmentation].Data;
//   data.IsExcluded = false;
//   // Remove regions that doesn't exist anymore
//   foreach(CountingFrame *cf, prevCF.subtract(newCF))
//   {
//     m_exclusionCFs.remove(cf);
//     data.ExclusionCFs.remove(cf->id());
//   }
//
//   foreach(CountingFrame *countingFrame, newCF.subtract(prevCF))
//   {
//     evaluateCountingFrame(countingFrame);
//   }

  //   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

// //------------------------------------------------------------------------
// void StereologicalInclusion::loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model)
// {
//   QString header(file.readLine());
//   if (header.toStdString() == FILE_VERSION)
//   {
//     char buffer[1024];
//     while (file.readLine(buffer, sizeof(buffer)) > 0)
//     {
//       QString line(buffer);
//       QStringList fields = line.split(SEP);
//
//       SegmentationPtr extensionSegmentation = NULL;
//       int i = 0;
//       while (!extensionSegmentation && i < model->segmentations().size())
//       {
//         SegmentationSPtr segmentation = model->segmentations()[i];
//         if ( segmentation->filter()->id()       == fields[0]
//           && segmentation->outputId()           == fields[1].toInt()
//           && segmentation->filter()->cacheDir() == tmpDir)
//         {
//           extensionSegmentation = segmentation.get();
//         }
//         i++;
//       }
//       if (extensionSegmentation)
//       {
//         ExtensionData &data = s_cache[extensionSegmentation].Data;
//
//         for (int f = 2; f < fields.size(); ++f)
//         {
//           CountingFrame::Id id  = fields[f].toInt();
//           bool excluded = id < 0;
//           data.ExclusionCFs[abs(id)] = excluded;
//           data.IsExcluded = data.IsExcluded || excluded;
//         }
//       } else
//       {
//         qWarning() << StereologicalInclusionID << "Invalid Cache Entry:" << line;
//       }
//     };
//   }
// }

// //------------------------------------------------------------------------
// // It's declared static to avoid collisions with other functions with same
// // signature in different compilation units
// static bool invalidData(SegmentationPtr seg)
// {
//   bool invalid = false;
//   if (seg->hasInformationExtension(StereologicalInclusionID))
//   {
//     invalid = !seg->informationExtension(StereologicalInclusionID)->isEnabled();
//   } else
//   {
//     invalid = seg->outputIsModified();
//   }
//   return invalid;
// }
//
// //------------------------------------------------------------------------
// bool StereologicalInclusion::saveCache(Snapshot &cacheList)
// {
//   s_cache.purge(invalidData);
//
//   if (s_cache.isEmpty())
//     return false;
//
//   std::ostringstream cache;
//   cache << FILE_VERSION;
//
//   SegmentationPtr segmentation;
//   foreach(segmentation, s_cache.keys())
//   {
//     ExtensionData &data = s_cache[segmentation].Data;
//
//     cache << segmentation->filter()->id().toStdString();
//     cache << SEP << segmentation->outputId();
//
//     foreach(CountingFrame::Id id, data.ExclusionCFs.keys())
//     {
//       cache << SEP << (data.ExclusionCFs[id]?-id:id);
//     }
//
//     cache << std::endl;
//   }
//
//   cacheList << QPair<QString, QByteArray>(FILE, cache.str().c_str());
//
//   return true;
// }


//------------------------------------------------------------------------
bool StereologicalInclusion::isExcluded() const
{
  if (!m_isInitialized)
  {
    const_cast<StereologicalInclusion *>(this)->evaluateCountingFrames();
  }

  return m_isExcluded;
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrames()
{
  SampleSPtr sample = Query::sample(m_segmentation);
  Q_ASSERT(sample);

//   CountingFrameList countingFrames;
//   for(auto channel : Query::channels(sample))
//   {
//     Channel::ExtensionPtr ext = channel->extension(CountingFrameExtensionID);
//     if (ext)
//     {
//       CountingFrameExtension *channelExt = dynamic_cast<CountingFrameExtension *>(ext);
//       countingFrames << channelExt->countingFrames();
//     }
//   }
//
//   if (!countingFrames.isEmpty())
//   {
//     m_isOnEdge = isOnEdge();
//
//     setCountingFrames(countingFrames);
//   } else
//   {
//     s_cache[m_segmentation].Data.IsExcluded = false;
//   }
//   s_cache.markAsClean(m_segmentation);
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrame(CountingFrame* countingFrame)
{
//   // Compute CF's exclusion value
//   bool excluded = m_isOnEdge || isExcludedFromCountingFrame(countingFrame);
//
//   m_exclusionCFs[countingFrame] = excluded;
//
//   ExtensionData &data = s_cache[m_segmentation].Data;
//   data.ExclusionCFs[countingFrame->id()] = excluded;
//
//   // Update segmentation's exclusion value
//   excluded = true;
//
//   int i = 0;
//   CountingFrameList countingFrames = m_exclusionCFs.keys();
//   while (excluded && i < countingFrames.size())
//   {
//     excluded = excluded && m_exclusionCFs[countingFrames[i]];
//     i++;
//   }
//   data.IsExcluded = excluded;
//
//   s_cache.markAsClean(m_segmentation);
}


//------------------------------------------------------------------------
bool StereologicalInclusion::isExcludedFromCountingFrame(CountingFrame* countingFrame)
{
//   const TaxonomyElement *taxonomicalConstraint = countingFrame->taxonomicalConstraint();
//
//   if (taxonomicalConstraint && m_segmentation->taxonomy().get() != taxonomicalConstraint)
//     return true;
//
//   EspinaRegion inputBB = m_segmentation->output()->region();
//
//   vtkPolyData  *region       = countingFrame->region();
//   vtkPoints    *regionPoints = region->GetPoints();
//   vtkCellArray *regionFaces  = region->GetPolys();
//   vtkCellData  *faceData     = region->GetCellData();
//
//   double bounds[6];
//   regionPoints->GetBounds(bounds);
//   EspinaRegion regionBB(bounds);
//
//   // If there is no intersection (nor is inside), then it is excluded
//   if (!inputBB.intersect(regionBB))
//     return true;
//
//   bool collisionDected = false;
//   // Otherwise, we have to test all faces collisions
//   int numOfCells = regionFaces->GetNumberOfCells();
//   regionFaces->InitTraversal();
//   for(int f=0; f < numOfCells; f++)
//   {
//     vtkIdType npts, *pts;
//     regionFaces->GetNextCell(npts, pts);
//
//     vtkSmartPointer<vtkPoints> facePoints = vtkSmartPointer<vtkPoints>::New();
//     for (int i=0; i < npts; i++)
//       facePoints->InsertNextPoint(regionPoints->GetPoint(pts[i]));
//
//     facePoints->GetBounds(bounds);
//     EspinaRegion faceBB(bounds);
//     if (inputBB.intersect(faceBB) && isRealCollision(inputBB.intersection(faceBB)))
//     {
//       if (faceData->GetScalars()->GetComponent(f,0) == 0)
//         return true;
//       collisionDected = true;
//     }
//   }
//
//   if (collisionDected)
//     return false;
//
//   // If no collision was detected we have to check for inclusion
//   for (int p=0; p + 7 < regionPoints->GetNumberOfPoints(); p +=4)
//   {
//     vtkSmartPointer<vtkPoints> slicePoints = vtkSmartPointer<vtkPoints>::New();
//     for (int i=0; i < 8; i++)
//       slicePoints->InsertNextPoint(regionPoints->GetPoint(p+i));
//
//     slicePoints->GetBounds(bounds);
//     EspinaRegion sliceBB(bounds);
//     if (inputBB.intersect(sliceBB) && isRealCollision(inputBB.intersection(sliceBB)))
//       return false;//;
//   }
//
//   // If no internal collision was detected, then the input was indeed outside our
//   // bounding region
//   return true;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isOnEdge()
{
  bool excluded  = false;
//   Nm   threshold = 1.0;
//
//   Segmentation::InformationExtension ext = m_segmentation->informationExtension(EdgeDistanceID);
//   if (ext)
//   {
//     EdgeDistancePtr distanceExtension = dynamic_cast<EdgeDistancePtr>(ext);
//     Segmentation::InfoTagList tags = distanceExtension->availableInformations();
//     int i = 0;
//     while (!excluded && i < tags.size())
//     {
//       bool ok = false;
//       double dist = m_segmentation->information(tags[i++]).toDouble(&ok);
//       excluded = ok && dist < threshold;
//     }
//   }
//
  return excluded;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isRealCollision(const Bounds& interscetion)
{
//   // TODO: esto tiene en cuenta el shift del origen de las imágenes de ITK?
//   SegmentationVolumeSPtr volume = segmentationVolume(m_segmentation->output());
//
//   itkVolumeType::Pointer input = volume->toITK();
//   for (int z = interscetion.zMin(); z <= interscetion.zMax(); z++)
//     for (int y = interscetion.yMin(); y <= interscetion.yMax(); y++)
//       for (int x = interscetion.xMin(); x <= interscetion.xMax(); x++)
//       {
//         itkVolumeType::IndexType index = volume->index(x, y, z);
//         if (input->GetLargestPossibleRegion().IsInside(index) && input->GetPixel(index))
//           return true;
//       }

  return false;
}

//------------------------------------------------------------------------
StereologicalInclusionSPtr EspINA::CF::stereologicalInclusion(SegmentationExtensionSPtr extension)
{
  //return dynamic_cast<StereologicalInclusionPtr>(extension);
}

