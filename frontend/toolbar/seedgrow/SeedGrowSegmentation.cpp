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
#include "gui/DefaultVOIAction.h"
#include "gui/ThresholdAction.h"

// EspinaModel
#include "common/gui/ActionSelector.h"
#include <gui/ViewManager.h>
#include "common/model/EspinaFactory.h"
#include "common/model/EspinaModel.h"
#include "common/tools/PixelSelector.h"
#include "common/tools/PickableItem.h"
#include <tools/IVOI.h>
#include "common/undo/AddFilter.h"
#include "common/undo/AddRelation.h"
#include "common/undo/AddSegmentation.h"
#include "common/widgets/RectangularRegion.h"

//GUI includes
#include <QSettings>
#include <QStyle>
#include <QFileDialog>

const int DEFAULT_THRESHOLD = 30;
const QString INPUTLINK = "Input";

const ModelItem::ArgumentId TYPE = "Type";


//-----------------------------------------------------------------------------
SeedGrowSegmentation::UndoCommand::UndoCommand(Channel* channel,
                                               SeedGrowSegmentationFilter* filter,
                                               TaxonomyElement* taxonomy,
                                               EspinaModel *model)
: m_model   (model)
, m_channel (channel)
, m_filter  (filter)
, m_taxonomy(taxonomy)
{
  ModelItem::Vector samples = m_channel->relatedItems(ModelItem::IN,
                                                      Channel::STAINLINK);
  Q_ASSERT(samples.size() > 0);
  m_sample = dynamic_cast<Sample *>(samples.first());
  m_seg = m_model->factory()->createSegmentation(m_filter, 0);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::UndoCommand::redo()
{
  m_model->addFilter(m_filter);
  m_model->addRelation(m_channel->filter(), m_filter, INPUTLINK);
  m_seg->setTaxonomy(m_taxonomy);
  m_model->addSegmentation(m_seg);
  m_model->addRelation(m_filter, m_seg, CREATELINK);
  m_model->addRelation(m_sample, m_seg, Sample::WHERE);
  m_model->addRelation(m_channel, m_seg, Channel::LINK);
  m_seg->initializeExtensions();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::UndoCommand::undo()
{
  m_model->removeRelation(m_channel->filter(), m_seg, INPUTLINK);
  m_model->removeRelation(m_sample, m_seg, Sample::WHERE);
  m_model->removeRelation(m_filter, m_seg, CREATELINK);
  m_model->removeSegmentation(m_seg);
  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  m_model->removeFilter(m_filter);
}


//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(EspinaModel *model,
                                           QUndoStack *undoStack,
                                           ViewManager *vm,
                                           QWidget* parent)
: QToolBar(parent)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
// , m_defaultVOI(NULL)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_pickerSelector(new ActionSelector(this))
, m_seedSelector  (new SeedGrowSelector(m_threshold, m_viewManager))
{
  setObjectName("SeedGrowSegmentation");
  setWindowTitle("Seed Grow Segmentation Tool Bar");

  initFactoryExtension(m_model->factory());
  buildSelectors();

  addAction(m_threshold);
  addAction(m_useDefaultVOI);
  addAction(m_pickerSelector);
  m_threshold->setSymmetricalThreshold(true);
  //QAction *batch = addAction(tr("Batch"));
  //connect(batch, SIGNAL(triggered(bool)), this, SLOT(batchMode()));

  connect(m_pickerSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changePicker(QAction*)));
  connect(m_pickerSelector, SIGNAL(actionCanceled()),
          this, SLOT(abortSelection()));
  connect(m_seedSelector.data(), SIGNAL(selectionAborted()),
          this, SLOT(onSelectionAborted()));
  connect(m_seedSelector.data(), SIGNAL(seedSelected(Channel*, EspinaVolume::IndexType)),
          this, SLOT(startSegmentation(Channel*,EspinaVolume::IndexType)));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentation::~SeedGrowSegmentation()
{
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::initFactoryExtension(EspinaFactory* factory)
{
  // Register Factory's filters
  factory->registerFilter(this, SeedGrowSegmentationFilter::TYPE);
}

//-----------------------------------------------------------------------------
Filter* SeedGrowSegmentation::createFilter(const QString filter,
                                           Filter::NamedInputs inputs,
                                           const ModelItem::Arguments args)
{
  Q_ASSERT(SeedGrowSegmentationFilter::TYPE == filter);

  return new SeedGrowSegmentationFilter(inputs, args);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::changePicker(QAction* action)
{
  Q_ASSERT(m_selectors.contains(action));
  m_seedSelector->setChannelPicker(m_selectors[action]);
  m_seedSelector->previewOn();
  m_viewManager->setActiveTool(m_seedSelector.data());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::abortSelection()
{
  m_seedSelector->previewOff();
  m_viewManager->unsetActiveTool(m_seedSelector.data());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::onSelectionAborted()
{
  m_pickerSelector->cancel();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::startSegmentation(Channel* channel,
                                             EspinaVolume::IndexType seed )
{
  double spacing[3];
  channel->spacing(spacing);

  Nm voiBounds[6];
  IVOI::Region currentVOI = m_viewManager->voiRegion();
  if (currentVOI)
  {
    memcpy(voiBounds, currentVOI, 6*sizeof(double));
  }
  else if (m_useDefaultVOI->useDefaultVOI())
  {
    voiBounds[0] = seed[0]*spacing[0] - m_settings->xSize();
    voiBounds[1] = seed[0]*spacing[0] + m_settings->xSize();
    voiBounds[2] = seed[1]*spacing[1] - m_settings->ySize();
    voiBounds[3] = seed[1]*spacing[1] + m_settings->ySize();
    voiBounds[4] = seed[2]*spacing[2] - m_settings->zSize();
    voiBounds[5] = seed[2]*spacing[2] + m_settings->zSize();
  } else
  {
    channel->bounds(voiBounds);
  }

  int voiExtent[6];
  for (int i=0; i<6; i++)
    voiExtent[i] = (voiBounds[i] / spacing[i/2]) + 0.5;

  if (voiExtent[0] <= seed[0] && seed[0] <= voiExtent[1] &&
    voiExtent[2] <= seed[1] && seed[1] <= voiExtent[3] &&
    voiExtent[4] <= seed[2] && seed[2] <= voiExtent[5])
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    Filter::NamedInputs inputs;
    Filter::Arguments args;
    SeedGrowSegmentationFilter::Parameters params(args);
    params.setSeed(seed);
    params.setLowerThreshold(m_threshold->lowerThreshold());
    params.setUpperThreshold(m_threshold->upperThreshold());
    params.setVOI(voiExtent);
    params.setCloseValue(m_settings->closing());
    inputs[INPUTLINK] = channel->filter();
    args[Filter::INPUTS] = INPUTLINK + "_" + QString::number(channel->outputNumber());
    SeedGrowSegmentationFilter *filter;
    filter = new SeedGrowSegmentationFilter(inputs, args);
    filter->update();
    Q_ASSERT(filter->numberOutputs() == 1);

    TaxonomyElement *tax = m_viewManager->activeTaxonomy();
    Q_ASSERT(tax);

    m_undoStack->push(new UndoCommand(channel, filter, tax, m_model));
    QApplication::restoreOverrideCursor();
  }
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::batchMode()
{
  QFileDialog dialog;
  if (dialog.exec())
  {
    QFile seedFile(dialog.selectedFiles().first());
    if (!seedFile.open(QIODevice::ReadOnly | QIODevice::Text))
      return;

    QTextStream in(&seedFile);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    while (!in.atEnd())
    {
      QString line = in.readLine();
      QStringList seedParams = line.split(" ");
      QString Seed = seedParams[0].split("=")[1];
      QString Threshold = seedParams[1].split("=")[1];
      QString VOI = seedParams[2].split("=")[1];
      QString taxonomy = seedParams[3].split("=")[1];

      Channel *channel = m_viewManager->activeChannel();

      Filter::NamedInputs inputs;
      Filter::Arguments args;
      SeedGrowSegmentationFilter::Parameters params(args);
      args[SeedGrowSegmentationFilter::SEED] = Seed;
      args[SeedGrowSegmentationFilter::LTHRESHOLD] = Threshold;
      args[SeedGrowSegmentationFilter::UTHRESHOLD] = Threshold;
      args[SeedGrowSegmentationFilter::VOI] = VOI;
      params.setCloseValue(0);
      inputs[INPUTLINK] = channel->filter();
      args[Filter::INPUTS] = INPUTLINK + "_" + QString::number(channel->outputNumber());
      SeedGrowSegmentationFilter *filter;
      filter = new SeedGrowSegmentationFilter(inputs, args);
      filter->update();
      Q_ASSERT(filter->numberOutputs() == 1);

      Taxonomy * const currentTax = m_model->taxonomy();
      TaxonomyElement *tax = currentTax->element(taxonomy);
      if (tax == NULL)
      {
        QModelIndex taxRoot = m_model->taxonomyRoot();
        m_model->addTaxonomyElement(taxRoot, taxonomy);
        tax = currentTax->element(taxonomy);
      }
      Q_ASSERT(tax);

      m_undoStack->push(new UndoCommand(channel, filter, tax, m_model));
    }
    QApplication::restoreOverrideCursor();
  }
}


//------------------------------------------------------------------------
void SeedGrowSegmentation::addPixelSelector(QAction* action, IPicker* handler)
{
  m_pickerSelector->addAction(action);
  m_selectors[action] = handler;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::buildSelectors()
{
  IPicker *selector;
  QAction *action;

  // Exact Pixel Selector
  action = new QAction(QIcon(":pixelSelector.svg"), tr("Add synapse (Ctrl +). Exact Pixel"), m_pickerSelector);
  selector = new PixelSelector();
  selector->setMultiSelection(false);
  selector->setPickable(IPicker::CHANNEL);
  addPixelSelector(action, selector);

  // Best Pixel Selector
  action = new QAction(QIcon(":bestPixelSelector.svg"), tr("Add synapse (Ctrl +). Best Pixel"), m_pickerSelector);
  BestPixelSelector *bestSelector = new BestPixelSelector();
  m_settings = new Settings(bestSelector);
  m_settingsPanel = new SettingsPanel(m_settings);
  m_model->factory()->registerSettingsPanel(m_settingsPanel);
//     bestSelector->setBestPixelValue(settings.value(BEST_PIXEL).toInt());
  selector = bestSelector;
  selector->setMultiSelection(false);
  selector->setPickable(IPicker::CHANNEL);
  selector->setCursor(QCursor(QPixmap(":crossRegion.svg")));
  addPixelSelector(action, selector);
}
