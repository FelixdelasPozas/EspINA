/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "SeedGrowSegmentationTool.h"
#include "SeedGrowSegmentationSettings.h"
#include "SeedGrowSegmentationHistoryWidget.h"
#include "SeedGrowSegmentationHistory.h"
#include <ToolGroups/ROI/ROITools.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/Utils/ModelAdapterUtils.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/FilterHistory.h>
#include <App/Settings/ROI/ROISettings.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Undo/AddSegmentations.h>

// Qt
#include <QAction>
#include <QUndoStack>
#include <QSettings>
#include <QMessageBox>

using namespace ESPINA;

const Filter::Type SGS_FILTER    = "SeedGrowSegmentation";
const Filter::Type SGS_FILTER_V4 = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

//-----------------------------------------------------------------------------
FilterTypeList SeedGrowSegmentationTool::SGSFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SGS_FILTER;
  filters << SGS_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SeedGrowSegmentationTool::SGSFactory::createFilter(InputSList          inputs,
                                                              const Filter::Type& filter,
                                                              SchedulerSPtr       scheduler) const throw (Unknown_Filter_Exception)
{
  if (!(filter == SGS_FILTER || filter == SGS_FILTER_V4)) throw Unknown_Filter_Exception();

  auto sgsFilter = std::make_shared<SeedGrowSegmentationFilter>(inputs, filter, scheduler);

  if (!m_dataFactory)
  {
    m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
  }
  sgsFilter->setDataFactory(m_dataFactory);

  return sgsFilter;
}

//-----------------------------------------------------------------------------
QList<Filter::Type> SeedGrowSegmentationTool::SGSFactory::availableFilterDelegates() const
{
  QList<Filter::Type> types;

  types << SGS_FILTER << SGS_FILTER_V4;

  return types;
}

//-----------------------------------------------------------------------------
FilterDelegateSPtr SeedGrowSegmentationTool::SGSFactory::createDelegate(FilterSPtr filter)
throw (Unknown_Filter_Type_Exception)
{
  if (!availableFilterDelegates().contains(filter->type())) throw Unknown_Filter_Type_Exception();

  auto sgsFilter = std::dynamic_pointer_cast<SeedGrowSegmentationFilter>(filter);

  return std::make_shared<SeedGrowSegmentationHistory>(sgsFilter);
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool(SeedGrowSegmentationSettings* settings,
                                                   ModelAdapterSPtr              model,
                                                   ModelFactorySPtr              factory,
                                                   FilterDelegateFactorySPtr     filterDelegateFactory,
                                                   ViewManagerSPtr               viewManager,
                                                   QUndoStack*                   undoStack)
: m_model           {model}
, m_factory         {factory}
, m_viewManager     {viewManager}
, m_undoStack       {undoStack}
, m_categorySelector{new CategorySelector(m_model)}
, m_selectorSwitch  {new ActionSelector()}
, m_seedThreshold   {new SeedThreshold()}
, m_roi             {new CustomROIWidget()}
, m_settings        {settings}
, m_sgsFactory      {new SGSFactory()}
{
  m_factory->registerFilterFactory(m_sgsFactory);
  filterDelegateFactory->registerFilterDelegateFactory(m_sgsFactory);

  { // Pixel Selector
    QAction *action = new QAction(QIcon(":/espina/pixelSelector.svg"),
                                  tr("Create segmentation based on selected pixel (Ctrl +)"),
                                  m_selectorSwitch);

    auto selector = std::make_shared<PixelSelector>();
    selector->setMultiSelection(false);

    addVoxelSelector(action, selector);
  }


  { // Best Pixel Selector
    QAction *action = new QAction(QIcon(":/espina/bestPixelSelector.svg"),
                                  tr("Create segmentation based on best pixel (Ctrl +)"),
                                  m_selectorSwitch);

    auto selector = std::make_shared<BestPixelSelector>();
    selector->setMultiSelection(false);

    QCursor cursor(QPixmap(":/espina/crossRegion.svg"));

    selector->setCursor(cursor);

    selector->setBestPixelValue(m_settings->bestPixelValue());

    connect(m_settings, SIGNAL(bestValueChanged(int)),
            selector.get(), SLOT(setBestPixelValue(int)));

    addVoxelSelector(action, selector);

    m_selectorSwitch->setDefaultAction(action);
  }

  m_roi->setValue(Axis::X, m_settings->xSize());
  m_roi->setValue(Axis::Y, m_settings->ySize());
  m_roi->setValue(Axis::Z, m_settings->zSize());

  connect(m_settings, SIGNAL(applyCategoryROIChanged(bool)),
          this,       SLOT(updateCurrentCategoryROIValues(bool)));

  connect(m_selectorSwitch, SIGNAL(triggered(QAction*)),
          this, SLOT(changeSelector(QAction*)));

  connect(m_selectorSwitch, SIGNAL(actionCanceled()),
          this, SLOT(unsetSelector()));
  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(onCategoryChanged(CategoryAdapterSPtr)));
  connect(m_categorySelector, SIGNAL(widgetCreated()),
          this, SLOT(onCategorySelectorWidgetCreation()));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::~SeedGrowSegmentationTool()
{
  delete m_categorySelector;
  delete m_selectorSwitch;
  delete m_seedThreshold;
  delete m_roi;
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
  actions << m_roi;

  return actions;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onToolEnabled(bool enabled)
{
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::addVoxelSelector(QAction* action, SelectorSPtr selector)
{
  m_selectorSwitch->addAction(action);
  m_voxelSelectors[action] = selector;

  selector->setMultiSelection(false);
  selector->setSelectionTag(Selector::CHANNEL);

  connect(selector.get(),   SIGNAL(eventHandlerInUse(bool)),
          m_selectorSwitch, SLOT(setChecked(bool)));
  connect(selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,           SLOT(launchTask(Selector::Selection)));
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
void SeedGrowSegmentationTool::launchTask(Selector::Selection selectedItems)
{
  if (selectedItems.size() != 1)
    return;

  auto element = selectedItems.first();

  Q_ASSERT(element.first->numberOfVoxels() == 1); // with one pixel

  auto pointBounds = element.first->bounds();
  NmVector3 seedPoint{ (pointBounds[0]+pointBounds[1])/2, (pointBounds[2]+pointBounds[3])/2, (pointBounds[4]+pointBounds[5])/2};

  Q_ASSERT(ItemAdapter::Type::CHANNEL == element.second->type());
  auto channel = m_viewManager->activeChannel();

  if (!channel)
    return;

  // FIXME: merged analysis channel's don't have outputs????
  Q_ASSERT(channel->output());

  auto volume = volumetricData(channel->output());

  NmVector3 seed;
  Bounds    seedBounds;
  ROISPtr   roi;

  for (int i = 0; i < 3; ++i)
  {
    seed[i] = seedBounds[2*i] = seedBounds[2*i+1] = seedPoint[i];
  }
  seedBounds.setUpperInclusion(true);

  auto currentROI = m_viewManager->currentROI();

  if (!currentROI && m_roi->applyROI())
  {
    // Create default ROI
    auto xSize = std::max(m_roi->value(Axis::X), (unsigned int) 2);
    auto ySize = std::max(m_roi->value(Axis::Y), (unsigned int) 2);
    auto zSize = std::max(m_roi->value(Axis::Z), (unsigned int) 2);

    Bounds bounds{seed[0]-xSize/2, seed[0]+xSize/2,
                  seed[1]-ySize/2, seed[1]+ySize/2,
                  seed[2]-zSize/2, seed[2]+zSize/2};

    auto spacing = channel->output()->spacing();
    auto origin  = channel->position();

    bounds = intersection(bounds, channel->bounds(), spacing);

    roi = std::make_shared<ROI>(bounds, spacing, origin);
  }
  else
  {
    roi = currentROI;
  }

  auto validSeed = true;

  if(roi != nullptr)
  {
    validSeed = contains(roi.get(), seed, volume->spacing());
  }

  validSeed &= contains(volume->bounds(), seedBounds, volume->spacing());

  if (validSeed)
  {
    //m_selectorSwitch->setEnabled(false);

    InputSList inputs;

    inputs << channel->asInput();

    auto filter = m_factory->createFilter<SeedGrowSegmentationFilter>(inputs, SGS_FILTER);

    filter->setSeed(seed);
    filter->setUpperThreshold(m_seedThreshold->upperThreshold());
    filter->setLowerThreshold(m_seedThreshold->lowerThreshold());
    filter->setDescription(tr("Seed Grow Segmentation"));
    if(roi != nullptr)
    {
      filter->setROI(roi->clone());
    }

    m_executingTasks[filter.get()] = filter;
    m_executingFilters[filter.get()] = filter;

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(createSegmentation()));

    Task::submit(filter);

    if (currentROI)
    {
      m_viewManager->consumeROI();
    }
  }
  else
  {
    QMessageBox box;
    box.setWindowTitle(tr("Seed Grow Segmentation"));
    box.setText(tr("The seed is not inside the channel or the region of interest."));
    box.setStandardButtons(QMessageBox::Ok);
    box.setIcon(QMessageBox::Information);
    box.exec();
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::createSegmentation()
{
  auto filter = dynamic_cast<FilterPtr>(sender());

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

//     m_viewManager->updateSegmentationRepresentations(segmentation.get());
//     m_viewManager->updateViews();

    auto sgsFilter = m_executingFilters[filter];
    if(sgsFilter->isTouchingROI())
    {
      QMessageBox box;
      box.setWindowTitle(tr("Seed Grow Segmentation"));
      box.setText(tr("The segmentation \"%1\" is incomplete because\nis touching the ROI or an edge of the channel.").arg(segmentation->data().toString()));
      box.setStandardButtons(QMessageBox::Ok);
      box.setIcon(QMessageBox::Information);
      box.exec();
    }
  }

  m_executingFilters.remove(filter);
  m_executingTasks.remove(filter);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onCategoryChanged(CategoryAdapterSPtr category)
{
  if (m_settings->applyCategoryROI())
  {
    QVariant xSize = category->property(Category::DIM_X());
    QVariant ySize = category->property(Category::DIM_Y());
    QVariant zSize = category->property(Category::DIM_Z());

    if (!xSize.isValid() || !ySize.isValid() || !zSize.isValid())
    {
      ESPINA_SETTINGS(settings);
      settings.beginGroup(ROI_SETTINGS_GROUP);

      xSize   = settings.value(DEFAULT_ROI_X, 500).toInt();
      ySize   = settings.value(DEFAULT_ROI_Y, 500).toInt();
      zSize   = settings.value(DEFAULT_ROI_Z, 500).toInt();
    }

    m_roi->setValue(Axis::X, xSize.toInt());
    m_roi->setValue(Axis::Y, ySize.toInt());
    m_roi->setValue(Axis::Z, zSize.toInt());
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onCategorySelectorWidgetCreation()
{
  onCategoryChanged(m_categorySelector->selectedCategory());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::updateCurrentCategoryROIValues(bool applyCategoryROI)
{
  if (applyCategoryROI)
  {
    onCategorySelectorWidgetCreation();
  }
  else
  {
    m_roi->setValue(Axis::X, m_settings->xSize());
    m_roi->setValue(Axis::Y, m_settings->ySize());
    m_roi->setValue(Axis::Z, m_settings->zSize());
  }
}
