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

#include "CountingFrameSegmentationExtension.h"

#include "CountingFrameChannelExtension.h"
#include "CountingFrames/CountingFrame.h"

#include <Core/Extensions/Margins/MarginsSegmentationExtension.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Channel.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <QDebug>
#include <QApplication>

using namespace EspINA;

const ModelItemExtension::ExtId CountingFrameSegmentationExtension::ID = "CountingFrameExtension";
const ModelItemExtension::InfoTag CountingFrameSegmentationExtension::DISCARTED = "Discarded By CF";

//------------------------------------------------------------------------
CountingFrameSegmentationExtension::CountingFrameSegmentationExtension()
: m_isOnEdge(false)
{
  m_availableInformations << DISCARTED;
}

//------------------------------------------------------------------------
CountingFrameSegmentationExtension::~CountingFrameSegmentationExtension()
{
  qDebug() << "Deleting Counting Frame Segmentation Extension";
}

//------------------------------------------------------------------------
ModelItemExtension::ExtId CountingFrameSegmentationExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingFrameSegmentationExtension::initialize(ModelItem::Arguments args)
{
  SampleSPtr sample = m_seg->sample();

  ModelItemSList relatedChannels = sample->relatedItems(EspINA::OUT, Channel::STAINLINK);
  Q_ASSERT(relatedChannels.size() > 0);

  CountingFrameList countingFrames;
  for (int i = 0; i < relatedChannels.size(); i++)
  {
    ChannelSPtr channel = channelPtr(relatedChannels[i]);
    ModelItemExtensionPtr ext = channel->extension(CountingFrameChannelExtension::ID);
    Q_ASSERT(ext);
    CountingFrameChannelExtension *channelExt = dynamic_cast<CountingFrameChannelExtension *>(ext.data());
    countingFrames << channelExt->countingFrames();
  }

  m_isOnEdge = isOnEdge();

  setCountingFrames(countingFrames);

  connect(m_seg, SIGNAL(modified(ModelItem*)),
          this, SLOT(evaluateCountingFrames()));
}

//------------------------------------------------------------------------
ModelItemExtension::ExtIdList CountingFrameSegmentationExtension::dependencies() const
{
  ExtIdList deps;
  deps << MarginsSegmentationExtension::ID;
  return deps;
}


//------------------------------------------------------------------------
QVariant CountingFrameSegmentationExtension::information(ModelItemExtension::InfoTag tag) const
{
  if (DISCARTED == tag)
  {
    QStringList discardingCFs;
    foreach(CountingFrame *countingFrame, m_isDiscardedBy.keys())
    {
      if (m_isDiscardedBy[countingFrame])
        discardingCFs << QString::number(countingFrame->id());
    }
    return discardingCFs.join(", ");
  }

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

// DEPRECATED ????
//------------------------------------------------------------------------
SegmentationRepresentationPtr CountingFrameSegmentationExtension::representation(QString representation)
{
  SegmentationRepresentationPtr rep;
  qWarning() << ID << ":" << representation << " is not provided";
  Q_ASSERT(false);
  return rep;
}


//------------------------------------------------------------------------
void CountingFrameSegmentationExtension::setCountingFrames(CountingFrameList countingFrames)
{
//   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
//   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
QSet<CountingFrame *> prevCF = m_isDiscardedBy.keys().toSet();
QSet<CountingFrame *> newCF  = countingFrames.toSet();

// Remove regions that doesn't exist anymore
foreach(CountingFrame *cf, prevCF.subtract(newCF))
{
  m_isDiscardedBy.remove(cf);
}

foreach(CountingFrame *countingFrame, newCF.subtract(prevCF))
{
  evaluateCountingFrame(countingFrame);
}
//   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

//------------------------------------------------------------------------
SegmentationExtensionPtr CountingFrameSegmentationExtension::clone()
{
  return SegmentationExtensionPtr(new CountingFrameSegmentationExtension());
}

//------------------------------------------------------------------------
bool CountingFrameSegmentationExtension::isDiscarded() const
{
  bool discarded = false;

  if (!m_isDiscardedBy.isEmpty())
  {
    discarded = true;

    int i = 0;
    CountingFrameList countingFrames = m_isDiscardedBy.keys();
    while (discarded && i < countingFrames.size())
    {
      discarded = discarded && m_isDiscardedBy[countingFrames[i]];
      i++;
    }
  }

  return discarded;
}

//------------------------------------------------------------------------
void CountingFrameSegmentationExtension::evaluateCountingFrames()
{
  m_isOnEdge = isOnEdge();

  foreach(CountingFrame *cf, m_isDiscardedBy.keys())
    evaluateCountingFrame(cf);

}

//------------------------------------------------------------------------
void CountingFrameSegmentationExtension::evaluateCountingFrame(CountingFrame* countingFrame)
{
  bool discarded = m_isOnEdge || isDiscardedByCountingFrame(countingFrame);

  m_isDiscardedBy[countingFrame] = discarded;

  QString tag       = "CountingFrameCondition %1";
  QString condition = discarded?
                      "<font color=\"red\">"   + tr("Discarded by Counting Frame %1").arg(countingFrame->id()) + "</font>":
                      "<font color=\"green\">" + tr("Inside of Counting Frame %1"   ).arg(countingFrame->id()) + "</font>";
  m_seg->addCondition(tag.arg(countingFrame->id()), ":/apply.svg", condition);
}


//------------------------------------------------------------------------
bool CountingFrameSegmentationExtension::isDiscardedByCountingFrame(CountingFrame* countingFame)
{
  const TaxonomyElement *taxonomicalConstraint = countingFame->taxonomicalConstraint();

  if (taxonomicalConstraint && m_seg->taxonomy() != taxonomicalConstraint)
    return true;

  EspinaRegion inputBB = m_seg->volume()->espinaRegion();

  vtkPolyData  *region       = countingFame->region();
  vtkPoints    *regionPoints = region->GetPoints();
  vtkCellArray *regionFaces  = region->GetPolys();
  vtkCellData  *faceData     = region->GetCellData();

  double bounds[6];
  regionPoints->GetBounds(bounds);
  EspinaRegion regionBB(bounds);

  // If there is no intersection (nor is inside), then it is discarted
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
    if (inputBB.intersect(faceBB) && realCollision(inputBB.intersection(faceBB)))
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
    if (inputBB.intersect(sliceBB) && realCollision(inputBB.intersection(sliceBB)))
      return false;//;
  }

  // If no internal collision was detected, then the input was indeed outside our
  // bounding region
  return true;
}

//------------------------------------------------------------------------
bool CountingFrameSegmentationExtension::isOnEdge()
{
  bool discarted = false;

  ModelItemExtensionPtr ext = m_seg->extension(MarginsSegmentationExtension::ID);
  MarginsSegmentationExtension *marginExt = dynamic_cast<MarginsSegmentationExtension *>(ext.data());
  if (marginExt)
  {
    InfoList tags = marginExt->availableInformations();
    int i = 0;
    while (!discarted && i < tags.size())
    {
      bool ok = false;
      double dist = m_seg->information(tags[i++]).toDouble(&ok);
      discarted = ok && dist < 1.0;
    }
  }

  return discarted;
}

//------------------------------------------------------------------------
bool CountingFrameSegmentationExtension::realCollision(EspinaRegion interscetion)
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

