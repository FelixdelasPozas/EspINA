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
#include "SeedGrowSegmentationSettings.h"

#include <Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <Settings/SeedGrowSegmentation/SettingsPanel.h>
#include <FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h>
#include <Undo/SeedGrowSegmentationCommand.h>

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

using namespace EspINA;

const int DEFAULT_THRESHOLD = 30;

const ModelItem::ArgumentId TYPE = "Type";


//-----------------------------------------------------------------------------
SeedGrowSegmentation::SeedGrowSegmentation(EspinaModel *model,
                                           QUndoStack  *undoStack,
                                           ViewManager *viewManager,
                                           QWidget     *parent)
: IToolBar        (parent)
, m_model         (model)
, m_undoStack     (undoStack)
, m_viewManager   (viewManager)
, m_threshold     (new ThresholdAction(this))
, m_useDefaultVOI (new DefaultVOIAction(this))
, m_pickerSelector(new ActionSelector(this))
{
  setObjectName("SeedGrowSegmentation");

  setWindowTitle(tr("Seed Grow Segmentation Tool Bar"));

  buildPickers();

  m_settingsPanel = ISettingsPanelPrototype(new SettingsPanel(m_settings, m_viewManager));

  initFactoryExtension(m_model->factory());


  m_tool = SeedGrowSegmentationToolSPtr(
  new SeedGrowSegmentationTool(model,
                               undoStack,
                               viewManager,
                               m_threshold,
                               m_useDefaultVOI,
                               m_settings) );

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
  connect(m_tool.data(), SIGNAL(segmentationStopped()),
          this, SLOT(cancelSegmentationOperation()));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentation::~SeedGrowSegmentation()
{
  delete m_settings;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::initToolBar(EspinaModel *model,
                                       QUndoStack  *undoStack,
                                       ViewManager *viewManager)
{

}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::initFactoryExtension(EspinaFactory *factory)
{
  // Register Factory's filters
  factory->registerFilter(this, SeedGrowSegmentationCommand::FILTER_TYPE);

  factory->registerSettingsPanel(m_settingsPanel.data());
}

//-----------------------------------------------------------------------------
FilterSPtr SeedGrowSegmentation::createFilter(const QString              &filter,
                                                   const Filter::NamedInputs  &inputs,
                                                   const ModelItem::Arguments &args)
{
  Q_ASSERT(SeedGrowSegmentationCommand::FILTER_TYPE == filter);

  // TODO 2012-12-17 Revisar esto
  SeedGrowSegmentationFilter *sgs = new SeedGrowSegmentationFilter(inputs, args, SeedGrowSegmentationCommand::FILTER_TYPE);

  Filter::FilterInspectorPtr inspector(new SGSFilterInspector(sgs));
  sgs->setFilterInspector(inspector);

  return FilterSPtr(sgs);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentation::changePicker(QAction* action)
{
  Q_ASSERT(m_pickers.contains(action));
  m_tool->setChannelPicker(m_pickers[action]);
  m_viewManager->setActiveTool(m_tool);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::cancelSegmentationOperation()
{
  m_pickerSelector->cancel();
  m_viewManager->unsetActiveTool(m_tool);
}

//------------------------------------------------------------------------
void SeedGrowSegmentation::reset()
{
  cancelSegmentationOperation();
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
void SeedGrowSegmentation::addVoxelPicker(QAction* action, IPickerSPtr picker)
{
  m_pickerSelector->addAction(action);
  m_pickers[action] = picker;
  picker->setMultiSelection(false);
  picker->setPickable(IPicker::CHANNEL);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentation::buildPickers()
{
  QAction *action;

  // Exact Pixel Picker
  action = new QAction(QIcon(":/espina/pixelSelector.svg"),
                       tr("Add synapse (Ctrl +). Exact Pixel"),
                       m_pickerSelector);
  PixelPicker *picker = new PixelPicker();
  addVoxelPicker(action, IPickerSPtr(picker));

  // Best Pixel Picker
  action = new QAction(QIcon(":/espina/bestPixelSelector.svg"),
                       tr("Add synapse (Ctrl +). Best Pixel"),
                       m_pickerSelector);
  BestPixelPicker *bestPicker = new BestPixelPicker();
  bestPicker->setCursor(QCursor(QPixmap(":/espina/crossRegion.svg")));
  addVoxelPicker(action, IPickerSPtr(bestPicker));

  m_settings = new SeedGrowSegmentationSettings(bestPicker);
}
