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

#include <common/model/ModelItem.h>
#include <common/model/Sample.h>

#include "CountingRegionSegmentationExtension.h"

const QString CountingRegionSampleExtension::ID = "CountingRegionExtension";

//-----------------------------------------------------------------------------
CountingRegionSampleExtension::CountingRegionSampleExtension()
{

}

//-----------------------------------------------------------------------------
CountingRegionSampleExtension::~CountingRegionSampleExtension()
{

}

//-----------------------------------------------------------------------------
QString CountingRegionSampleExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void CountingRegionSampleExtension::initialize(Sample* sample)
{
  m_sample = sample;
}

//-----------------------------------------------------------------------------
SampleExtension *CountingRegionSampleExtension::clone()
{
  return new CountingRegionSampleExtension();
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
      segExt->updateRegions(m_regions);
    }
  }
}
