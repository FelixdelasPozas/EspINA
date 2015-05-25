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
#include <ToolGroups/Restrict/RestrictToolGroup.h>
#include <ToolGroups/Restrict/OrthogonalROITool.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/CategorySelector.h>
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
#include <QHBoxLayout>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support::Widgets;

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
FilterDelegateSPtr SeedGrowSegmentationTool::SGSFactory::createDelegate(SegmentationAdapterPtr segmentation,
                                                                        FilterSPtr             filter)
throw (Unknown_Filter_Type_Exception)
{
  if (!availableFilterDelegates().contains(filter->type())) throw Unknown_Filter_Type_Exception();

  auto sgsFilter = std::dynamic_pointer_cast<SeedGrowSegmentationFilter>(filter);

  return std::make_shared<SeedGrowSegmentationHistory>(segmentation, sgsFilter);
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool(SeedGrowSegmentationSettings* settings,
                                                   FilterDelegateFactorySPtr     filterDelegateFactory,
                                                   Support::Context       &context)
: ProgressTool(":/espina/pixelSelector.svg", tr("Create segmentation based on selected pixel"))
, m_context         (context)
, m_categorySelector{new CategorySelector(context.model())}
, m_seedThreshold   {new SeedThreshold()}
, m_roi             {new CustomROIWidget()}
, m_settings        {settings}
, m_sgsFactory      {new SGSFactory()}
{
  setCheckable(true);

  m_context.factory()->registerFilterFactory(m_sgsFactory);
  filterDelegateFactory->registerFilterDelegateFactory(m_sgsFactory);

  initPixelSelectors();

  initSettingsWidgets();

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(setEventHandler(bool)));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::~SeedGrowSegmentationTool()
{
  delete m_categorySelector;
  delete m_seedThreshold;
  delete m_roi;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::abortOperation()
{
  setEventHandler(false);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onToolEnabled(bool enabled)
{
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initPixelSelectors()
{
  initPixelSelector();
  initBestPixelSelctor();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initPixelSelector()
{
  m_pixelSelector = std::make_shared<PixelSelector>();

  initSelector(m_pixelSelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initBestPixelSelctor()
{
  QCursor cursor(QPixmap(":/espina/crossRegion.svg"));

  auto selector = std::make_shared<BestPixelSelector>();

  selector->setCursor(cursor);
  selector->setBestPixelValue(m_settings->bestPixelValue());

  connect(m_settings,     SIGNAL(bestValueChanged(int)),
          selector.get(), SLOT(setBestPixelValue(int)));

  m_bestPixelSelctor = selector;

  initSelector(m_bestPixelSelctor);
}

void SeedGrowSegmentationTool::initSelector(SelectorSPtr selector)
{
  selector->setMultiSelection(false);
  selector->setSelectionTag(Selector::CHANNEL);

  connect(selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,           SLOT(launchTask(Selector::Selection)));

  //   connect(selector.get(),   SIGNAL(eventHandlerInUse(bool)),
//           m_selectorSwitch, SLOT(setChecked(bool)));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initSettingsWidgets()
{
  initCategorySelector();

  addSettingsWidget(m_seedThreshold);

  initROISelector();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initCategorySelector()
{
  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(onCategoryChanged(CategoryAdapterSPtr)));

  addSettingsWidget(m_categorySelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initROISelector()
{
  m_roi->setValue(Axis::X, m_settings->xSize());
  m_roi->setValue(Axis::Y, m_settings->ySize());
  m_roi->setValue(Axis::Z, m_settings->zSize());

  connect(m_settings, SIGNAL(applyCategoryROIChanged(bool)),
          this,       SLOT(updateCurrentCategoryROIValues(bool)));

  addSettingsWidget(m_roi);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::launchTask(Selector::Selection selectedItems)
{
  if (selectedItems.size() != 1) return;

  auto element = selectedItems.first();

  Q_ASSERT(element.first->numberOfVoxels() == 1); // with one pixel

  auto pointBounds = element.first->bounds();
  NmVector3 seedPoint{(pointBounds[0]+pointBounds[1])/2, (pointBounds[2]+pointBounds[3])/2, (pointBounds[4]+pointBounds[5])/2};

  Q_ASSERT(isChannel(element.second));
  auto channel = inputChannel();

  if (!channel) return;

  // FIXME: merged analysis channel's don't have outputs????
  Q_ASSERT(channel->output());

  auto volume = readLockVolume(channel->output());

  NmVector3 seed;
  Bounds    seedBounds;

  for (int i = 0; i < 3; ++i)
  {
    seed[i] = seedBounds[2*i] = seedBounds[2*i+1] = seedPoint[i];
  }
  seedBounds.setUpperInclusion(true);

  auto currentROI = m_context.roiProvider()->currentROI();

  if (!currentROI && m_roi->applyROI())
  {
    // Create default ROI
    auto xSize = std::max(m_roi->value(Axis::X), (unsigned int) 2);
    auto ySize = std::max(m_roi->value(Axis::Y), (unsigned int) 2);
    auto zSize = std::max(m_roi->value(Axis::Z), (unsigned int) 2);

    auto bounds  = OrthogonalROITool::createRegion(seed, xSize, ySize, zSize);
    auto spacing = channel->output()->spacing();
    auto origin  = channel->position();

    bounds = intersection(bounds, channel->bounds(), spacing);

    currentROI = std::make_shared<ROI>(bounds, spacing, origin);
  }

  auto validSeed = true;

  if(currentROI)
  {
    validSeed = contains(currentROI.get(), seed, volume->spacing());
  }

  validSeed &= contains(volume->bounds(), seedBounds, volume->spacing());

  if (validSeed)
  {
    //m_selectorSwitch->setEnabled(false);

    InputSList inputs;

    inputs << channel->asInput();

    auto filter = m_context.factory()->createFilter<SeedGrowSegmentationFilter>(inputs, SGS_FILTER);

    filter->setSeed(seed);
    filter->setUpperThreshold(m_seedThreshold->upperThreshold());
    filter->setLowerThreshold(m_seedThreshold->lowerThreshold());
    filter->setDescription(tr("Seed Grow Segmentation"));

    if(currentROI)
    {
      filter->setROI(currentROI->clone());
    }

    m_executingTasks[filter.get()]   = filter;
    m_executingFilters[filter.get()] = filter;

    showTaskProgress(filter);

    connect(filter.get(), SIGNAL(finished()),
            this,         SLOT(createSegmentation()));

    Task::submit(filter);

    if (currentROI == m_context.roiProvider()->currentROI())
    {
      m_context.roiProvider()->clear();
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

    auto segmentation = m_context.factory()->createSegmentation(adapter, 0);

    auto category = m_categorySelector->selectedCategory();
    Q_ASSERT(category);

    segmentation->setCategory(category);

    SampleAdapterSList samples;
    samples << QueryAdapter::sample(inputChannel());
    Q_ASSERT(samples.size() == 1);

    auto undoStack = m_context.undoStack();
    undoStack->beginMacro(tr("Add Segmentation"));
    undoStack->push(new AddSegmentations(segmentation, samples, m_context.model()));
    undoStack->endMacro();

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
void SeedGrowSegmentationTool::updateCurrentCategoryROIValues(bool applyCategoryROI)
{
  if (applyCategoryROI)
  {
    onCategoryChanged(m_categorySelector->selectedCategory());
  }
  else
  {
    m_roi->setValue(Axis::X, m_settings->xSize());
    m_roi->setValue(Axis::Y, m_settings->ySize());
    m_roi->setValue(Axis::Z, m_settings->zSize());
  }
}

//-----------------------------------------------------------------------------
ChannelAdapterPtr SeedGrowSegmentationTool::inputChannel() const
{
  return getActiveChannel(m_context);
}

//-----------------------------------------------------------------------------
SelectorSPtr SeedGrowSegmentationTool::activeSelector() const
{
  return m_bestPixelSelctor;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setEventHandler(bool value)
{
  if (value)
  {
    m_context.viewState().setEventHandler(activeSelector());
  }
  else
  {
    m_context.viewState().unsetEventHandler(activeSelector());
  }
}
