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

#include "CountingRegionSegmentationExtension.h"

#include "CountingRegionChannelExtension.h"
#include "regions/BoundingRegion.h"

#include <Core/Extensions/Margins/MarginsSegmentationExtension.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Channel.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <QDebug>
#include <QApplication>

const ModelItemExtension::ExtId CountingRegionSegmentationExtension::ID = "CountingRegionExtension";
const ModelItemExtension::InfoTag CountingRegionSegmentationExtension::DISCARTED = "Discarted";

//------------------------------------------------------------------------
CountingRegionSegmentationExtension::CountingRegionSegmentationExtension()
: m_isOnEdge(false)
{
  m_availableInformations << DISCARTED;
}

//------------------------------------------------------------------------
CountingRegionSegmentationExtension::~CountingRegionSegmentationExtension()
{
}

//------------------------------------------------------------------------
ModelItemExtension::ExtId CountingRegionSegmentationExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::initialize(ModelItem::Arguments args)
{
  Sample *sample = m_seg->sample();

  ModelItem::Vector relatedChannels = sample->relatedItems(ModelItem::OUT, Channel::STAINLINK);
  Q_ASSERT(relatedChannels.size() > 0);

  QList<BoundingRegion *> regions;
  for (int i = 0; i < relatedChannels.size(); i++)
  {
    Channel *channel = dynamic_cast<Channel *>(relatedChannels[i]);
    ModelItemExtension *ext = channel->extension(CountingRegionChannelExtension::ID);
    Q_ASSERT(ext);
    CountingRegionChannelExtension *channelExt = dynamic_cast<CountingRegionChannelExtension *>(ext);
    regions << channelExt->regions();
  }

  m_isOnEdge = isOnEdge();

  setBoundingRegions(regions);

  connect(m_seg, SIGNAL(modified(ModelItem*)),
          this, SLOT(evaluateBoundingRegions()));
}

//------------------------------------------------------------------------
ModelItemExtension::ExtIdList CountingRegionSegmentationExtension::dependencies() const
{
  ExtIdList deps;
  deps << MarginsSegmentationExtension::ID;
  return deps;
}


//------------------------------------------------------------------------
QVariant CountingRegionSegmentationExtension::information(ModelItemExtension::InfoTag tag) const
{
  if (DISCARTED == tag)
  {
    QStringList discartingRegions;
    foreach(BoundingRegion *region, m_isDiscartedBy.keys())
    {
      if (m_isDiscartedBy[region])
        discartingRegions << QString::number(region->id());
    }
    return discartingRegions.join(", ");
  }

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
SegmentationRepresentation* CountingRegionSegmentationExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  Q_ASSERT(false);
  return NULL;
}


//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::setBoundingRegions(QList<BoundingRegion *> regions)
{
//   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
//   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
QSet<BoundingRegion *> prevRegions = m_isDiscartedBy.keys().toSet();
QSet<BoundingRegion *> newRegions  = regions.toSet();

// Remove regions that doesn't exist anymore
foreach(BoundingRegion *region, prevRegions.subtract(newRegions))
{
  m_isDiscartedBy.remove(region);
}

foreach(BoundingRegion *region, newRegions.subtract(prevRegions))
{
  evaluateBoundingRegion(region);
}
//   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

//------------------------------------------------------------------------
SegmentationExtension* CountingRegionSegmentationExtension::clone()
{
  return new CountingRegionSegmentationExtension();
}

//------------------------------------------------------------------------
bool CountingRegionSegmentationExtension::isDiscarted() const
{
  bool discarted = m_isOnEdge;

  if (!m_isDiscartedBy.isEmpty())
  {
    discarted = true;

    int i = 0;
    QList<BoundingRegion *> regions = m_isDiscartedBy.keys();
    while (discarted && i < regions.size())
    {
      discarted = discarted && m_isDiscartedBy[regions[i]];
      i++;
    }
  }

  return discarted;
}

//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::evaluateBoundingRegions()
{
  m_isOnEdge = isOnEdge();

  foreach(BoundingRegion *region, m_isDiscartedBy.keys())
    evaluateBoundingRegion(region);

}

//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::evaluateBoundingRegion(BoundingRegion* region)
{
  bool discarted = m_isOnEdge || isDiscartedByRegion(region);

  m_isDiscartedBy[region] = discarted;

  QString tag       = "CountingRegionCondition %1";
  QString condition = discarted?
                      "<font color=\"red\">"   + tr("Discarted by Counting Frame %1").arg(region->id()) + "</font>":
                      "<font color=\"green\">" + tr("Inside of Counting Frame %1"   ).arg(region->id()) + "</font>";
  m_seg->addCondition(tag.arg(region->id()), ":/apply.svg", condition);
}


//------------------------------------------------------------------------
bool CountingRegionSegmentationExtension::isDiscartedByRegion(BoundingRegion* boundingRegion)
{
  EspinaRegion inputBB = m_seg->volume()->espinaRegion();

  vtkPolyData  *region       = boundingRegion->region();
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
bool CountingRegionSegmentationExtension::isOnEdge()
{
  bool discarted = false;

  ModelItemExtension *ext = m_seg->extension(MarginsSegmentationExtension::ID);
  MarginsSegmentationExtension *marginExt = dynamic_cast<MarginsSegmentationExtension *>(ext);
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
bool CountingRegionSegmentationExtension::realCollision(EspinaRegion interscetion)
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

