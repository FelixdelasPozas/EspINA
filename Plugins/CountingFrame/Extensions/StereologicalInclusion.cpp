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
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/VolumetricData.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <itkImageRegionIterator.h>

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
  //   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
  //   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
  QSet<CountingFrame *> prevCF = m_exclusionCFs.keys().toSet();
  QSet<CountingFrame *> newCF  = countingFrames.toSet();

  m_isExcluded = false;

  // Remove regions that doesn't exist anymore
  for(auto cf : prevCF.subtract(newCF))
  {
    m_exclusionCFs.remove(cf);
    m_excludedByCF.remove(cf->id());
  }

  for(auto cf : newCF.subtract(prevCF))
  {
    evaluateCountingFrame(cf);
  }
//     EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
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
  m_isInitialized = false;

  if (!m_segmentation)
    return;

  auto samples = Query::samples(m_segmentation);

  if (samples.size() > 1)
  {
    qWarning() << "Counting Frame<evaluateCountingFrames>: Tiling mode not suppoerted";
  } else if (!samples.isEmpty())
  {
    auto sample = samples.first();

    CountingFrameList countingFrames;
    for(auto channel : Query::channels(sample))
    {
      if (channel->hasExtension(CountingFrameExtension::TYPE))
      {
        auto extension = channel->extension(CountingFrameExtension::TYPE);

        auto cfExtension = std::dynamic_pointer_cast<CountingFrameExtension>(extension);
        countingFrames << cfExtension->countingFrames();
      }
    }

    if (countingFrames.isEmpty())
    {
      m_isExcluded = false;
    } else
    {
      m_isOnEdge = isOnEdge();

      setCountingFrames(countingFrames);
    }

    m_isInitialized = true;
  }
}

//------------------------------------------------------------------------
void StereologicalInclusion::evaluateCountingFrame(CountingFrame* cf)
{
  // Compute CF's exclusion value
  bool excluded = m_isOnEdge || isExcludedByCountingFrame(cf);

  m_exclusionCFs[cf] = excluded;

  m_excludedByCF[cf->id()] = excluded;

  // Update segmentation's exclusion value
  excluded = true;

  int i = 0;
  CountingFrameList countingFrames = m_exclusionCFs.keys();
  while (excluded && i < countingFrames.size())
  {
    excluded = excluded && m_exclusionCFs[countingFrames[i]];
    i++;
  }
  m_isExcluded = excluded;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isExcludedByCountingFrame(CountingFrame* cf)
{
  auto categoryConstraint = cf->categoryConstraint();

  if (categoryConstraint && m_segmentation->category() != categoryConstraint)
    return true;

  Bounds inputBB = m_segmentation->output()->bounds();
  //qDebug() << "Input:" << inputBB.toString();

  vtkPolyData  *region       = cf->region();
  vtkPoints    *regionPoints = region->GetPoints();
  vtkCellArray *regionFaces  = region->GetPolys();
  vtkCellData  *faceData     = region->GetCellData();

  auto pointBounds = [] (vtkPoints *points) {
    Bounds bounds;
    for (int i = 0; i < 6; ++i)
      bounds[i] = points->GetBounds()[i];
    return bounds;
  };

  Bounds regionBB = pointBounds(regionPoints);
  //qDebug() << "Region:" << regionBB.toString();

  // If there is no intersection (nor is inside), then it is excluded
  if (!intersect(inputBB, regionBB))
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

    Bounds faceBB = pointBounds(facePoints);
    //qDebug() << "Face:" << faceBB.toString();
    //qDebug() << " - intersect:" << intersect(inputBB, faceBB);
    //qDebug() << " - interscetion:" << intersection(inputBB, faceBB).toString();
    //qDebug() << " - type:" << faceData->GetScalars()->GetComponent(f, 0);

    if (intersect(inputBB, faceBB) && isRealCollision(intersection(inputBB, faceBB)))
    {
      if (faceData->GetScalars()->GetComponent(f,0) == cf->EXCLUSION_FACE)
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

    Bounds sliceBB = pointBounds(slicePoints);
    if (intersect(inputBB, sliceBB) && isRealCollision(intersection(inputBB, sliceBB)))
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

  if (m_segmentation->hasExtension(EdgeDistance::TYPE))
  {
    auto extension = m_segmentation->extension(EdgeDistance::TYPE);

    auto distanceExtension = std::dynamic_pointer_cast<EdgeDistance>(extension);

    auto tags = distanceExtension->availableInformations();

    int i = 0;
    while (!excluded && i < tags.size())
    {
      bool   ok   = false;
      double dist = m_segmentation->information(tags[i++]).toDouble(&ok);

      excluded = ok && dist < threshold;
    }
  }

  return excluded;
}

//------------------------------------------------------------------------
bool StereologicalInclusion::isRealCollision(const Bounds& interscetion)
{
  using ImageIterator = itk::ImageRegionIterator<itkVolumeType>;
  // TODO?: add some
  auto volume = volumetricData(m_segmentation->output());

  auto image = volume->itkImage(interscetion);

  ImageIterator it = ImageIterator(image, image->GetLargestPossibleRegion());
  it.GoToBegin();
  while (!it.IsAtEnd()) {
    if (it.Get() != volume->backgroundValue())
      return true;
    ++it;
  }

  return false;
}

//------------------------------------------------------------------------
StereologicalInclusionSPtr EspINA::CF::stereologicalInclusion(SegmentationExtensionSPtr extension)
{
  return std::dynamic_pointer_cast<StereologicalInclusion>(extension);
}

