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
#include "SegmentationToolBar.h"
#include "ThresholdAction.h"
#include "DefaultVOIAction.h"
#include "SeedGrowSegmentationSettings.h"

#include <Tools/SeedGrowSegmentation/SeedGrowSegmentationTool.h>
#include <Settings/SeedGrowSegmentation/SettingsPanel.h>
#include <FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h>
#include <Undo/SeedGrowSegmentationCommand.h>
#include <App/FilterInspectors/TubularSegmentation/TubularFilterInspector.h>

#include <QDebug>

// EspinaModel
#include <GUI/QtWidget/ActionSelector.h>
#include <GUI/ViewManager.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <GUI/Pickers/PixelSelector.h>
#include <Core/Model/PickableItem.h>
#include <GUI/Tools/IVOI.h>
#include <GUI/Representations/BasicGraphicalRepresentationFactory.h>
#include <Core/Filters/SeedGrowSegmentationFilter.h>

//GUI includes
#include <QSettings>
#include <QStyle>
#include <QFileDialog>

using namespace EspINA;

const int DEFAULT_THRESHOLD = 30;

const ModelItem::ArgumentId TYPE = "Type";


//-----------------------------------------------------------------------------
SegmentationToolBar::SegmentationToolBar(EspinaModel *model,
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
, m_tubularAction (new QAction(this))
{
  setObjectName("SegmentationToolBar");

  setWindowTitle(tr("Segmentation Tool Bar"));

  buildPickers();

  m_SeedGrowSettingsPanel = ISettingsPanelPrototype(new SeedGrowSegmentationsSettingsPanel(m_settings, m_viewManager));

  initFactoryExtension(m_model->factory());

  m_SGStool = SeedGrowSegmentationToolSPtr(new SeedGrowSegmentationTool(model,
                                                                     undoStack,
                                                                     viewManager,
                                                                     m_threshold,
                                                                     m_useDefaultVOI,
                                                                     m_settings) );

  m_tubularTool = TubularToolSPtr(new TubularTool(m_viewManager, m_undoStack, m_model));

  addAction(m_threshold);
  addAction(m_useDefaultVOI);
  addAction(m_pickerSelector);
  m_threshold->setSymmetricalThreshold(true);

  m_tubularAction->setIcon(QIcon(":espina/tubular.svg"));
  m_tubularAction->setCheckable(true);
  this->addAction(m_tubularAction);

  //QAction *batch = addAction(tr("Batch"));
  //connect(batch, SIGNAL(triggered(bool)), this, SLOT(batchMode()));

  connect(m_pickerSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changePicker(QAction*)));
  connect(m_pickerSelector, SIGNAL(actionCanceled()),
          this, SLOT(cancelSegmentationOperation()));
  connect(m_SGStool.get(), SIGNAL(segmentationStopped()),
          this, SLOT(cancelSegmentationOperation()));
  connect(m_tubularAction, SIGNAL(toggled(bool)),
          this, SLOT(tubularActionStateChanged(bool)));
  connect(m_tubularTool.get(), SIGNAL(segmentationStopped()),
          this, SLOT(cancelTubularSegmentationOperation()));
}

//-----------------------------------------------------------------------------
SegmentationToolBar::~SegmentationToolBar()
{
  delete m_settings;

  if (m_tubularTool != NULL)
    m_tubularTool.reset();
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::initToolBar(EspinaModel *model,
                                      QUndoStack  *undoStack,
                                      ViewManager *viewManager)
{
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::initFactoryExtension(EspinaFactory *factory)
{
  // Register Factory's filters
  factory->registerFilter(this, SeedGrowSegmentationCommand::FILTER_TYPE);
  factory->registerFilter(this, TubularSegmentationFilter::FILTER_TYPE);


  // Register settings panels
  factory->registerSettingsPanel(m_SeedGrowSettingsPanel.get());
}

//-----------------------------------------------------------------------------
FilterSPtr SegmentationToolBar::createFilter(const QString              &filter,
                                                   const Filter::NamedInputs  &inputs,
                                                   const ModelItem::Arguments &args)
{
  if (SeedGrowSegmentationCommand::FILTER_TYPE == filter)
  {
    SeedGrowSegmentationFilter *sgsFilter = new SeedGrowSegmentationFilter(inputs, args, SeedGrowSegmentationCommand::FILTER_TYPE);
    SetBasicGraphicalRepresentationFactory(sgsFilter);

    Filter::FilterInspectorPtr sgsInspector(new SGSFilterInspector(sgsFilter));
    sgsFilter->setFilterInspector(sgsInspector);

    return FilterSPtr(sgsFilter);
  }

  if (TubularSegmentationFilter::FILTER_TYPE == filter)
  {
    TubularSegmentationFilter::Pointer tubularFilter = TubularSegmentationFilter::Pointer(new TubularSegmentationFilter(inputs, args, TubularSegmentationFilter::FILTER_TYPE));
    Q_ASSERT(m_tubularTool);

    Filter::FilterInspectorPtr inspector = Filter::FilterInspectorPtr(new TubularFilterInspector(tubularFilter, m_undoStack, m_viewManager, m_tubularTool));
    tubularFilter->setFilterInspector(inspector);

    return FilterSPtr(tubularFilter);
  }

  return FilterSPtr();
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::changePicker(QAction* action)
{
  Q_ASSERT(m_pickers.contains(action));
  m_SGStool->setChannelPicker(m_pickers[action]);
  m_viewManager->setActiveTool(m_SGStool);
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::cancelSegmentationOperation()
{
  m_pickerSelector->cancel();
  m_viewManager->unsetActiveTool(m_SGStool);
}

//------------------------------------------------------------------------
void SegmentationToolBar::reset()
{
  cancelSegmentationOperation();
  cancelTubularSegmentationOperation();
}

//-----------------------------------------------------------------------------
QList<MenuEntry> SegmentationToolBar::menuEntries()
{
  QList<MenuEntry> entries;

  QStringList hierarchy;
  hierarchy << "Analysis" << "Tubular Segmentation Nodes";

  QAction *action = new QAction(tr("Tubular Segmentation Nodes"), this);
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(showNodesInformation()));

  entries << MenuEntry(hierarchy, action);

  return entries;
}

//------------------------------------------------------------------------
void SegmentationToolBar::batchMode()
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
void SegmentationToolBar::addVoxelPicker(QAction* action, IPickerSPtr picker)
{
  m_pickerSelector->addAction(action);
  m_pickers[action] = picker;
  picker->setMultiSelection(false);
  picker->setPickable(ISelector::CHANNEL);
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::buildPickers()
{
  QAction *action;

  // Exact Pixel Picker
  action = new QAction(QIcon(":/espina/pixelSelector.svg"),
                       tr("Add synapse (Ctrl +). Exact Pixel"),
                       m_pickerSelector);
  PixelSelector *picker = new PixelSelector();
  addVoxelPicker(action, IPickerSPtr(picker));

  // Best Pixel Picker
  action = new QAction(QIcon(":/espina/bestPixelSelector.svg"),
                       tr("Add synapse (Ctrl +). Best Pixel"),
                       m_pickerSelector);
  BestPixelSelector *bestPicker = new BestPixelSelector();
  bestPicker->setCursor(QCursor(QPixmap(":/espina/crossRegion.svg")));
  addVoxelPicker(action, IPickerSPtr(bestPicker));

  m_settings = new SeedGrowSegmentationSettings(bestPicker);
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::tubularActionStateChanged(bool segmenting)
{
  if (segmenting)
  {
    m_viewManager->setActiveTool(m_tubularTool);
    m_viewManager->setSelectionEnabled(false);
  }
  else
  {
    m_viewManager->unsetActiveTool(m_tubularTool);
    m_viewManager->setSelectionEnabled(true);
  }
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::showNodesInformation()
{
  if (m_tubularTool == NULL)
    return;

  m_tubularTool->showSpineInformation();
}

//-----------------------------------------------------------------------------
void SegmentationToolBar::cancelTubularSegmentationOperation()
{
  if (m_tubularAction->isChecked())
  {
    m_tubularAction->setChecked(false);
    if (m_tubularTool != NULL)
    {
      m_tubularTool->Reset();
      m_viewManager->unsetActiveTool(m_tubularTool);
    }
  }
}
