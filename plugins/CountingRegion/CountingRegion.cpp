/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "CountingRegion.h"
#include "CountingRegionExtension.h"

#include "espINAFactory.h"
#include "espina.h"

#include "filter.h"


#include "cache/cachedObjectBuilder.h"

// ParaView
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>

// Debug
#include <QDebug>
#include <assert.h>
#include "RegionRenderer.h"

CountingRegion::CountingRegion(QWidget * parent): QDockWidget(parent)
{
  this->setWindowTitle(tr("Counting Brick"));
  QWidget *dockWidget = new QWidget();
  setupUi(dockWidget);
  setWidget(dockWidget);

  connect(createRegion, SIGNAL(clicked()),
          this, SLOT(createNewRegion()));

  CountingRegionExtension ext(this);
  EspINAFactory::instance()->addSegmentationExtension(&ext);
  RegionRenderer widget(m_regions);
  EspINAFactory::instance()->addViewWidget(&widget);
  

  regions->setModel(&m_regionsModel);

  connect(EspINA::instance(), SIGNAL(focusSampleChanged(Sample*)),
          this, SLOT(focusSampleChanged(Sample *)));

  std::cout << "Creating Plugin" << std::endl;
}

//! Add existing bounding areas to the new Counting Region Extension
void CountingRegion::initializeExtension(CountingRegionExtension* ext)
{
  qDebug() << "Initializing Managed Extensions";
  qDebug() << ext->segmentation()->name << " of " << ext->segmentation()->origin()->id();
  ext->updateRegions(m_regions[ext->segmentation()->origin()]);
}


//! Updates the GUI to show the information of the current
//! focused selection
void CountingRegion::focusSampleChanged(Sample* sample)
{
  createRegion->setEnabled(sample != NULL);
  if (createRegion->isEnabled())
  {
    int extent[6];
    sample->extent(extent);
    leftMargin->setMinimum(extent[0]);
    leftMargin->setMaximum(extent[1]);
    rightMargin->setMinimum(extent[0]);
    rightMargin->setMaximum(extent[1]);
    topMargin->setMinimum(extent[2]);
    topMargin->setMaximum(extent[3]);
    bottomMargin->setMinimum(extent[2]);
    bottomMargin->setMaximum(extent[3]);
    upperSlice->setMinimum(extent[4]);
    upperSlice->setMaximum(extent[5]);
    upperSlice->setValue(extent[5]);
    lowerSlice->setMinimum(extent[4]);
    lowerSlice->setMaximum(extent[5]);
  }
}

//! Creates a bounding region on the current focused/active
//! sample and update all their segmentations counting
//! regions extension
void CountingRegion::createNewRegion()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  Sample *sample = EspINA::instance()->activeSample();

  // Configuration of Bounding Region interface
  VtkParamList args;
  VtkArg arg;
  arg.name = "Input";
  arg.type = INPUT;
  VtkParam param;
  param.first = arg;
  param.second = sample->id();
  args.push_back(param);
  arg.name = "Inclusion";
  arg.type = INTVECT;
  param.first = arg;
  param.second = QString("%1,%2,%3")
                 .arg(leftMargin->value())
                 .arg(topMargin->value())
                 .arg(upperSlice->value());
  args.push_back(param);
  arg.name = "Exclusion";
  arg.type = INTVECT;
  param.first = arg;
  param.second = QString("%1,%2,%3")
                 .arg(rightMargin->value())
                 .arg(bottomMargin->value())
                 .arg(lowerSlice->value());
  args.push_back(param);

  pqPipelineSource *region = cob->createFilter("filters", "BoundingRegion", args);
  if (!region)
  {
    qDebug()
    << "Couldn't create Bounding Region Filter";
    assert(false);
  }
  m_regions[sample].push_back(region);

  bool updateOk = updateRegions(sample);

  regions->addItem(QString("Counting Brick (%1,%2,%3,%4,%5,%6)")
                   .arg(leftMargin->value())
                   .arg(topMargin->value())
                   .arg(upperSlice->value())
                   .arg(rightMargin->value())
                   .arg(bottomMargin->value())
                   .arg(lowerSlice->value())
                  );
}

//! Update all sample's segmentations' filter with the new set of
//! bounding regions
bool CountingRegion::updateRegions(Sample* sample)
{
  foreach(Segmentation *seg, EspINA::instance()->segmentations(sample))
  {
    CountingRegionExtension *ext = dynamic_cast<CountingRegionExtension *>(
                                     seg->extension(CountingRegionExtension::ID));
    if (!ext)
    {
      qDebug() << "Failed to load Counting Brick Extension on " << seg->name;
      assert(false);
    }
    ext->updateRegions(m_regions[sample]);
  }
}

