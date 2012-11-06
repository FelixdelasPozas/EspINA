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
  lowerMargin->installEventFilter(this);
  upperMargin->installEventFilter(this);
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
    else if (object == upperMargin)
      preview->setPixmap(QPixmap(":/upper.png"));
    else if (object == lowerMargin)
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
  upperMargin->setMinimum(min);
  upperMargin->setMaximum(max);

  rightMargin->setMinimum(min);
  rightMargin->setMaximum(max);
  bottomMargin->setMinimum(min);
  bottomMargin->setMaximum(max);
  lowerMargin->setMinimum(min);
  lowerMargin->setMaximum(max);
}

const QString CountingRegion::ID = "CountingRegionExtension";

//------------------------------------------------------------------------
CountingRegion::CountingRegion(QWidget * parent)
: IDockWidget(parent)
, m_gui(new GUI())
, m_espinaModel(NULL)
, m_viewManager(NULL)
, m_activeRegion(NULL)
{
  setObjectName("CountingRegionDock");
  setWindowTitle(tr("Counting Region"));
  setWidget(m_gui);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_gui->saveDescription->setIcon(iconSave);
  connect(m_gui->saveDescription, SIGNAL(clicked(bool)),
          this, SLOT(saveRegionDescription()));

  m_gui->regions->addItem("Create Adaptive Region");
  m_gui->regions->addItem("Create Rectangular Region");

  connect(m_gui->createRegion, SIGNAL(clicked()),
          this, SLOT(createBoundingRegion()));
  connect(m_gui->removeRegion, SIGNAL(clicked()),
          this, SLOT(removeSelectedBoundingRegion()));

  connect(m_gui->regions,SIGNAL(currentIndexChanged(int)),
          this, SLOT(updateUI(int)));

  connect(m_gui->leftMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBoundingMargins()));
  connect(m_gui->topMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBoundingMargins()));
  connect(m_gui->upperMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBoundingMargins()));
  connect(m_gui->rightMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBoundingMargins()));
  connect(m_gui->bottomMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBoundingMargins()));
  connect(m_gui->lowerMargin, SIGNAL(valueChanged(int)),
          this, SLOT(updateBoundingMargins()));

}

//------------------------------------------------------------------------
CountingRegion::~CountingRegion()
{
  clearBoundingRegions();
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
  registerRegion(channelExt, region);
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
  registerRegion(channelExt, region);
}

//------------------------------------------------------------------------
void CountingRegion::clearBoundingRegions()
{
  while (m_gui->regions->count() > 2)
    m_gui->regions->removeItem(2);

  m_gui->regionDescription->clear();
  m_gui->createRegion->setEnabled(false);
  m_gui->removeRegion->setEnabled(false);

  // TODO 2012-11-06 It should be removed due to channel being destroyed
  foreach(BoundingRegion *region, m_regions)
  {
    m_viewManager->removeWidget(region);
    delete region;
  }

  m_regions.clear();
}

//------------------------------------------------------------------------
void CountingRegion::updateUI(int row)
{
  if (row > RECTANGULAR)
  {
    m_gui->createRegion->setIcon(QIcon(":/update-cr.svg"));
    m_gui->removeRegion   ->setEnabled(true);
    m_gui->saveDescription->setEnabled(true);

    BoundingRegion *region = m_regions.value(row-NUM_FIXED_ROWS, NULL);
    Q_ASSERT(region);

    showInfo(region);

  } else
  {
    m_activeRegion = NULL;

    m_gui->createRegion->setIcon(QIcon(":/create-cr.svg"));
    m_gui->removeRegion   ->setEnabled(false);
    m_gui->saveDescription->setEnabled(false);

    m_gui->leftMargin  ->setValue(0);
    m_gui->topMargin   ->setValue(0);
    m_gui->upperMargin ->setValue(0);
    m_gui->rightMargin ->setValue(0);
    m_gui->bottomMargin->setValue(0);
    m_gui->lowerMargin ->setValue(0);

    m_gui->regionDescription->clear();

    m_gui->regions->setFocus();
  }
}

//------------------------------------------------------------------------
void CountingRegion::createBoundingRegion()
{
  Channel *channel = m_viewManager->activeChannel();
  Q_ASSERT(channel);

  Nm inclusion[3];
  Nm exclusion[3];

  QApplication::setOverrideCursor(Qt::WaitCursor);

  computeOptimalMargins(channel, inclusion, exclusion);

  if (m_activeRegion)
  {
    m_activeRegion->setMargins(inclusion, exclusion);
  }else
  {
    if (ADAPTIVE == m_gui->regions->currentIndex())
      createAdaptiveRegion(channel, inclusion, exclusion);
    else if (RECTANGULAR == m_gui->regions->currentIndex())
      createRectangularRegion(channel, inclusion, exclusion);
    else
      Q_ASSERT(false);
  }

  updateSegmentations();

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void CountingRegion::updateBoundingMargins()
{
  if (!m_activeRegion)
    return;

  Nm inclusion[3];
  Nm exclusion[3];

  inclusionMargins(inclusion);
  exclusionMargins(exclusion);

  m_activeRegion->setMargins(inclusion, exclusion);
}


//------------------------------------------------------------------------
void CountingRegion::removeSelectedBoundingRegion()
{
  if (!m_activeRegion)
    return;

  m_viewManager->removeWidget(m_activeRegion);
  m_regions.removeAll(m_activeRegion);

  delete m_activeRegion;
  m_activeRegion = NULL;

  m_gui->regions->removeItem(m_gui->regions->currentIndex());

  updateSegmentations();
}

//------------------------------------------------------------------------
void CountingRegion::channelChanged(Channel* channel)
{
  m_gui->createRegion->setEnabled(channel != NULL);
  if (channel)
  {
    double bounds[6];
    channel->bounds(bounds);
    double lenght[3];
    for (int i=0; i < 3; i++)
      lenght[i] = bounds[2*i+1]-bounds[2*i];

    m_gui->leftMargin  ->setMaximum(lenght[0]);
    m_gui->topMargin   ->setMaximum(lenght[1]);
    m_gui->upperMargin ->setMaximum(lenght[2]);
    m_gui->rightMargin ->setMaximum(lenght[0]);
    m_gui->bottomMargin->setMaximum(lenght[1]);
    m_gui->lowerMargin ->setMaximum(lenght[2]);
  }
  else
    m_gui->setOffsetRanges(0,0);
}

//------------------------------------------------------------------------
void CountingRegion::showInfo(BoundingRegion* region)
{
  m_activeRegion = region;

  int regionIndex = m_regions.indexOf(region);
  m_gui->regions->setCurrentIndex(regionIndex + NUM_FIXED_ROWS);

  m_gui->leftMargin  ->blockSignals(true);
  m_gui->topMargin   ->blockSignals(true);
  m_gui->upperMargin ->blockSignals(true);
  m_gui->rightMargin ->blockSignals(true);
  m_gui->bottomMargin->blockSignals(true);
  m_gui->lowerMargin ->blockSignals(true);

  m_gui->leftMargin  ->setValue(region->left()  );
  m_gui->topMargin   ->setValue(region->top()   );
  m_gui->upperMargin ->setValue(region->upper() );
  m_gui->rightMargin ->setValue(region->right() );
  m_gui->bottomMargin->setValue(region->bottom());
  m_gui->lowerMargin ->setValue(region->lower() );

  m_gui->leftMargin  ->blockSignals(false);
  m_gui->topMargin   ->blockSignals(false);
  m_gui->upperMargin ->blockSignals(false);
  m_gui->rightMargin ->blockSignals(false);
  m_gui->bottomMargin->blockSignals(false);
  m_gui->lowerMargin ->blockSignals(false);

  m_gui->regionDescription->setText(region->data(BoundingRegion::DescriptionRole).toString());

}

//------------------------------------------------------------------------
void CountingRegion::updateSegmentations()
{
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
void CountingRegion::inclusionMargins(double values[3])
{
  values[0] = m_gui->leftMargin->value();
  values[1] = m_gui->topMargin->value();
  values[2] = m_gui->upperMargin->value();
}

//------------------------------------------------------------------------
void CountingRegion::exclusionMargins(double values[3])
{
  values[0] = m_gui->rightMargin->value();
  values[1] = m_gui->bottomMargin->value();
  values[2] = m_gui->lowerMargin->value();
}

//------------------------------------------------------------------------
void CountingRegion::registerRegion(CountingRegionChannelExtension* ext,
                                    BoundingRegion* region)
{
  ext->addRegion(region);
  m_viewManager->addWidget(region);
  m_regions << region;

  connect(region, SIGNAL(modified(BoundingRegion*)),
          this, SLOT(showInfo(BoundingRegion*)));
  connect(region, SIGNAL(modified(BoundingRegion*)),
          this, SLOT(updateSegmentations()));

  m_gui->regions->addItem(region->data(Qt::DisplayRole).toString());


  showInfo(region);
}

//------------------------------------------------------------------------
void CountingRegion::resetState()
{
  clearBoundingRegions();
}

Q_EXPORT_PLUGIN2(CountingRegionPlugin, CountingRegion)
