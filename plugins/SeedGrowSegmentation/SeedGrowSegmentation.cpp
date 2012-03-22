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

// EspinaModel
#include "SeedGrowSelector.h"
#include "common/gui/ActionSelector.h"
#include "gui/DefaultVOIAction.h"
#include "gui/ThresholdAction.h"
#include "common/model/EspinaModel.h"
#include "common/undo/AddSegmentation.h"
#include "common/EspinaCore.h"
#include "common/undo/AddFilter.h"
#include "common/selection/SelectableItem.h"
#include "common/undo/AddRelation.h"
#include "common/model/Channel.h"
#include "common/model/EspinaFactory.h"

// #include "SeedGrowSegmentationFilter.h"
#include "common/processing/pqData.h"
#include "common/selection/PixelSelector.h"
#include "common/selection/SelectionManager.h"

//GUI includes
#include <QSettings>
#include <QStyle>

#include <pqApplicationCore.h>
#include <pqPipelineFilter.h>
#include <pqServer.h>
#include <pqServerManagerModel.h>
#include <vtkSMProxy.h>
#include <vtkSMInputProperty.h>
#include <vtkSMPropertyHelper.h>

#define DEFAULT_THRESHOLD 30


//-----------------------------------------------------------------------------
SeedGrowSegmentation::UndoCommand::UndoCommand(Channel* channel,
					       SeedGrowSegmentationFilter* filter,
					       TaxonomyNode* taxonomy)
: m_channel (channel)
, m_filter  (filter)
, m_taxonomy(taxonomy)
{
  ModelItem::Vector samples = m_channel->relatedItems(ModelItem::IN, "mark");
  Q_ASSERT(samples.size() > 0);
  m_sample = dynamic_cast<Sample *>(samples.first());
  m_seg = m_filter->product(0);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::UndoCommand::redo()
{
  QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

  model->addFilter(m_filter);
  model->addRelation(m_channel, m_filter, "Channel");
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

  model->removeRelation(m_channel, m_seg, "Channel");
  model->removeRelation(m_sample, m_seg, "where");
  model->removeRelation(m_filter, m_seg, "CreateSegmentation");
  model->removeSegmentation(m_seg);
  model->removeRelation(m_channel, m_filter, "Channel");
  model->removeFilter(m_filter);
}


//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(QObject* parent)
: QActionGroup(parent)
// , m_defaultVOI(NULL)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_segment       (new ActionSelector(this))
, m_selector      (new SeedGrowSelector(m_threshold))
// , m_preferences(NULL)
{
  setObjectName("SeedGrowSegmentation");
  // Register Factory's filters
  EspinaFactory::instance()->registerFilter(SGSF, this);

//   m_defaultVOI = new RectangularVOI(false);

  buildSelectors();

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
Filter *SeedGrowSegmentation::createFilter(const QString filter, const ModelItem::Arguments args)
{
  Q_ASSERT(filter == SGSF);

  return new SeedGrowSegmentationFilter(args);
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
//   QApplication::setOverrideCursor(Qt::WaitCursor);

  // Initialize application context
//   pqApplicationCore* core = pqApplicationCore::instance();
  //pqServerManagerModel* sm = core->getServerManagerModel();

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

    Filter::Arguments args;
    args["Type"] = SGSF;
    args["Channel"]= input->volume().id();
    args["Seed"] = QString("%1,%2,%3").arg(seed.x()).arg(seed.y()).arg(seed.z());
    args["Threshold"] = QString::number(m_threshold->threshold());

    int growSeed[3] = {seed.x(), seed.y(), seed.z()};

    int VOI[6];
    if (m_useDefaultVOI->useDefaultVOI())
    {
      const int W = 60;
      VOI[0] = seed.x() - W;
      VOI[1] = seed.x() + W;
      VOI[2] = seed.y() - W;
      VOI[3] = seed.y() + W;
      VOI[4] = seed.z() - W;
      VOI[5] = seed.z() + W;
    } else
    {
      Q_ASSERT(false);
    }

    Q_ASSERT(ModelItem::CHANNEL == input->type());
    Channel *channel = dynamic_cast<Channel *>(input);

    SeedGrowSegmentationFilter *filter =
	new SeedGrowSegmentationFilter(input->volume(),
				     growSeed,
				     m_threshold->threshold(),
				     VOI);
    Q_ASSERT(filter->numProducts() == 1);

    TaxonomyNode *tax = EspinaCore::instance()->activeTaxonomy();
    Q_ASSERT(tax);

    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
//     undo->beginMacro("Add Segmentation");
    undo->push(new UndoCommand(channel, filter, tax));
//     undo->endMacro();

  // args.insert("VOI",SelectionManager::instance()->voi()->save());
  //createFilter(m_pluginName + "::" + "SeedGrowSegmentationFilter",args);createFilter(m_pluginName + "::" + "SeedGrowSegmentationFilter",args);

//   IVOI *voi = SelectionManager::instance()->voi();
//   if (!voi && m_useDefaultVOI->isChecked())
//   {
//     voi = m_defaultVOI;
//     Sample *input = EspinaModel::instance()->activeSample();
//     voi->setSource(input);
//     double spacing[3];
//     input->spacing(spacing);
//     double defVOI[6] = {(seed.x - m_preferences->xSize())*spacing[0],
// 			(seed.x + m_preferences->xSize())*spacing[0],
// 			(seed.y - m_preferences->ySize())*spacing[1],
// 			(seed.y + m_preferences->ySize())*spacing[1],
// 			(seed.z - m_preferences->zSize())*spacing[2],
// 			(seed.z + m_preferences->zSize())*spacing[2]};
//     vtkSMPropertyHelper(voi->getProxy(),"Bounds").Set(defVOI,6);
//     voi->getProxy()->UpdateVTKObjects();
// //     double checkBounds[6];
//     vtkSMPropertyHelper(voi->getProxy(),"Bounds").Get(defVOI,6);
//   }

//     m_lastFilter = new SeedGrowSegmentationFilter(args);
//   if (!sgs_sgsf)
//     qWarning() << "SeedGrowSegmentation: Failed to create new segmentation";
  }

//   QApplication::restoreOverrideCursor();
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
//   m_preferences = new SeedGrowSegmentationSettings(bestSelector);
//   EspinaPluginManager::instance()->registerPreferencePanel(m_preferences);
//   QSettings settings;
//   if (settings.contains(BEST_PIXEL))
//     bestSelector->setBestPixelValue(settings.value(BEST_PIXEL).toInt());
  selector = bestSelector;
  selector->setMultiSelection(false);
  selector->setSelectable(SelectionHandler::EspINA_Channel);
  selector->setCursor(QCursor(QPixmap(":crossRegion.svg")));
  addPixelSelector(action, selector);
}
