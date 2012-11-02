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

#include <common/model/Channel.h>
#include <common/model/EspinaModel.h>
#include <common/model/EspinaFactory.h>
#include <common/model/Segmentation.h>
#include <common/gui/ViewManager.h>
#include <common/extensions/Margins/MarginsSegmentationExtension.h>
#include <common/EspinaRegions.h>

#include "regions/RectangularBoundingRegion.h"
#include "regions/AdaptiveBoundingRegion.h"
#include "extensions/CountingRegionSegmentationExtension.h"
#include "extensions/CountingRegionChannelExtension.h"
#include "RegionRenderer.h"
#include "colorEngines/CountingRegionColorEngine.h"

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
, m_espinaModel(NULL)
, m_viewManager(NULL)
{
  setObjectName("CountingRegionDock");
  setWindowTitle(tr("Counting Region"));
  setWidget(m_gui);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_gui->saveDescription->setIcon(iconSave);
  connect(m_gui->saveDescription, SIGNAL(clicked(bool)),
          this, SLOT(saveRegionDescription()));


  m_gui->regionView->setModel(&m_regionModel);
  m_regionModel.setHorizontalHeaderItem(0, new QStandardItem(tr("Name")));
//   m_regionModel.setHorizontalHeaderItem(1, new QStandardItem(tr("XY")));
//   m_regionModel.setHorizontalHeaderItem(2, new QStandardItem(tr("YZ")));
//   m_regionModel.setHorizontalHeaderItem(3, new QStandardItem(tr("XZ")));
//   m_regionModel.setHorizontalHeaderItem(4, new QStandardItem(tr("3D")));

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

}

//------------------------------------------------------------------------
CountingRegion::~CountingRegion()
{
}

//------------------------------------------------------------------------
void CountingRegion::initDockWidget(EspinaModel* model, QUndoStack* undoStack, ViewManager* viewManager)
{
  m_espinaModel = model;
  m_viewManager = viewManager;
  connect(m_viewManager, SIGNAL(activeChannelChanged(Channel*)),
          this, SLOT(channelChanged(Channel*)));

  ChannelExtension::SPtr channelExtension(new CountingRegionChannelExtension(this, m_viewManager));
  m_espinaModel->factory()->registerChannelExtension(channelExtension);
  SegmentationExtension::SPtr segExtension(new CountingRegionSegmentationExtension());
  m_espinaModel->factory()->registerSegmentationExtension(segExtension);
  m_espinaModel->factory()->registerRenderer(new RegionRenderer(this));
}

//------------------------------------------------------------------------
IColorEngineProvider::EngineList CountingRegion::colorEngines()
{
  EngineList engines;
  engines << Engine(tr("Counting Region"), ColorEnginePtr(new CountingRegionColorEngine()));

  return engines;
}

//------------------------------------------------------------------------
void CountingRegion::reset()
{
  clearBoundingRegions();
}

//------------------------------------------------------------------------
void CountingRegion::createAdaptiveRegion(Channel *channel,
                                          Nm inclusion[3],
                                          Nm exclusion[3])
{
  ModelItemExtension *ext = channel->extension(CountingRegionChannelExtension::ID);
  Q_ASSERT(ext);
  CountingRegionChannelExtension *channelExt = dynamic_cast<CountingRegionChannelExtension *>(ext);
  Q_ASSERT(channelExt);

  AdaptiveBoundingRegion *region(new AdaptiveBoundingRegion(channelExt,
                                                            inclusion,
                                                            exclusion,
                                                            m_viewManager));
  channelExt->addRegion(region);
  m_regionModel.appendRow(region);
  m_viewManager->addWidget(region);
  m_gui->removeRegion->setEnabled(true);
  m_regions << region;
  emit regionCreated(region);
}

//------------------------------------------------------------------------
void CountingRegion::createRectangularRegion(Channel *channel,
                                             Nm inclusion[3],
                                             Nm exclusion[3])
{
  ModelItemExtension *ext = channel->extension(CountingRegionChannelExtension::ID);
  Q_ASSERT(ext);
  CountingRegionChannelExtension *channelExt = dynamic_cast<CountingRegionChannelExtension *>(ext);
  Q_ASSERT(channelExt);

  double borders[6];
  channel->bounds(borders);

  RectangularBoundingRegion *region(new RectangularBoundingRegion(channelExt,
                                                                  borders,
                                                                  inclusion,
                                                                  exclusion,
                                                                  m_viewManager));
  channelExt->addRegion(region);
  m_regionModel.appendRow(region);
  m_viewManager->addWidget(region);
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

  Channel *channel = m_viewManager->activeChannel();
  Q_ASSERT(channel);

  computeOptimalMargins(channel, inclusion, exclusion);

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
    m_viewManager->removeWidget(region);
    m_regions.removeAll(region);
    emit regionRemoved(region);
    delete region;
    m_viewManager->updateViews();
    m_regionModel.removeRow(selectedRegion);
  }

  m_gui->regionDescription->clear();
  m_gui->saveDescription->setEnabled(false);
  m_gui->removeRegion->setEnabled(m_regionModel.rowCount() > 0);
}

//------------------------------------------------------------------------
void CountingRegion::channelChanged(Channel* channel)
{
  m_gui->createRegion->setEnabled(channel != NULL);
  if (channel)
    m_gui->setOffsetRanges(-1000,1000);
  else
    m_gui->setOffsetRanges(0,0);
}

//------------------------------------------------------------------------
void CountingRegion::showInfo(const QModelIndex& index)
{
  m_gui->regionDescription->setText(index.data(BoundingRegion::DescriptionRole).toString());
  m_gui->saveDescription->setEnabled(index.isValid());
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
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

//------------------------------------------------------------------------
void CountingRegion::computeOptimalMargins(Channel* channel,
                                           Nm inclusion[3],
                                           Nm exclusion[3])
{
  const Nm delta[3] = {
    0.5*channel->itkVolume()->GetSpacing()[0],
    0.5*channel->itkVolume()->GetSpacing()[1],
    0.5*channel->itkVolume()->GetSpacing()[2]
  };

  memset(inclusion, 0, 3*sizeof(Nm));
  memset(exclusion, 0, 3*sizeof(Nm));

  ModelItem::Vector items = channel->relatedItems(ModelItem::OUT, Channel::LINK);
  SegmentationList channelSegs;
  foreach(ModelItem *item, items)
  {
    if (ModelItem::SEGMENTATION == item->type())
      channelSegs << dynamic_cast<Segmentation *>(item);
  }

  foreach(Segmentation *seg, channelSegs)
  {
    ModelItemExtension *ext = seg->extension(MarginsSegmentationExtension::ID);
    MarginsSegmentationExtension *marginExt = dynamic_cast<MarginsSegmentationExtension *>(ext);
    if (marginExt)
    {
      Nm dist2Margin[6];
      marginExt->margins(dist2Margin);
      double segBounds[6];
      VolumeBounds(seg->itkVolume(), segBounds);

      for (int i=0; i < 3; i++)
      {
        double length = segBounds[2*i+1] - segBounds[2*i];
        length += seg->itkVolume()->GetSpacing()[i];
        if (dist2Margin[2*i] < delta[i])
          inclusion[i] = std::max(length, inclusion[i]);
        if (dist2Margin[2*i+1] < delta[i])
          exclusion[i] = std::max(length, exclusion[i]);
      }
    }
  }
  //qDebug() << "Inclusion:" << inclusion[0] << inclusion[1] << inclusion[2];
  //qDebug() << "Exclusion:" << exclusion[0] << exclusion[1] << exclusion[2];
}

//------------------------------------------------------------------------
void CountingRegion::resetState()
{
  clearBoundingRegions();
}

Q_EXPORT_PLUGIN2(CountingRegionPlugin, CountingRegion)
