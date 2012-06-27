/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "SeedGrowSegmentation.h"

#include <QDebug>

#include "SeedGrowSegmentationFilter.h"
#include "SeedGrowSelector.h"
#include "Settings.h"
#include "SettingsPanel.h"

// EspinaModel
#include "common/EspinaCore.h"
#include "common/gui/ActionSelector.h"
#include <common/gui/EspinaView.h>
#include "common/model/Channel.h"
#include "common/model/EspinaFactory.h"
#include "common/model/EspinaModel.h"
#include "common/selection/PixelSelector.h"
#include "common/selection/SelectableItem.h"
#include "common/selection/SelectionManager.h"
#include "common/undo/AddFilter.h"
#include "common/undo/AddRelation.h"
#include "common/undo/AddSegmentation.h"
#include "common/widgets/RectangularSelection.h"
#include "gui/DefaultVOIAction.h"
#include "gui/ThresholdAction.h"

//GUI includes
#include <QSettings>
#include <QStyle>

const int DEFAULT_THRESHOLD = 30;
const QString INPUTLINK = "Input";

const ModelItem::ArgumentId TYPE = ModelItem::ArgumentId("Type", true);


//-----------------------------------------------------------------------------
SeedGrowSegmentation::UndoCommand::UndoCommand(Channel* channel,
					       SeedGrowSegmentationFilter* filter,
					       TaxonomyNode* taxonomy)
: m_channel (channel)
, m_filter  (filter)
, m_taxonomy(taxonomy)
{
  ModelItem::Vector samples = m_channel->relatedItems(ModelItem::IN,
                                                      Channel::STAINLINK);
  Q_ASSERT(samples.size() > 0);
  m_sample = dynamic_cast<Sample *>(samples.first());
  m_seg = EspinaFactory::instance()->createSegmentation(m_filter, 0);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::UndoCommand::redo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  model->addFilter(m_filter);
  model->addRelation(m_channel->filter(), m_filter, INPUTLINK);
  m_seg->setTaxonomy(m_taxonomy);
  model->addSegmentation(m_seg);
  model->addRelation(m_filter, m_seg, "CreateSegmentation");
  model->addRelation(m_sample, m_seg, "where");
  model->addRelation(m_channel, m_seg, "Channel");
  m_seg->initialize();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::UndoCommand::undo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  model->removeRelation(m_channel->filter(), m_seg, INPUTLINK);
  model->removeRelation(m_sample, m_seg, "where");
  model->removeRelation(m_filter, m_seg, "CreateSegmentation");
  model->removeSegmentation(m_seg);
  model->removeRelation(m_channel, m_filter, "Channel");
  model->removeFilter(m_filter);
}


//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(QWidget* parent)
: QToolBar(parent)
// , m_defaultVOI(NULL)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_segment       (new ActionSelector(this))
, m_selector      (new SeedGrowSelector(m_threshold))
{
  setObjectName("SeedGrowSegmentation");
  setWindowTitle("Seed Grow Segmentation Tool Bar");
  // Register Factory's filters
  EspinaFactory::instance()->registerFilter(SeedGrowSegmentationFilter::TYPE, this);

  buildSelectors();

  addAction(m_threshold);
  addAction(m_useDefaultVOI);
  addAction(m_segment);
  //   m_defaultVOI = new RectangularVOI(false);

  connect(m_segment, SIGNAL(triggered(QAction*)),
          this, SLOT(waitSeedSelection(QAction*)));
  connect(m_selector.data(), SIGNAL(selectionAborted()),
          this, SLOT(onSelectionAborted()));
  connect(m_segment, SIGNAL(actionCanceled()),
          this, SLOT(abortSelection()));
  connect(m_threshold,SIGNAL(thresholdChanged(int)),
          this, SLOT(modifyLastFilter(int)));
}


//-----------------------------------------------------------------------------
SeedGrowSegmentation::~SeedGrowSegmentation()
{
}

//-----------------------------------------------------------------------------
Filter* SeedGrowSegmentation::createFilter(const QString filter, Filter::NamedInputs inputs, const ModelItem::Arguments args)
{
  Q_ASSERT(SeedGrowSegmentationFilter::TYPE == filter);

  return new SeedGrowSegmentationFilter(inputs, args);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::waitSeedSelection(QAction* action)
{
  Q_ASSERT(m_selectors.contains(action));
  m_selector->setPixelSelector(m_selectors[action]);
  m_selector->previewOn();
  SelectionManager::instance()->setSelectionHandler(m_selector.data());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::abortSelection()
{
  m_selector->previewOff();
  SelectionManager::instance()->setSelectionHandler(NULL);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::onSelectionAborted()
{
  m_segment->cancel();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::startSegmentation(SelectionHandler::MultiSelection msel)
{
  if (msel.size() > 0)
  {
    qDebug() << "Start Segmentation";
    Q_ASSERT(msel.size() == 1);// Only one element selected
    SelectionHandler::Selelection element = msel.first();

    SelectableItem *input = element.second;

    Q_ASSERT(element.first.size() == 1); // with one pixel
    QVector3D seed = element.first.first();
    //     qDebug() << "Channel:" << input->volume().id();
    //     qDebug() << "Threshold:" << m_threshold->threshold();
    //     qDebug() << "Seed:" << seed;
    //     qDebug() << "Use Default VOI:" << m_useDefaultVOI->useDefaultVOI();

    Q_ASSERT(ModelItem::CHANNEL == input->type());
    Channel *channel = dynamic_cast<Channel *>(input);

    int growSeed[3] = {seed.x(), seed.y(), seed.z()};

    int VOI[6];
    EspinaWidget *currentVOI = SelectionManager::instance()->voi();
    if (currentVOI)
    {
      double bounds[6];
      currentVOI->bounds(bounds);
      double spacing[3];
      channel->spacing(spacing);
      for (int i=0; i<6; i++)
        VOI[i] = bounds[i] / spacing[i/2];
      //qDebug() << VOI[0] << VOI[1] << VOI[2] << VOI[3] << VOI[4] << VOI[5];
    }
    else if (m_useDefaultVOI->useDefaultVOI())
    {
      VOI[0] = seed.x() - m_settings->xSize();
      VOI[1] = seed.x() + m_settings->xSize();
      VOI[2] = seed.y() - m_settings->ySize();
      VOI[3] = seed.y() + m_settings->ySize();
      VOI[4] = seed.z() - m_settings->zSize();
      VOI[5] = seed.z() + m_settings->zSize();
    } else
    {
      channel->extent(VOI);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    Filter::NamedInputs inputs;
    SeedGrowSegmentationFilter::SArguments args;
    args[Filter::ID] = Filter::generateId();
    args.setSeed(growSeed);
    args.setLowerThreshold(m_threshold->threshold());
    args.setUpperThreshold(m_threshold->threshold());
    args.setVOI(VOI);
    args.setCloseValue(m_settings->closing());
    inputs[INPUTLINK] = channel->filter();
    args[Filter::INPUTS] = INPUTLINK + "_" + QString::number(channel->outputNumber());

    SeedGrowSegmentationFilter *filter;
    filter = new SeedGrowSegmentationFilter(inputs, args);
    filter->update();
    Q_ASSERT(filter->numberOutputs() == 1);

    TaxonomyNode *tax = EspinaCore::instance()->activeTaxonomy();
    Q_ASSERT(tax);

    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->push(new UndoCommand(channel, filter, tax));
    EspinaCore::instance()->viewManger()->currentView()->forceRender();
    QApplication::restoreOverrideCursor();
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::modifyLastFilter(int value)
{
//   if (m_lastFilter)
//     m_lastFilter->setThreshold(value);
}


//------------------------------------------------------------------------
void SeedGrowSegmentation::addPixelSelector(QAction* action, SelectionHandler* handler)
{
  m_segment->addAction(action);
  m_selectors[action] = handler;
  connect(handler, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	  this, SLOT(startSegmentation(SelectionHandler::MultiSelection)));
//   connect(handler, SIGNAL(selectionAborted()),
// 	  this, SLOT(abortSelection()));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::buildSelectors()
{
  SelectionHandler *selector;
  QAction *action;

  // Exact Pixel Selector
  action = new QAction(
    QIcon(":pixelSelector.svg")
    , tr("Add synapse (Ctrl +). Exact Pixel"),
    m_segment);
  selector = new PixelSelector();
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  addPixelSelector(action, selector);

//   // Best Pixel Selector
  action = new QAction(
    QIcon(":bestPixelSelector.svg")
    , tr("Add synapse (Ctrl +). Best Pixel"),
    m_segment);
  BestPixelSelector *bestSelector = new BestPixelSelector();
  m_settings = new Settings(bestSelector);
  m_settingsPanel = new SettingsPanel(m_settings);
  EspinaFactory::instance()->registerSettingsPanel(m_settingsPanel);
//     bestSelector->setBestPixelValue(settings.value(BEST_PIXEL).toInt());
  selector = bestSelector;
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  selector->setCursor(QCursor(QPixmap(":crossRegion.svg")));
  addPixelSelector(action, selector);
}
