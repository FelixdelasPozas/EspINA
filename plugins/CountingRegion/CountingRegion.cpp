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
#include "ui_CountingRegion.h"

#include <common/EspinaCore.h>
#include <common/model/Channel.h>
#include <common/gui/EspinaView.h>

#include "regions/RectangularBoundingRegion.h"
#include "regions/AdaptiveBoundingRegion.h"

// #include "CountingRegionExtension.h"

const int ADAPTIVE = 0;
const int RECTANGULAR = 1;

//------------------------------------------------------------------------
class CountingRegion::GUI
: public QWidget
, public Ui::CountingRegion
{
public:
  explicit GUI();

  void setOffsetRanges(int min, int max);

private:
  bool eventFilter(QObject *object, QEvent *event);
};

//------------------------------------------------------------------------
CountingRegion::GUI::GUI()
{
  setupUi(this);

  leftMargin->installEventFilter(this);
  rightMargin->installEventFilter(this);
  topMargin->installEventFilter(this);
  bottomMargin->installEventFilter(this);
  lowerSlice->installEventFilter(this);
  upperSlice->installEventFilter(this);
}

//------------------------------------------------------------------------
bool CountingRegion::GUI::eventFilter(QObject* object, QEvent* event)
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

//------------------------------------------------------------------------
void CountingRegion::GUI::setOffsetRanges(int min, int max)
{
  leftMargin->setMinimum(min);
  leftMargin->setMaximum(max);
  topMargin->setMinimum(min);
  topMargin->setMaximum(max);
  upperSlice->setMinimum(min);
  upperSlice->setMaximum(max);

  rightMargin->setMinimum(min);
  rightMargin->setMaximum(max);
  bottomMargin->setMinimum(min);
  bottomMargin->setMaximum(max);
  lowerSlice->setMinimum(min);
  lowerSlice->setMaximum(max);
}


const QString CountingRegion::ID = "CountingRegionExtension";

//------------------------------------------------------------------------
CountingRegion::CountingRegion(QWidget * parent)
: EspinaDockWidget(parent)
, m_gui(new GUI())
{
  setObjectName("CountingRegionDock");
  setWindowTitle(tr("Counting Region"));
  setWidget(m_gui);

  m_gui->regionView->setModel(&m_regionModel);
  m_regionModel.setHorizontalHeaderItem(0, new QStandardItem(tr("Name")));
//   m_regionModel.setHorizontalHeaderItem(1, new QStandardItem(tr("XY")));
//   m_regionModel.setHorizontalHeaderItem(2, new QStandardItem(tr("YZ")));
//   m_regionModel.setHorizontalHeaderItem(3, new QStandardItem(tr("XZ")));
//   m_regionModel.setHorizontalHeaderItem(4, new QStandardItem(tr("3D")));
  m_espinaModel = EspinaCore::instance()->model();

  connect(EspinaCore::instance(), SIGNAL(sampleSelected(Sample*)),
	  this, SLOT(sampleChanged(Sample*)));

  connect(m_gui->createRegion, SIGNAL(clicked()),
          this, SLOT(createBoundingRegion()));
  connect(m_gui->removeRegion, SIGNAL(clicked()),
          this, SLOT(removeSelectedBoundingRegion()));

  connect(m_gui->regionView, SIGNAL(clicked(QModelIndex)),
	  this, SLOT(showInfo(QModelIndex)));
  connect(&m_regionModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this, SLOT(showInfo(QModelIndex)));
  
  connect(&m_regionModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  m_gui->regionView, SLOT(setCurrentIndex(QModelIndex)));

  connect(EspinaCore::instance(), SIGNAL(currentAnalysisClosed()),
	  this, SLOT(clearBoundingRegions()));
//   connect(regionType, SIGNAL(currentIndexChanged(int)),
// 	  this, SLOT(regionTypeChanged(int)));

//   connect(&m_model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),
// 	  this, SLOT(visibilityModified()));

//   // Register Counting Brick extensions
//   SegmentationExtension segExt;
//   EspINAFactory::instance()->addSegmentationExtension(&segExt);
//   SampleExtension sampleExt;
//   EspINAFactory::instance()->addSampleExtension(&sampleExt);
//   RegionRenderer region;
//   EspINAFactory::instance()->addViewWidget(region.clone());
// //   EXTENSION_DEBUG(<< "Counting Region Panel created");
}

//------------------------------------------------------------------------
CountingRegion::~CountingRegion()
{
}

//------------------------------------------------------------------------
void CountingRegion::createAdaptiveRegion(double inclusion[3], double exclusion[3])
{
  QSharedPointer<ViewManager> vm = EspinaCore::instance()->viewManger();
  EspinaView *view = vm->currentView();

  Sample *sample = EspinaCore::instance()->sample();
  Q_ASSERT(sample);
  ModelItem::Vector channels = sample->relatedItems(ModelItem::OUT,"mark");
  Q_ASSERT(channels.size() > 0);

  Channel *channel = dynamic_cast<Channel *>(channels.first());

  AdaptiveBoundingRegion *region(new AdaptiveBoundingRegion(channel, inclusion, exclusion));
  m_regionModel.appendRow(region);
  view->addWidget(region);
}

//------------------------------------------------------------------------
void CountingRegion::createRectangularRegion(double inclusion[3], double exclusion[3])
{
  QSharedPointer<ViewManager> vm = EspinaCore::instance()->viewManger();
  EspinaView *view = vm->currentView();

  Sample *sample = EspinaCore::instance()->sample();
  Q_ASSERT(sample);

  double borders[6];
  sample->bounds(borders);

  RectangularBoundingRegion *region(new RectangularBoundingRegion(borders, inclusion, exclusion));
  m_regionModel.appendRow(region);
  view->addWidget(region);
}

//------------------------------------------------------------------------
void CountingRegion::clearBoundingRegions()
{
  m_regionModel.clear();
  m_gui->regionDescription->clear();
  m_gui->createRegion->setEnabled(false);
  m_gui->removeRegion->setEnabled(false);
}

//------------------------------------------------------------------------
void CountingRegion::createBoundingRegion()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  double inclusion[3];
  inclusion[0] = m_gui->leftMargin->value();
  inclusion[1] = m_gui->topMargin->value();
  inclusion[2] = m_gui->upperSlice->value();

  double exclusion[3];
  exclusion[0] = m_gui->rightMargin->value();
  exclusion[1] = m_gui->bottomMargin->value();
  exclusion[2] = m_gui->lowerSlice->value();

  if (m_gui->regionType->currentIndex() == ADAPTIVE)
    createAdaptiveRegion(inclusion, exclusion);
  else if (m_gui->regionType->currentIndex() == RECTANGULAR)
    createRectangularRegion(inclusion, exclusion);
  else
    Q_ASSERT(false);

  m_gui->removeRegion->setEnabled(true);

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void CountingRegion::removeSelectedBoundingRegion()
{
  int selectedRegion = m_gui->regionView->currentIndex().row();
  if (selectedRegion >= 0 && selectedRegion < m_regionModel.rowCount())
    m_regionModel.removeRow(selectedRegion);

  m_gui->regionDescription->clear();
  m_gui->removeRegion->setEnabled(m_regionModel.rowCount() > 0);
}

//------------------------------------------------------------------------
void CountingRegion::sampleChanged(Sample* sample)
{
  m_gui->createRegion->setEnabled(sample != NULL);
  if (sample)
    m_gui->setOffsetRanges(-1000,1000);
  else
    m_gui->setOffsetRanges(0,0);
}

//------------------------------------------------------------------------
void CountingRegion::showInfo(const QModelIndex& index)
{
  m_gui->regionDescription->setText(index.data(BoundingRegion::DescriptionRole).toString());
}
