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

#include <common/EspinaRegions.h>
#include <common/extensions/Margins/MarginsSegmentationExtension.h>
#include <common/model/Segmentation.h>
#include <common/model/Sample.h>
#include <common/model/Channel.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>

#include <QDebug>
#include <QApplication>

CountingRegionSegmentationExtension::BoundingBox::BoundingBox(vtkPoints* points)
{
  Q_ASSERT(points->GetNumberOfPoints());
  // Init Bounding Box
  double bounds[6];
  points->GetBounds(bounds);
  xMin = bounds[0];
  xMax = bounds[1];
  yMin = bounds[2];
  yMax = bounds[3];
  zMin = bounds[4];
  zMax = bounds[5];
}

CountingRegionSegmentationExtension::BoundingBox::BoundingBox(EspinaVolume* image)
{
  double bounds[6];
  VolumeBounds(image, bounds);
  xMin = bounds[0];
  xMax = bounds[1];
  yMin = bounds[2];
  yMax = bounds[3];
  zMin = bounds[4];
  zMax = bounds[5];
}


bool CountingRegionSegmentationExtension::BoundingBox::intersect(BoundingBox& bb)
{
  bool xOverlap = xMin <= bb.xMax && xMax >= bb.xMin;
  bool yOverlap = yMin <= bb.yMax && yMax >= bb.yMin;
  bool zOverlap = zMin <= bb.zMax && zMax >= bb.zMin;

  return xOverlap && yOverlap && zOverlap;
}

CountingRegionSegmentationExtension::BoundingBox CountingRegionSegmentationExtension::BoundingBox::intersection(BoundingBox& bb)
{
  BoundingBox res;
  res.xMin = std::max(xMin, bb.xMin);
  res.xMax = std::min(xMax, bb.xMax);
  res.yMin = std::max(yMin, bb.yMin);
  res.yMax = std::min(yMax, bb.yMax);
  res.zMin = std::max(zMin, bb.zMin);
  res.zMax = std::min(zMax, bb.zMax);
  return res;
}


const ModelItemExtension::ExtId CountingRegionSegmentationExtension::ID = "CountingRegionExtension";

const ModelItemExtension::InfoTag CountingRegionSegmentationExtension::DISCARTED = "Discarted";

//------------------------------------------------------------------------
CountingRegionSegmentationExtension::CountingRegionSegmentationExtension()
: m_isDiscarted(false)
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
  ModelItem::Vector relatedSamples = m_seg->relatedItems(ModelItem::IN, Sample::WHERE);
  Q_ASSERT(relatedSamples.size() == 1);
  Sample *sample = dynamic_cast<Sample *>(relatedSamples[0]);
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
    return m_isDiscarted;
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
void CountingRegionSegmentationExtension::setBoundingRegions(QList<BoundingRegion *> bRegions)
{
//   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
//   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
  m_boundingRegions = bRegions;
  evaluateBoundingRegions();
//   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

//------------------------------------------------------------------------
SegmentationExtension* CountingRegionSegmentationExtension::clone()
{
  return new CountingRegionSegmentationExtension();
}

//------------------------------------------------------------------------
bool CountingRegionSegmentationExtension::discartedByRegion(BoundingBox inputBB, vtkPolyData* region)
{
  vtkPoints *regionPoints = region->GetPoints();
  vtkCellArray *regionFaces = region->GetPolys();
  vtkCellData *faceData = region->GetCellData();

  BoundingBox regionBB(regionPoints);

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

    BoundingBox faceBB(facePoints);
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

    BoundingBox sliceBB(slicePoints);
    if (inputBB.intersect(sliceBB) &&  realCollision(inputBB.intersection(sliceBB)))
      return false;//;
  }

  // If no internal collision was detected, then the input was indeed outside our
  // bounding region
  return true;
}

//------------------------------------------------------------------------
bool CountingRegionSegmentationExtension::realCollision(BoundingBox interscetion)
{
  EspinaVolume *input = m_seg->itkVolume();
  for (int z = interscetion.zMin; z <= interscetion.zMax; z++)
    for (int y = interscetion.yMin; y <= interscetion.yMax; y++)
      for (int x = interscetion.xMin; x <= interscetion.xMax; x++)
      {
	EspinaVolume::IndexType index = m_seg->index(x, y, z);
	if (input->GetLargestPossibleRegion().IsInside(index) && input->GetPixel(index))
	  return true;
      }

  return false;
}


//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::evaluateBoundingRegions()
{
  m_isDiscarted = false;

  if (m_boundingRegions.size() == 0)
    return;

//   qDebug() << "EValuate Region";
  ModelItemExtension *ext = m_seg->extension(MarginsSegmentationExtension::ID);
  MarginsSegmentationExtension *marginExt = dynamic_cast<MarginsSegmentationExtension *>(ext);
  if (marginExt)
  {
    InfoList tags = marginExt->availableInformations();
    int i = 0;
    while (!m_isDiscarted && i < tags.size())
    {
      bool ok = false;
      double dist = m_seg->information(tags[i++]).toDouble(&ok);
      m_isDiscarted = ok && dist < 1.0;
    }
  }

  if (!m_isDiscarted)
  {
    BoundingBox inputBB(m_seg->itkVolume());

    foreach(BoundingRegion *br, m_boundingRegions)
      m_isDiscarted |= discartedByRegion(inputBB, br->region());
  }
  QString condition = m_isDiscarted?
                      "<font color=\"red\">Outside</font>":
                      "<font color=\"green\">Inside</font>";
  m_seg->addCondition("CountingRegionCondition", ":/apply.svg", condition);
}