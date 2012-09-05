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
#include <common/model/EspinaFactory.h>
#include <common/selection/SelectionManager.h>

#include "regions/RectangularBoundingRegion.h"
#include "regions/AdaptiveBoundingRegion.h"
#include "extensions/CountingRegionSegmentationExtension.h"
#include "extensions/CountingRegionChannelExtension.h"
#include "RegionRenderer.h"

#include <QFileDialog>

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
      preview->setPixmap(QPixmap(":/left.png"));
    else if (object == rightMargin)
      preview->setPixmap(QPixmap(":/right.png"));
    else if (object == topMargin)
      preview->setPixmap(QPixmap(":/top.png"));
    else if (object == bottomMargin)
      preview->setPixmap(QPixmap(":/bottom.png"));
    else if (object == upperSlice)
      preview->setPixmap(QPixmap(":/upper.png"));
    else if (object == lowerSlice)
      preview->setPixmap(QPixmap(":/lower.png"));

  }else if (event->type() == QEvent::FocusOut)
  {
    preview->setPixmap(QPixmap(":/allPlanes.png"));
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
: IDockWidget(parent)
, m_gui(new GUI())
{
  setObjectName("CountingRegionDock");
  setWindowTitle(tr("Counting Region"));
  setWidget(m_gui);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_gui->saveDescription->setIcon(iconSave);
  connect(m_gui->saveDescription, SIGNAL(clicked(bool)),
	  this, SLOT(saveRegionDescription()));

  ChannelExtension::SPtr channelExtension(new CountingRegionChannelExtension(this));
  EspinaFactory::instance()->registerChannelExtension(channelExtension);
  SegmentationExtension::SPtr segExtension(new CountingRegionSegmentationExtension());
  EspinaFactory::instance()->registerSegmentationExtension(segExtension);
  EspinaFactory::instance()->registerRenderer(new RegionRenderer(this));

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
}

//------------------------------------------------------------------------
CountingRegion::~CountingRegion()
{
}

//------------------------------------------------------------------------
void CountingRegion::createAdaptiveRegion(Channel *channel,
                                          Nm inclusion[3],
                                          Nm exclusion[3])
{
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();

  ModelItemExtension *ext = channel->extension(CountingRegionChannelExtension::ID);
  Q_ASSERT(ext);
  CountingRegionChannelExtension *channelExt = dynamic_cast<CountingRegionChannelExtension *>(ext);
  Q_ASSERT(channelExt);

  AdaptiveBoundingRegion *region(new AdaptiveBoundingRegion(channelExt, inclusion, exclusion));
  channelExt->addRegion(region);
  m_regionModel.appendRow(region);
  view->addWidget(region);
  m_gui->removeRegion->setEnabled(true);
  m_regions << region;
  emit regionCreated(region);
}

//------------------------------------------------------------------------
void CountingRegion::createRectangularRegion(Channel *channel,
                                             Nm inclusion[3],
                                             Nm exclusion[3])
{
  EspinaView *view = EspinaCore::instance()->viewManger()->currentView();

  ModelItemExtension *ext = channel->extension(CountingRegionChannelExtension::ID);
  Q_ASSERT(ext);
  CountingRegionChannelExtension *channelExt = dynamic_cast<CountingRegionChannelExtension *>(ext);
  Q_ASSERT(channelExt);

  double borders[6];
  channel->bounds(borders);

  RectangularBoundingRegion *region(new RectangularBoundingRegion(channelExt, borders, inclusion, exclusion));
  channelExt->addRegion(region);
  m_regionModel.appendRow(region);
  view->addWidget(region);
  m_gui->removeRegion->setEnabled(true);
  m_regions << region;
  emit regionCreated(region);
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

  Nm inclusion[3];
  inclusion[0] = m_gui->leftMargin->value();
  inclusion[1] = m_gui->topMargin->value();
  inclusion[2] = m_gui->upperSlice->value();

  Nm exclusion[3];
  exclusion[0] = m_gui->rightMargin->value();
  exclusion[1] = m_gui->bottomMargin->value();
  exclusion[2] = m_gui->lowerSlice->value();

  Channel *channel = SelectionManager::instance()->activeChannel();
  Q_ASSERT(channel);

  if (ADAPTIVE == m_gui->regionType->currentIndex())
    createAdaptiveRegion(channel, inclusion, exclusion);
  else if (RECTANGULAR == m_gui->regionType->currentIndex())
    createRectangularRegion(channel, inclusion, exclusion);
  else
    Q_ASSERT(false);

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void CountingRegion::removeSelectedBoundingRegion()
{
  int selectedRegion = m_gui->regionView->currentIndex().row();
  if (selectedRegion >= 0 && selectedRegion < m_regionModel.rowCount())
  {
    QStandardItem *item = m_regionModel.item(selectedRegion);
    BoundingRegion *region = dynamic_cast<BoundingRegion *>(item);
    Q_ASSERT(region);
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    view->removeWidget(region);
    m_regions.removeAll(region);
    emit regionRemoved(region);
    delete region;
    view->forceRender();
    m_regionModel.removeRow(selectedRegion);
  }

  m_gui->regionDescription->clear();
  m_gui->saveDescription->setEnabled(false);
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
  m_gui->saveDescription->setEnabled(index.isValid());
}

//------------------------------------------------------------------------
void CountingRegion::saveRegionDescription()
{
  QString title   = tr("Save Counting Region Description");
  QString fileExt = tr("Text File (*.txt)");
  QString fileName = QFileDialog::getSaveFileName(this, title, "", fileExt);

  if (!fileName.isEmpty())
  {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly |  QIODevice::Text);
    QTextStream out(&file);
    out << m_gui->regionDescription->toPlainText();
    file.close();
  }
}

Q_EXPORT_PLUGIN2(CountingRegionPlugin, CountingRegion)