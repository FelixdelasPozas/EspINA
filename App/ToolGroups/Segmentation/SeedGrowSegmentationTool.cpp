/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "SeedGrowSegmentationTool.h"

#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/Utils/ModelAdapterUtils.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>
#include <Undo/AddSegmentations.h>

#include <QAction>
#include <QUndoStack>

using namespace EspINA;

const Filter::Type SGS_FILTER    = "SeedGrowSegmentation";
const Filter::Type SGS_FILTER_V4 = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

//-----------------------------------------------------------------------------
FilterTypeList SeedGrowSegmentationTool::SGSFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SGS_FILTER;
  filters << SGS_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SeedGrowSegmentationTool::SGSFilterFactory::createFilter(InputSList         inputs,
                                                  const Filter::Type& filter,
                                                  SchedulerSPtr       scheduler) const throw (Unknown_Filter_Exception)
{
  if (!(filter == SGS_FILTER || filter == SGS_FILTER_V4)) throw Unknown_Filter_Exception();

  auto sgsFilter = FilterSPtr{new SeedGrowSegmentationFilter(inputs, filter, scheduler)};

  if (!m_fetchBehaviour)
  {
    m_fetchBehaviour = FetchBehaviourSPtr{new MarchingCubesFromFetchedVolumetricData()};
  }
  sgsFilter->setFetchBehaviour(m_fetchBehaviour);

  return sgsFilter;
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool(ModelAdapterSPtr model,
                                                   ModelFactorySPtr factory,
                                                   ViewManagerSPtr  viewManager,
                                                   QUndoStack      *undoStack)
: m_model(model)
, m_factory(factory)
, m_viewManager(viewManager)
, m_undoStack(undoStack)
, m_enabled(false)
, m_categorySelector(new CategorySelector(m_model))
, m_selectorSwitch(new ActionSelector())
, m_seedThreshold(new SeedThreshold())
, m_applyVOI(new ApplyROI())
, m_filterFactory(new SGSFilterFactory())
{
  m_factory->registerFilterFactory(m_filterFactory);

  { // Pixel Selector
    QAction *action = new QAction(QIcon(":/espina/pixelSelector.svg"),
                                  tr("Create segmentation based on selected pixel (Ctrl +)"),
                                  m_selectorSwitch);

    SelectorSPtr selector{new PixelSelector()};

    addVoxelSelector(action, selector);
  }


  { // Best Pixel Selector
    QAction *action = new QAction(QIcon(":/espina/bestPixelSelector.svg"),
                                  tr("Create segmentation based on best pixel (Ctrl +)"),
                                  m_selectorSwitch);

    SelectorSPtr selector{new BestPixelSelector()};
    QCursor      cursor(QPixmap(":/espina/crossRegion.svg"));

    selector->setCursor(cursor);

    addVoxelSelector(action, selector);

    m_selectorSwitch->setDefaultAction(action);
  }

  connect(m_selectorSwitch, SIGNAL(triggered(QAction*)),
          this, SLOT(changeSelector(QAction*)));

  connect(m_selectorSwitch, SIGNAL(actionCanceled()),
          this, SLOT(unsetSelector()));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::~SeedGrowSegmentationTool()
{
  delete m_selectorSwitch;
  delete m_seedThreshold;
  delete m_applyVOI;
}

//-----------------------------------------------------------------------------
QList<QAction *> SeedGrowSegmentationTool::actions() const
{
  QList<QAction *> actions;

  if (m_currentSelector)
  {
    m_selectorSwitch->setChecked(m_viewManager->eventHandler() == m_currentSelector);
  }

  actions << m_categorySelector;
  actions << m_selectorSwitch;
  actions << m_seedThreshold;
  actions << m_applyVOI;

  return actions;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationTool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setEnabled(bool value)
{
  m_enabled = value;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::addVoxelSelector(QAction* action, SelectorSPtr selector)
{
  m_selectorSwitch->addAction(action);
  m_voxelSelectors[action] = selector;

  selector->setMultiSelection(false);
  selector->setSelectionTag(Selector::CHANNEL);

  connect(selector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_selectorSwitch, SLOT(setChecked(bool)));
  connect(selector.get(), SIGNAL(itemsSelected(Selector::SelectionList)),
          this, SLOT(launchTask(Selector::SelectionList)));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::changeSelector(QAction* action)
{
  m_currentSelector = m_voxelSelectors[action];

  m_viewManager->setEventHandler(m_currentSelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::unsetSelector()
{
  m_viewManager->unsetEventHandler(m_currentSelector);
  m_currentSelector.reset();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::launchTask(Selector::SelectionList selectedItems)
{
  if (selectedItems.size() != 1)
    return;

  auto element = selectedItems.first();
  auto input   = element.second;

  Q_ASSERT(element.first->GetNumberOfPoints() == 1); // with one pixel

  Nm seedPoint[3];
  element.first->GetPoint(0, seedPoint);

  Q_ASSERT(ItemAdapter::Type::CHANNEL == input->type());
  auto channel = m_viewManager->activeChannel();

  if (!channel)
    return;

  auto volume = volumetricData(channel->output());

  NmVector3 seed;
  Bounds seedBounds;

  for (int i = 0; i < 3; ++i)
  {
    seed[i] = seedBounds[2*i] = seedBounds[2*i+1] = seedPoint[i];
  }
  seedBounds.setUpperInclusion(true);

  //auto seedVoxel = volume->itkImage(seedBounds);
  ROI roi = m_viewManager->currentROI();

  if (!roi && m_applyVOI->isChecked())
  {
    // TODO: Create default ROI
  }

  bool validSeed = //TODO (roi && contains(roi, seed)) ||
                   contains(volume->bounds(), seed);

  if (validSeed)
  {
    //m_selectorSwitch->setEnabled(false);

    InputSList inputs;

    inputs << channel->asInput();

    auto adapter = m_factory->createFilter<SeedGrowSegmentationFilter>(inputs, SGS_FILTER);
    auto filter  = adapter->get();

    filter->setSeed(seed);
    filter->setUpperThreshold(m_seedThreshold->upperThreshold());
    filter->setLowerThreshold(m_seedThreshold->lowerThreshold());
    filter->setDescription(tr("Seed Grow Segmentation"));

    m_executingTasks[adapter.get()] = adapter;

    connect(adapter.get(), SIGNAL(progress(int)),
            this,   SLOT(onTaskProgres(int)));
    connect(adapter.get(), SIGNAL(finished()),
            this,   SLOT(createSegmentation()));

    adapter->submit();
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onTaskProgres(int progress)
{
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::createSegmentation()
{
  auto filter = dynamic_cast<FilterAdapterPtr>(sender());

  if (!filter->isAborted())
  {
    auto adapter = m_executingTasks[filter];

    if (filter->numberOfOutputs() != 1) throw Filter::Undefined_Output_Exception();

    auto segmentation = m_factory->createSegmentation(adapter, 0);

    auto category = m_categorySelector->selectedCategory();
    Q_ASSERT(category);

    segmentation->setCategory(category);

    SampleAdapterSList samples;
    samples << QueryAdapter::sample(m_viewManager->activeChannel());
    Q_ASSERT(samples.size() == 1);

    m_undoStack->beginMacro(tr("Add Segmentation"));
    m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));
    m_undoStack->endMacro();

    m_viewManager->updateSegmentationRepresentations(segmentation.get());
    m_viewManager->updateViews();
  }

  m_executingTasks.remove(filter);

//   m_undoStack->beginMacro(tr("Seed Grow Segmentation"));
//    m_undoStack->push(new SeedGrowSegmentationCommand(channel,
//                                                      seed,
//                                                      voiExtent,
//                                                      m_threshold->lowerThreshold(),
//                                                      m_threshold->upperThreshold(),
//                                                      m_settings->closing(),
//                                                      m_viewManager->activeTaxonomy(),
//                                                      m_model,
//                                                      m_viewManager,
//                                                      createdSegmentations));
//    m_model->emitSegmentationAdded(createdSegmentations);
//    m_undoStack->endMacro();
   //m_selectorSwitch->setEnabled(true);
}


