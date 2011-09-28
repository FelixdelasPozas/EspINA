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

const int ADAPTIVE = 0;
const int RECTANGULAR = 1;

const QString CountingRegion::ID = "CountingRegionExtension";

CountingRegion::CountingRegion(QWidget * parent): QDockWidget(parent)
, m_model(0,5)
{
  // Create UI
  this->setWindowTitle(tr("Counting Brick"));
  QWidget *dockWidget = new QWidget();
  setupUi(dockWidget);
  setWidget(dockWidget);
  
  leftMargin->installEventFilter(this);
  rightMargin->installEventFilter(this);
  topMargin->installEventFilter(this);
  bottomMargin->installEventFilter(this);
  lowerSlice->installEventFilter(this);
  upperSlice->installEventFilter(this);

  regionView->setModel(&m_model);
  regionCombo->setModel(&m_model);
  resetRegionsModel();

  connect(createRegion, SIGNAL(clicked()),
          this, SLOT(createBoundingRegion()));
  connect(removeRegion, SIGNAL(clicked()),
          this, SLOT(removeBoundingRegion()));
  connect(regionType, SIGNAL(currentIndexChanged(int)),
	  this, SLOT(regionTypeChanged(int)));

  connect(EspINA::instance(), SIGNAL(focusSampleChanged(Sample*)),
          this, SLOT(focusSampleChanged(Sample *)));
  connect(&m_model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this, SLOT(visibilityModified()));

  // Register Counting Brick extensions
  SegmentationExtension segExt;
  EspINAFactory::instance()->addSegmentationExtension(&segExt);
  SampleExtension sampleExt;
  EspINAFactory::instance()->addSampleExtension(&sampleExt);
  RegionRenderer region;
  EspINAFactory::instance()->addViewWidget(region.clone());
//   EXTENSION_DEBUG(<< "Counting Region Panel created");
}

//! Add existing bounding areas to the new Counting Region Extension
void CountingRegion::initializeExtension(SegmentationExtension* ext)
{
  
//   EXTENSION_DEBUG(<< "Adding existing bounding areas to new counting region");
//   EXTENSION_DEBUG(<< ext->segmentation()->name << " of " << ext->segmentation()->origin()->id());
  
//   ext->updateRegions(m_regions[ext->segmentation()->origin()]);
}

//! Changes the image displayed to show which plane is being modified
bool CountingRegion::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::FocusIn)
  {
    if (object == leftMargin)
      preview->setPixmap(QPixmap(":/espina/left.png"));
    else if (object == rightMargin)
      preview->setPixmap(QPixmap(":/espina/right.png"));
    else if (object == topMargin)
      preview->setPixmap(QPixmap(":/espina/top.png"));
    else if (object == bottomMargin)
      preview->setPixmap(QPixmap(":/espina/bottom.png"));
    else if (object == upperSlice)
      preview->setPixmap(QPixmap(":/espina/upper.png"));
    else if (object == lowerSlice)
      preview->setPixmap(QPixmap(":/espina/lower.png"));
      
  }else if (event->type() == QEvent::FocusOut)
  {
      preview->setPixmap(QPixmap(":/espina/allPlanes.png"));
  }
  return QObject::eventFilter(object, event);
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
    regionTypeChanged(regionType->currentIndex());
    m_focusedSample = sample;
    
  
    resetRegionsModel();
  
    QStandardItem *sampleItem = new QStandardItem(sample->data(Qt::DisplayRole).toString());
    m_parentItem->appendRow(sampleItem);
    m_parentItem = sampleItem;
    m_parentItem->setColumnCount(3);
    regionCombo->setRootModelIndex(m_model.index(0,0,QModelIndex()));
  }
}

void CountingRegion::regionTypeChanged(int type)
{
  switch (type)
  {
    case ADAPTIVE:
    case RECTANGULAR:
      leftMargin->setValue(0);
      rightMargin->setValue(0);
      upperSlice->setValue(0);
      topMargin->setValue(0);
      bottomMargin->setValue(0);
      lowerSlice->setValue(0);
      break;
//       leftMargin->setValue(leftMargin->minimum());
//       rightMargin->setValue(rightMargin->maximum());
//       topMargin->setValue(topMargin->maximum());
//       bottomMargin->setValue(bottomMargin->minimum());
//       break;
    default:
      assert(false);
  };
}


//! Creates a bounding region on the current focused/active
//! sample and update all their segmentations counting regions
//! extension discarting those that are out of the region
void CountingRegion::createBoundingRegion()
{
  SampleExtension *ext = dynamic_cast<SampleExtension *>(
    m_focusedSample->extension(CountingRegion::ID));
  assert(ext); 
  
  QList<QStandardItem *> row;
  if (regionType->currentIndex() == ADAPTIVE)
  {
    ext->createAdaptiveRegion(
      leftMargin->value(),  topMargin->value(),    upperSlice->value(),
      rightMargin->value(), bottomMargin->value(), lowerSlice->value(),
      row);
  } else if (regionType->currentIndex() == RECTANGULAR)
  {
    ext->createRectangularRegion(
      leftMargin->value(),  topMargin->value(),    upperSlice->value(),
      rightMargin->value(), bottomMargin->value(), lowerSlice->value(),
      row);
  } else
    assert(false);
  
  m_parentItem->appendRow(row);
  
  removeRegion->setEnabled(true);
  
  // Update information displayed in the dock
  regionView->expandAll();
  QStandardItem *descItem = row.last();
  regionDescription->setText(descItem->data(Qt::DisplayRole).toString());
}

//------------------------------------------------------------------------
void CountingRegion::removeBoundingRegion()
{
  SampleExtension *ext = dynamic_cast<SampleExtension *>(m_focusedSample->extension(CountingRegion::ID));
  assert(ext); 
  
  QModelIndex region = regionView->currentIndex().sibling(regionView->currentIndex().row(),0);
  if (!region.parent().isValid()) 
    return;// Sample Node
    
  QString regionName = region.data(Qt::DisplayRole).toString();
  ext->removeRegion(regionName);
  m_model.removeRow(region.row(),region.parent());
  
  removeRegion->setEnabled(false);
 
  for (int r = 0; r < m_model.rowCount(); r++)
  {
    QModelIndex sample = m_model.index(r,0);
    if (m_model.rowCount(sample))
      removeRegion->setEnabled(true);
  }
}

//------------------------------------------------------------------------
void CountingRegion::visibilityModified()
{
  EspINA::instance()->activeSample()->notifyInternalUpdate();
}

//------------------------------------------------------------------------
void CountingRegion::resetRegionsModel()
{
  //TODO: Manage load events/unload
  m_model.clear();
  m_model.setHorizontalHeaderItem(0, new QStandardItem(tr("Name")));
  m_model.setHorizontalHeaderItem(1, new QStandardItem(tr("XY")));
  m_model.setHorizontalHeaderItem(2, new QStandardItem(tr("YZ")));
  m_model.setHorizontalHeaderItem(3, new QStandardItem(tr("XZ")));
  m_model.setHorizontalHeaderItem(4, new QStandardItem(tr("3D")));
  m_parentItem = m_model.invisibleRootItem();
  // END_ToDo
}

