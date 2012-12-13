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

#include "CountingFramePanel.h"
#include "ui_CountingFramePanel.h"

#include "CountingFrames/RectangularCountingFrame.h"
#include "CountingFrames/AdaptiveCountingFrame.h"
#include "Extensions/CountingFrameSegmentationExtension.h"
#include "Extensions/CountingFrameChannelExtension.h"
#include "CountingFrameRenderer.h"
#include "ColorEngines/CountingFrameColorEngine.h"

#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>
#include <Core/Extensions/Margins/MarginsSegmentationExtension.h>

#include <QFileDialog>

const int ADAPTIVE = 0;
const int RECTANGULAR = 1;

//------------------------------------------------------------------------
class CountingFramePanel::GUI
: public QWidget
, public Ui::CountingFramePanel
{
public:
  explicit GUI();

  void setOffsetRanges(int min, int max);

private:
  bool eventFilter(QObject *object, QEvent *event);
};

//------------------------------------------------------------------------
CountingFramePanel::GUI::GUI()
{
  setupUi(this);

  leftMargin  ->installEventFilter(this);
  rightMargin ->installEventFilter(this);
  topMargin   ->installEventFilter(this);
  bottomMargin->installEventFilter(this);
  lowerMargin ->installEventFilter(this);
  upperMargin ->installEventFilter(this);
}

//------------------------------------------------------------------------
bool CountingFramePanel::GUI::eventFilter(QObject* object, QEvent* event)
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
    preview->setPixmap(QPixmap(":/allPlanes.png"));

  return QObject::eventFilter(object, event);
}

//------------------------------------------------------------------------
void CountingFramePanel::GUI::setOffsetRanges(int min, int max)
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

const QString CountingFramePanel::ID = "CountingFrameExtension";

//------------------------------------------------------------------------
CountingFramePanel::CountingFramePanel(QWidget * parent)
: IDockWidget(parent)
, m_gui(new GUI())
, m_espinaModel(NULL)
, m_viewManager(NULL)
, m_activeCF(NULL)
, m_nextId(1)
{
  setObjectName("CountingFrameDock");
  setWindowTitle(tr("Counting Frame"));
  setWidget(m_gui);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_gui->saveDescription->setIcon(iconSave);
  connect(m_gui->saveDescription, SIGNAL(clicked(bool)),
          this, SLOT(saveCountingFrameDescription()));

  m_gui->countingFrames->addItem("Create Adaptive CF");
  m_gui->countingFrames->addItem("Create Rectangular CF");

  connect(m_gui->createCF, SIGNAL(clicked()),
          this, SLOT(createCountingFrame()));
  connect(m_gui->deleteCF, SIGNAL(clicked()),
          this, SLOT(deleteSelectedCountingFrame()));

  connect(m_gui->countingFrames,SIGNAL(currentIndexChanged(int)),
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

  connect(m_gui->useTaxonomicalConstraint, SIGNAL(toggled(bool)),
          this, SLOT(enableTaxonomicalConstraints(bool)));
  connect(m_gui->taxonomySelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(applyTaxonomicalConstraint()));
}

//------------------------------------------------------------------------
CountingFramePanel::~CountingFramePanel()
{
  clearCountingFrames();
}

//------------------------------------------------------------------------
void CountingFramePanel::initDockWidget(EspinaModel* model, QUndoStack* undoStack, ViewManager* viewManager)
{
  m_espinaModel = model;
  m_viewManager = viewManager;
  connect(m_viewManager, SIGNAL(activeChannelChanged(Channel*)),
          this, SLOT(channelChanged(Channel*)));

  ChannelExtension::SPtr channelExtension(new CountingFrameChannelExtension(this, m_viewManager));
  m_espinaModel->factory()->registerChannelExtension(channelExtension);
  SegmentationExtension::SPtr segExtension(new CountingFrameSegmentationExtension());
  m_espinaModel->factory()->registerSegmentationExtension(segExtension);
  m_espinaModel->factory()->registerRenderer(new CountingFrameRenderer(this));


  m_gui->taxonomySelector->setModel(m_espinaModel);
  m_gui->taxonomySelector->setRootModelIndex(m_espinaModel->taxonomyRoot());
}

//------------------------------------------------------------------------
IColorEngineProvider::EngineList CountingFramePanel::colorEngines()
{
  EngineList engines;
  engines << Engine(tr("Counting Frame"), ColorEnginePtr(new CountingFrameColorEngine()));

  return engines;
}

//------------------------------------------------------------------------
void CountingFramePanel::reset()
{
  clearCountingFrames();
}

//------------------------------------------------------------------------
void CountingFramePanel::createAdaptiveCF(Channel *channel,
                                          Nm inclusion[3],
                                          Nm exclusion[3])
{
  ModelItemExtension *ext = channel->extension(CountingFrameChannelExtension::ID);
  Q_ASSERT(ext);
  CountingFrameChannelExtension *channelExt = dynamic_cast<CountingFrameChannelExtension *>(ext);
  Q_ASSERT(channelExt);

  AdaptiveCountingFrame *cf(new AdaptiveCountingFrame(m_nextId,
                                                            channelExt,
                                                            inclusion,
                                                            exclusion,
                                                            m_viewManager));
  registerCF(channelExt, cf);
}

//------------------------------------------------------------------------
void CountingFramePanel::createRectangularCF(Channel *channel,
                                             Nm inclusion[3],
                                             Nm exclusion[3])
{
  ModelItemExtension *ext = channel->extension(CountingFrameChannelExtension::ID);
  Q_ASSERT(ext);
  CountingFrameChannelExtension *channelExt = dynamic_cast<CountingFrameChannelExtension *>(ext);
  Q_ASSERT(channelExt);

  double borders[6];
  channel->volume()->bounds(borders);

  RectangularCountingFrame *cf(new RectangularCountingFrame(m_nextId,
                                                                  channelExt,
                                                                  borders,
                                                                  inclusion,
                                                                  exclusion,
                                                                  m_viewManager));
  registerCF(channelExt, cf);
}

//------------------------------------------------------------------------
void CountingFramePanel::applyTaxonomicalConstraint()
{
  if (m_activeCF && m_gui->useTaxonomicalConstraint->isChecked())
  {
    QModelIndex taxonomyIndex = m_gui->taxonomySelector->currentModelIndex();
    if (taxonomyIndex.isValid())
    {
      ModelItem *item = indexPtr(taxonomyIndex);
      Q_ASSERT(ModelItem::TAXONOMY == item->type());

      TaxonomyElement *taxonomy = dynamic_cast<TaxonomyElement *>(item);
      m_activeCF->setTaxonomicalConstraint(taxonomy);
    }
  }
}

//------------------------------------------------------------------------
void CountingFramePanel::clearCountingFrames()
{
  while (m_gui->countingFrames->count() > 2)
    m_gui->countingFrames->removeItem(2);

  m_gui->countingFrameDescription->clear();
  m_gui->createCF->setEnabled(false);
  m_gui->deleteCF->setEnabled(false);

  // TODO 2012-11-06 It should be removed due to channel being destroyed
  foreach(CountingFrame *cf, m_countingFrames)
  {
    m_viewManager->removeWidget(cf);
    delete cf;
  }

  m_countingFrames.clear();
}

//------------------------------------------------------------------------
void CountingFramePanel::enableTaxonomicalConstraints(bool enable)
{
  m_gui->taxonomySelector->setEnabled(enable);

  applyTaxonomicalConstraint();
}

//------------------------------------------------------------------------
void CountingFramePanel::updateUI(int row)
{
  if (row > RECTANGULAR)
  {
    m_gui->createCF->setIcon(QIcon(":/update-cr.svg"));
    m_gui->deleteCF->setEnabled(true);
    m_gui->saveDescription->setEnabled(true);

    CountingFrame *cf = m_countingFrames.value(row-NUM_FIXED_ROWS, NULL);
    Q_ASSERT(cf);

    showInfo(cf);

  } else
  {
    m_activeCF = NULL;

    m_gui->createCF->setIcon(QIcon(":/create-cr.svg"));
    m_gui->deleteCF->setEnabled(false);
    m_gui->saveDescription->setEnabled(false);

    m_gui->leftMargin  ->setValue(0);
    m_gui->topMargin   ->setValue(0);
    m_gui->upperMargin ->setValue(0);
    m_gui->rightMargin ->setValue(0);
    m_gui->bottomMargin->setValue(0);
    m_gui->lowerMargin ->setValue(0);

    m_gui->useTaxonomicalConstraint->setChecked(false);

    m_gui->countingFrameDescription->clear();

    m_gui->countingFrames->setFocus();
  }

  m_gui->taxonomySelector->setEnabled(m_gui->useTaxonomicalConstraint->isChecked());
}

//------------------------------------------------------------------------
void CountingFramePanel::createCountingFrame()
{
  Channel *channel = m_viewManager->activeChannel();
  Q_ASSERT(channel);

  Nm inclusion[3];
  Nm exclusion[3];

  QApplication::setOverrideCursor(Qt::WaitCursor);

  computeOptimalMargins(channel, inclusion, exclusion);
  memset(exclusion, 0, 3*sizeof(Nm));

  if (m_activeCF)
  {
    m_activeCF->setMargins(inclusion, exclusion);
  }else
  {
    if (ADAPTIVE == m_gui->countingFrames->currentIndex())
      createAdaptiveCF(channel, inclusion, exclusion);
    else if (RECTANGULAR == m_gui->countingFrames->currentIndex())
      createRectangularCF(channel, inclusion, exclusion);
    else
      Q_ASSERT(false);
    m_nextId++;
  }

  updateSegmentations();

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void CountingFramePanel::updateBoundingMargins()
{
  if (!m_activeCF)
    return;

  Nm inclusion[3];
  Nm exclusion[3];

  inclusionMargins(inclusion);
  exclusionMargins(exclusion);

  m_activeCF->setMargins(inclusion, exclusion);
}


//------------------------------------------------------------------------
void CountingFramePanel::deleteSelectedCountingFrame()
{
  if (!m_activeCF)
    return;

  m_viewManager->removeWidget(m_activeCF);
  m_countingFrames.removeAll(m_activeCF);

  delete m_activeCF;
  m_activeCF = NULL;

  m_gui->countingFrames->removeItem(m_gui->countingFrames->currentIndex());

  updateSegmentations();
}

//------------------------------------------------------------------------
void CountingFramePanel::channelChanged(Channel* channel)
{
  m_gui->taxonomySelector->setRootModelIndex(m_espinaModel->taxonomyRoot());

  m_gui->createCF->setEnabled(channel != NULL);
  if (channel)
  {
    double bounds[6];
    channel->volume()->bounds(bounds);
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
void CountingFramePanel::showInfo(CountingFrame* cf)
{
  m_activeCF = cf;

  int cfIndex = m_countingFrames.indexOf(cf);
  m_gui->countingFrames->setCurrentIndex(cfIndex + NUM_FIXED_ROWS);

  m_gui->leftMargin  ->blockSignals(true);
  m_gui->topMargin   ->blockSignals(true);
  m_gui->upperMargin ->blockSignals(true);
  m_gui->rightMargin ->blockSignals(true);
  m_gui->bottomMargin->blockSignals(true);
  m_gui->lowerMargin ->blockSignals(true);

  m_gui->leftMargin  ->setValue(cf->left()  );
  m_gui->topMargin   ->setValue(cf->top()   );
  m_gui->upperMargin ->setValue(cf->upper() );
  m_gui->rightMargin ->setValue(cf->right() );
  m_gui->bottomMargin->setValue(cf->bottom());
  m_gui->lowerMargin ->setValue(cf->lower() );

  m_gui->leftMargin  ->blockSignals(false);
  m_gui->topMargin   ->blockSignals(false);
  m_gui->upperMargin ->blockSignals(false);
  m_gui->rightMargin ->blockSignals(false);
  m_gui->bottomMargin->blockSignals(false);
  m_gui->lowerMargin ->blockSignals(false);

  m_gui->useTaxonomicalConstraint->setChecked(NULL != cf->taxonomicalConstraint());

  m_gui->countingFrameDescription->setText(cf->data(CountingFrame::DescriptionRole).toString());

}

//------------------------------------------------------------------------
void CountingFramePanel::updateSegmentations()
{
  m_viewManager->updateSegmentationRepresentations();
  m_viewManager->updateViews();
}


//------------------------------------------------------------------------
void CountingFramePanel::saveCountingFrameDescription()
{
  QString title   = tr("Save Counting Frame Description");
  QString fileExt = tr("Text File (*.txt)");
  QString fileName = QFileDialog::getSaveFileName(this, title, "", fileExt);

  if (!fileName.isEmpty())
  {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly |  QIODevice::Text);
    QTextStream out(&file);
    out << m_gui->countingFrameDescription->toPlainText();
    file.close();
  }
}

//------------------------------------------------------------------------
void CountingFramePanel::computeOptimalMargins(Channel* channel,
                                           Nm inclusion[3],
                                           Nm exclusion[3])
{
  double spacing[3];
  channel->volume()->spacing(spacing);

  const Nm delta[3] = { 0.5*spacing[0], 0.5*spacing[1], 0.5*spacing[2] };

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
      seg->volume()->bounds(segBounds);
      double spacing[3];
      seg->volume()->spacing(spacing);

      for (int i=0; i < 3; i++)
      {
        double length = segBounds[2*i+1] - segBounds[2*i];
        length += spacing[i];
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
void CountingFramePanel::inclusionMargins(double values[3])
{
  values[0] = m_gui->leftMargin->value();
  values[1] = m_gui->topMargin->value();
  values[2] = m_gui->upperMargin->value();
}

//------------------------------------------------------------------------
void CountingFramePanel::exclusionMargins(double values[3])
{
  values[0] = m_gui->rightMargin->value();
  values[1] = m_gui->bottomMargin->value();
  values[2] = m_gui->lowerMargin->value();
}

//------------------------------------------------------------------------
void CountingFramePanel::registerCF(CountingFrameChannelExtension* ext,
                                    CountingFrame* cf)
{
  ext->addCountingFrame(cf);
  m_viewManager->addWidget(cf);
  m_countingFrames << cf;

  connect(cf, SIGNAL(modified(CountingFrame*)),
          this, SLOT(showInfo(CountingFrame*)));
  connect(cf, SIGNAL(modified(CountingFrame*)),
          this, SLOT(updateSegmentations()));

  m_gui->countingFrames->addItem(cf->data(Qt::DisplayRole).toString());

  m_activeCF = cf; // To make applyTaxonomicalConstraint work
  applyTaxonomicalConstraint();

  showInfo(cf);
}

//------------------------------------------------------------------------
void CountingFramePanel::resetState()
{
  clearCountingFrames();
  m_nextId = 1;
}

Q_EXPORT_PLUGIN2(CountingFramePlugin, CountingFramePanel)
