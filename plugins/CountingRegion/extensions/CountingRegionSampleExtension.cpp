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


#include "CountingRegionSampleExtension.h"

#include "CountingRegion.h"
#include "CountingRegionSegmentationExtension.h"
#include "regions/RectangularBoundingRegion.h"

#include <common/model/ModelItem.h>
#include <common/model/Sample.h>

typedef ModelItem::ArgumentId ArgumentId;

const ModelItemExtension::ExtId CountingRegionSampleExtension::ID = "CountingRegionExtension";

const ArgumentId CountingRegionSampleExtension::REGIONS = ArgumentId("Regions", ArgumentId::VARIABLE);

//-----------------------------------------------------------------------------
CountingRegionSampleExtension::CountingRegionSampleExtension(CountingRegion *plugin)
: m_plugin(plugin)
{
}

//-----------------------------------------------------------------------------
CountingRegionSampleExtension::~CountingRegionSampleExtension()
{

}

//-----------------------------------------------------------------------------
ModelItemExtension::ExtId CountingRegionSampleExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void CountingRegionSampleExtension::initialize(Sample* sample,
					       ModelItem::Arguments args)
{
  m_sample = sample;
  QStringList regions = args[REGIONS].split(";");

  foreach (QString region, regions)
  {
    if (region.isEmpty())
      continue;

    QString type = region.section('=',0,0);
    QStringList margins = region.section('=',-1).split(',');
    double inclusion[3], exclusion[3];
    for (int i=0; i<3; i++)
    {
      inclusion[i] = margins[i].toInt();
      exclusion[i] = margins[3+i].toInt();
    }
    if (type == "RectangularRegion")
      m_plugin->createRectangularRegion(inclusion, exclusion);
    //TODO
//       createRectangularRegion(inclusion[0],inclusion[1],inclusion[2],
// 			      exclusion[0], exclusion[1], exclusion[2], row);
//       else if (type == AdaptiveRegion::ID)
// 	createAdaptiveRegion(inclusion[0],inclusion[1],inclusion[2],
// 			     exclusion[0], exclusion[1], exclusion[2], row);
  }
}

//-----------------------------------------------------------------------------
SampleExtension *CountingRegionSampleExtension::clone()
{
  return new CountingRegionSampleExtension(m_plugin);
}

//-----------------------------------------------------------------------------
void CountingRegionSampleExtension::addRegion(BoundingRegion* region)
{
  Q_ASSERT(!m_regions.contains(region));
  m_regions << region;

  ModelItem::Vector items = m_sample->relatedItems(ModelItem::OUT, "where");
  foreach(ModelItem *item, items)
  {
    if (ModelItem::SEGMENTATION == item->type())
    {
      ModelItemExtension *ext = item->extension(CountingRegionSegmentationExtension::ID);
      Q_ASSERT(ext);
      CountingRegionSegmentationExtension *segExt = dynamic_cast<CountingRegionSegmentationExtension *>(ext);
      segExt->setBoundingRegions(m_regions);
    }
  }
}

//-----------------------------------------------------------------------------
void CountingRegionSampleExtension::removeRegion(BoundingRegion* region)
{
  Q_ASSERT(m_regions.contains(region));
  m_regions.removeOne(region);

  ModelItem::Vector items = m_sample->relatedItems(ModelItem::OUT, "where");
  foreach(ModelItem *item, items)
  {
    if (ModelItem::SEGMENTATION == item->type())
    {
      ModelItemExtension *ext = item->extension(CountingRegionSegmentationExtension::ID);
      Q_ASSERT(ext);
      CountingRegionSegmentationExtension *segExt = dynamic_cast<CountingRegionSegmentationExtension *>(ext);
      segExt->setBoundingRegions(m_regions);
    }
  }
}
