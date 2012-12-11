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
#include "ThresholdAction.h"
#include "DefaultVOIAction.h"
#include "Settings.h"
#include <Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <Settings/SeedGrowSegmentation/SettingsPanel.h>
#include <FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h>

#include <QDebug>

// EspinaModel
#include <GUI/QtWidget/ActionSelector.h>
#include <GUI/ViewManager.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/Pickers/PixelPicker.h>
#include <Core/Model/PickableItem.h>
#include <GUI/Tools/IVOI.h>
#include <Undo/AddFilter.h>
#include <Undo/AddRelation.h>
#include <Undo/AddSegmentation.h>
#include <Filters/SeedGrowSegmentationFilter.h>

//GUI includes
#include <QSettings>
#include <QStyle>
#include <QFileDialog>

const int DEFAULT_THRESHOLD = 30;

const ModelItem::ArgumentId TYPE = "Type";


//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(EspinaModel *model,
                                           QUndoStack  *undoStack,
                                           ViewManager *viewManager,
                                           QWidget* parent)
: QToolBar(parent)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_pickerSelector(new ActionSelector(this))
, m_tool(NULL)
{
  setObjectName("SeedGrowSegmentation");
  setWindowTitle("Seed Grow Segmentation Tool Bar");

  initFactoryExtension(m_model->factory());
  buildSelectors();

  m_tool = new SeedGrowSegmentationTool(model,
                                        undoStack,
                                        viewManager,
                                        m_threshold,
                                        m_useDefaultVOI,
                                        m_settings);

  addAction(m_threshold);
  addAction(m_useDefaultVOI);
  addAction(m_pickerSelector);
  m_threshold->setSymmetricalThreshold(true);
  //QAction *batch = addAction(tr("Batch"));
  //connect(batch, SIGNAL(triggered(bool)), this, SLOT(batchMode()));

  connect(m_pickerSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changePicker(QAction*)));
  connect(m_pickerSelector, SIGNAL(actionCanceled()),
          this, SLOT(cancelSegmentationOperation()));
  connect(m_tool, SIGNAL(segmentationStopped()),
          this, SLOT(cancelSegmentationOperation()));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentation::~SeedGrowSegmentation()
{
  delete m_tool;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::initFactoryExtension(EspinaFactory* factory)
{
  // Register Factory's filters
  factory->registerFilter(this, SeedGrowSegmentationFilter::TYPE);
}

//-----------------------------------------------------------------------------
Filter *SeedGrowSegmentation::createFilter(const QString              &filter,
                                           const Filter::NamedInputs  &inputs,
                                           const ModelItem::Arguments &args)
{
  Q_ASSERT(SeedGrowSegmentationFilter::TYPE == filter);

  SeedGrowSegmentationFilter *sgs = new SeedGrowSegmentationFilter(inputs, args);
  // NOTE: automatically assigns widget to filter on new()
  new SGSFilterInspector(sgs);

  return sgs;
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::changePicker(QAction* action)
{
  Q_ASSERT(m_selectors.contains(action));
  m_tool->setChannelPicker(m_selectors[action]);
  m_viewManager->setActiveTool(m_tool);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::cancelSegmentationOperation()
{
  m_pickerSelector->cancel();
  m_viewManager->unsetActiveTool(m_tool);
}


//------------------------------------------------------------------------
void SeedGrowSegmentation::batchMode()
{
//   QFileDialog dialog;
//   if (dialog.exec())
//   {
//     QFile seedFile(dialog.selectedFiles().first());
//     if (!seedFile.open(QIODevice::ReadOnly | QIODevice::Text))
//       return;
// 
//     QTextStream in(&seedFile);
//     QApplication::setOverrideCursor(Qt::WaitCursor);
//     while (!in.atEnd())
//     {
//       QString line = in.readLine();
//       QStringList seedParams = line.split(" ");
//       QString Seed = seedParams[0].split("=")[1];
//       QString Threshold = seedParams[1].split("=")[1];
//       QString VOI = seedParams[2].split("=")[1];
//       QString taxonomy = seedParams[3].split("=")[1];
// 
//       Channel *channel = m_viewManager->activeChannel();
// 
//       Filter::NamedInputs inputs;
//       Filter::Arguments args;
//       SeedGrowSegmentationFilter::Parameters params(args);
//       args[SeedGrowSegmentationFilter::SEED] = Seed;
//       args[SeedGrowSegmentationFilter::LTHRESHOLD] = Threshold;
//       args[SeedGrowSegmentationFilter::UTHRESHOLD] = Threshold;
//       args[SeedGrowSegmentationFilter::VOI] = VOI;
//       params.setCloseValue(0);
//       inputs[SeedGrowSegmentationFilter::INPUTLINK] = channel->filter();
//       args[Filter::INPUTS] = SeedGrowSegmentationFilter::INPUTLINK + "_" + QString::number(channel->outputNumber());
//       SeedGrowSegmentationFilter *filter;
//       filter = new SeedGrowSegmentationFilter(inputs, args);
//       filter->update();
//       Q_ASSERT(filter->numberOutputs() == 1);
// 
//       Taxonomy * const currentTax = m_model->taxonomy();
//       TaxonomyElement *tax = currentTax->element(taxonomy);
//       if (tax == NULL)
//       {
//         QModelIndex taxRoot = m_model->taxonomyRoot();
//         m_model->addTaxonomyElement(taxRoot, taxonomy);
//         tax = currentTax->element(taxonomy);
//       }
//       Q_ASSERT(tax);
// 
//       m_undoStack->push(new UndoCommand(channel, filter, tax, m_model));
//     }
//     QApplication::restoreOverrideCursor();
//   }
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
  action = new QAction(QIcon(":/espina/pixelSelector.svg"), tr("Add synapse (Ctrl +). Exact Pixel"), m_pickerSelector);
  selector = new PixelSelector();
  selector->setMultiSelection(false);
  selector->setPickable(IPicker::CHANNEL);
  addPixelSelector(action, selector);

  // Best Pixel Selector
  action = new QAction(QIcon(":/espina/bestPixelSelector.svg"), tr("Add synapse (Ctrl +). Best Pixel"), m_pickerSelector);
  BestPixelSelector *bestSelector = new BestPixelSelector();
  m_settings = new Settings(bestSelector);
  m_settingsPanel = new SettingsPanel(m_settings);
  m_model->factory()->registerSettingsPanel(m_settingsPanel);
  selector = bestSelector;
  selector->setMultiSelection(false);
  selector->setPickable(IPicker::CHANNEL);
  selector->setCursor(QCursor(QPixmap(":/espina/crossRegion.svg")));
  addPixelSelector(action, selector);
}
