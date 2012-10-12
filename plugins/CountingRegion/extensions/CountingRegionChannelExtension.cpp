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


#include "CountingRegionChannelExtension.h"

#include "CountingRegionSegmentationExtension.h"
#include "regions/BoundingRegion.h"
#include <regions/RectangularBoundingRegion.h>
#include <regions/AdaptiveBoundingRegion.h>
#include <CountingRegion.h>

#include <common/model/Sample.h>
#include <common/model/Channel.h>
#include <common/extensions/Margins/MarginsSegmentationExtension.h>
#include <common/gui/ViewManager.h>

#include <QDebug>

typedef ModelItem::ArgumentId ArgumentId;

const ModelItemExtension::ExtId CountingRegionChannelExtension::ID = "CountingRegionExtension";

const ArgumentId CountingRegionChannelExtension::REGIONS = "Regions";

//-----------------------------------------------------------------------------
CountingRegionChannelExtension::CountingRegionChannelExtension(CountingRegion* plugin,
                                                               ViewManager *vm)
: m_plugin(plugin)
, m_viewManager(vm)
{

}

//-----------------------------------------------------------------------------
CountingRegionChannelExtension::~CountingRegionChannelExtension()
{

}

//-----------------------------------------------------------------------------
void CountingRegionChannelExtension::initialize(ModelItem::Arguments args)
{
  QStringList regions = args[REGIONS].split(";");

  foreach (QString region, regions)
  {
    if (region.isEmpty())
      continue;

    QString type = region.section('=',0,0);
    QStringList margins = region.section('=',-1).split(',');
    Nm inclusion[3], exclusion[3];
    for (int i=0; i<3; i++)
    {
      inclusion[i] = margins[i].toDouble();
      exclusion[i] = margins[3+i].toDouble();
    }
    if (RectangularBoundingRegion::ID == type)
      m_plugin->createRectangularRegion(m_channel, inclusion, exclusion);
    else if (AdaptiveBoundingRegion::ID == type)
      m_plugin->createAdaptiveRegion(m_channel, inclusion, exclusion);
  }

  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
QString CountingRegionChannelExtension::serialize() const
{
  QStringList regionValue;
  foreach(BoundingRegion *region, m_regions)
    regionValue << region->serialize();

  m_args[REGIONS] = "[" + regionValue.join(";") + "]";
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
ModelItemExtension::ExtIdList CountingRegionChannelExtension::dependencies() const
{
  ExtIdList deps;
  deps << MarginsSegmentationExtension::ID;
  return deps;
}

//-----------------------------------------------------------------------------
ChannelExtension* CountingRegionChannelExtension::clone()
{
  return new CountingRegionChannelExtension(m_plugin, m_viewManager);
}

//-----------------------------------------------------------------------------
void CountingRegionChannelExtension::addRegion(BoundingRegion* region)
{
  Q_ASSERT(!m_regions.contains(region));
  m_regions << region;

  Sample *sample = m_channel->sample();
  Q_ASSERT(sample);
  ModelItem::Vector items = sample->relatedItems(ModelItem::OUT, "where");
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
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}

//-----------------------------------------------------------------------------
void CountingRegionChannelExtension::removeRegion(BoundingRegion* region)
{
  Q_ASSERT(m_regions.contains(region));
  m_regions.removeOne(region);

  Sample *sample = m_channel->sample();
  Q_ASSERT(sample);
  ModelItem::Vector items = sample->relatedItems(ModelItem::OUT, "where");
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
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}