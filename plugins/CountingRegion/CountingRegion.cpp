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

#include "RegionRenderer.h"

// Debug
#include "espina_debug.h"

// EspINA
// #include "CountingRegionExtension.h"

#include "espina.h"
#include "espINAFactory.h"

#include "filter.h"
#include "sample.h"
#include "segmentation.h"

#include "cache/cachedObjectBuilder.h"

// ParaView
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>

const QString CountingRegion::ID = "CountingRegionExtension";

CountingRegion::CountingRegion(QWidget * parent): QDockWidget(parent)
{
  this->setWindowTitle(tr("Counting Brick"));
  QWidget *dockWidget = new QWidget();
  setupUi(dockWidget);
  setWidget(dockWidget);

  connect(rectangularRegion, SIGNAL(clicked()),
          this, SLOT(createRectangularRegion()));
  connect(adaptativeRegion, SIGNAL(clicked()),
          this, SLOT(createAdaptativeRegion()));

  SegmentationExtension segExt;
  EspINAFactory::instance()->addSegmentationExtension(&segExt);
  SampleExtension sampleExt;
  EspINAFactory::instance()->addSampleExtension(&sampleExt);
  RegionRenderer region;
  EspINAFactory::instance()->addViewWidget(region.clone());
//   
  regions->setModel(&m_regionsModel);

  regionView->setModel(&m_model);
  m_parentItem = m_model.invisibleRootItem();

  connect(EspINA::instance(), SIGNAL(focusSampleChanged(Sample*)),
          this, SLOT(focusSampleChanged(Sample *)));
//   EXTENSION_DEBUG(<< "Counting Region Panel created");
}

//! Add existing bounding areas to the new Counting Region Extension
void CountingRegion::initializeExtension(SegmentationExtension* ext)
{
  
//   EXTENSION_DEBUG(<< "Adding existing bounding areas to new counting region");
//   EXTENSION_DEBUG(<< ext->segmentation()->name << " of " << ext->segmentation()->origin()->id());
  
//   ext->updateRegions(m_regions[ext->segmentation()->origin()]);
}


//! Updates the GUI to show the information of the current
//! focused selection
void CountingRegion::focusSampleChanged(Sample* sample)
{
  rectangularRegion->setEnabled(sample != NULL);
  adaptativeRegion->setEnabled(sample != NULL);
  if (rectangularRegion->isEnabled())
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
    m_focusedSample = sample;
    
    QStandardItem *sampleItem = new QStandardItem(sample->data(Qt::DisplayRole).toString());
    m_parentItem->appendRow(sampleItem);
    m_parentItem = sampleItem;
  }
}

void CountingRegion::createAdaptativeRegion()
{
  SampleExtension *ext = dynamic_cast<SampleExtension *>(m_focusedSample->extension(CountingRegion::ID));
  assert(ext); 
  
  ext->createAdaptativeRegion(leftMargin->value(),topMargin->value(), upperSlice->value(),
		       rightMargin->value(), bottomMargin->value(), lowerSlice->value());
  
  QStandardItem *regionItem = new QStandardItem(
    QString("Adaptative Counting Brick (%1,%2,%3,%4,%5,%6)")
    .arg(leftMargin->value())
    .arg(topMargin->value())
    .arg(upperSlice->value())
    .arg(rightMargin->value())
    .arg(bottomMargin->value())
    .arg(lowerSlice->value())
  );
  m_parentItem->appendRow(regionItem);
}


//! Creates a bounding region on the current focused/active
//! sample and update all their segmentations counting
//! regions extension
void CountingRegion::createRectangularRegion()
{
  SampleExtension *ext = dynamic_cast<SampleExtension *>(m_focusedSample->extension(CountingRegion::ID));
  assert(ext); 
  
  ext->createRectangularRegion(leftMargin->value(),topMargin->value(), upperSlice->value(),
		       rightMargin->value(), bottomMargin->value(), lowerSlice->value());
    
  QStandardItem *regionItem = new QStandardItem(
    QString("Rectangular Counting Brick (%1,%2,%3,%4,%5,%6)")
    .arg(leftMargin->value())
    .arg(topMargin->value())
    .arg(upperSlice->value())
    .arg(rightMargin->value())
    .arg(bottomMargin->value())
    .arg(lowerSlice->value())
  );
  
  QList<QStandardItem *> list;
  list << new QStandardItem("hola");
  list << new QStandardItem("que");
  list << new QStandardItem("tal");
  regionItem->appendColumn(list);
  m_parentItem->appendRow(regionItem);
}


