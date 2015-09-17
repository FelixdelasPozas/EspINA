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
#include "SeedGrowSegmentationRefineWidget.h"
#include "CustomROIWidget.h"
#include "SeedGrowSegmentationRefiner.h"
#include <ToolGroups/Restrict/RestrictToolGroup.h>
#include <ToolGroups/Restrict/OrthogonalROITool.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/PixelValueSelector.h>
#include <GUI/Widgets/NumericalInput.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/FilterRefiner.h>
#include <App/Settings/ROI/ROISettings.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Undo/AddSegmentations.h>

// Qt
#include <QAction>
#include <QUndoStack>
#include <QSettings>
#include <QMessageBox>
#include <QCheckBox>
#include <QHBoxLayout>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

const Filter::Type SGS_FILTER    = "SeedGrowSegmentation";
const Filter::Type SGS_FILTER_V4 = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

const QString UPPER_THRESHOLD = "Upper threshold";
const QString LOWER_THRESHOLD = "Lower threshold";
const QString XSIZE           = "ROI X Size";
const QString YSIZE           = "ROI Y Size";
const QString ZSIZE           = "ROI Z Size";
const QString APPLY_ROI       = "Apply category ROI";
const QString APPLY_CLOSE     = "Apply close";
const QString CLOSE_RADIUS    = "Close radius";
const QString BEST_VALUE      = "Best value";
const QString CATEGORY        = "Category selected";

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
SeedGrowSegmentationTool::SeedGrowSegmentationTool(SeedGrowSegmentationSettings* settings,
                                                   FilterRefinerRegister        &filterRefiners,
                                                   Support::Context             &context)
: ProgressTool("1-GreyLevelSegmentation", ":/espina/grey_level_segmentation.svg", tr("Grey Level Segmentation"), context)
, m_context         (context)
, m_categorySelector{new CategorySelector(context.model())}
, m_seedThreshold   {new SeedThreshold()}
, m_useBestPixel    {Styles::createToolButton(":espina/best_pixel_selector.svg", tr("Apply on best pixel"))}
, m_colorLabel      {new QLabel(tr("Pixel Color:"))}
, m_colorSelector   {new PixelValueSelector()}
, m_roi             {new CustomROIWidget()}
, m_applyClose      {Styles::createToolButton(":espina/morphological_close.svg", tr("Apply close"))}
, m_close           {new NumericalInput()}
, m_settings        {settings}
, m_sgsFactory      {new SGSFactory()}
{
  setCheckable(true);
  setExclusive(true);

  m_context.factory()->registerFilterFactory(m_sgsFactory);

  auto sgsRefiner = std::make_shared<SeedGrowSegmentationRefiner>();

  for (auto type : m_sgsFactory->providedFilters())
  {
    filterRefiners.registerFilterRefiner(sgsRefiner, type);
  }

  initPixelSelectors();

  m_activeSelector = m_bestPixelSelector;

  initSettingsWidgets();

  setEventHandler(activeSelector());
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
  deactivateEventHandler();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initPixelSelectors()
{
  initPixelSelector();
  initBestPixelSelector();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initPixelSelector()
{
  QCursor cursor(QPixmap(":/espina/crossRegion.svg"));
  m_pixelSelector = std::make_shared<PixelSelector>();
  m_pixelSelector->setCursor(cursor);

  initSelector(m_pixelSelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initBestPixelSelector()
{
  QCursor cursor(QPixmap(":/espina/crossRegion.svg"));

  auto selector = std::make_shared<BestPixelSelector>();

  selector->setCursor(cursor);
  selector->setBestPixelValue(m_settings->bestPixelValue());

  connect(m_settings,     SIGNAL(bestValueChanged(int)),
          selector.get(), SLOT(setBestPixelValue(int)));

  m_bestPixelSelector = selector;

  initSelector(m_bestPixelSelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initSelector(SelectorSPtr selector)
{
  selector->setMultiSelection(false);
  selector->setSelectionTag(Selector::CHANNEL);

  connect(selector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,           SLOT(launchTask(Selector::Selection)));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initSettingsWidgets()
{
  initCategorySelector();

  addSettingsWidget(m_seedThreshold);

  initBestPixelWidgets();

  initROISelector();

  initCloseWidgets();
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
void SeedGrowSegmentationTool::initBestPixelWidgets()
{
  auto enabled = (m_bestPixelSelector.get() == activeSelector().get());

  m_useBestPixel->setCheckable(true);
  m_useBestPixel->setChecked(enabled);

  connect(m_useBestPixel, SIGNAL(toggled(bool)),
          this,           SLOT(useBestPixelSelector(bool)));

  m_colorLabel->setVisible(enabled);

  m_colorSelector->setVisible(enabled);
  Styles::setBarStyle(m_colorSelector);

  connect(m_colorSelector, SIGNAL(newValue(int)),
          this,            SLOT(onNewPixelValue(int)));

  m_colorSelector->setValue(m_settings->bestPixelValue());

  addSettingsWidget(m_useBestPixel);
  addSettingsWidget(m_colorLabel);
  addSettingsWidget(m_colorSelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initCloseWidgets()
{
  auto enabled = (m_settings->applyClose());

  m_applyClose->setCheckable(true);
  m_applyClose->setChecked(enabled);

  connect(m_applyClose, SIGNAL(toggled(bool)),
          this,         SLOT(onCloseStateChanged(bool)));

  m_close->setVisible(enabled);
  m_close->setLabelText(tr("Radius"));
  m_close->setSliderVisibility(false);
  m_close->setMinimum(1);
  m_close->setMaximum(10);

  addSettingsWidget(m_applyClose);
  addSettingsWidget(m_close);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::launchTask(Selector::Selection selectedItems)
{
  if (selectedItems.size() != 1) return;

  auto element = selectedItems.first();

  Q_ASSERT(element.first->numberOfVoxels() == 1); // with one pixel

  auto seedPoint = centroid(element.first->bounds());

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

  auto volumeSpacing = volume->bounds().spacing();

  if(currentROI)
  {
    validSeed = contains(currentROI.get(), seed, volumeSpacing);
  }

  validSeed &= contains(volume->bounds(), seedBounds, volumeSpacing);

  if (validSeed)
  {
    auto filter = m_context.factory()->createFilter<SeedGrowSegmentationFilter>(channel, SGS_FILTER);

    filter->setSeed(seed);
    filter->setUpperThreshold(m_seedThreshold->upperThreshold());
    filter->setLowerThreshold(m_seedThreshold->lowerThreshold());
    filter->setDescription(tr("Grey Level Segmentation"));

    if(m_applyClose->isChecked())
    {
      filter->setClosingRadius(m_close->value());
    }

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
    auto title = tr("Grey Level Segmentation");
    auto msg   = tr("The seed is not inside the channel or the region of interest.");

    GUI::DefaultDialogs::InformationMessage(msg, title);
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

    auto sgsFilter = m_executingFilters[filter];
    if(sgsFilter->isTouchingROI())
    {
      QMessageBox box;
      box.setWindowTitle(tr("Grey Level Segmentation"));
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

      xSize = settings.value(DEFAULT_ROI_X_SIZE_KEY, 500).toInt();
      ySize = settings.value(DEFAULT_ROI_Y_SIZE_KEY, 500).toInt();
      zSize = settings.value(DEFAULT_ROI_Z_SIZE_KEY, 500).toInt();
    }

    m_roi->setValue(Axis::X, xSize.toInt());
    m_roi->setValue(Axis::Y, ySize.toInt());
    m_roi->setValue(Axis::Z, zSize.toInt());
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_seedThreshold->setUpperThreshold(settings->value(UPPER_THRESHOLD, 30).toInt());
  m_seedThreshold->setLowerThreshold(settings->value(LOWER_THRESHOLD, 30).toInt());

  auto roiX = settings->value(XSIZE,500).toInt();
  m_roi->setValue(Axis::X, roiX);
  m_settings->setXSize(roiX);

  auto roiY = settings->value(YSIZE,500).toInt();
  m_roi->setValue(Axis::Y, roiY);
  m_settings->setYSize(roiY);

  auto roiZ = settings->value(ZSIZE,500).toInt();
  m_roi->setValue(Axis::Z, roiZ);
  m_settings->setZSize(roiZ);

  auto applyROI = settings->value(APPLY_ROI, true).toBool();
  m_roi->setApplyROI(applyROI);
  m_settings->setApplyCategoryROI(applyROI);

  auto applyClose = settings->value(APPLY_CLOSE, false).toBool();
  m_settings->setApplyClose(applyClose);

  auto radius = settings->value(CLOSE_RADIUS, 0).toInt();
  m_settings->setCloseRadius(radius);

  m_settings->setBestPixelValue(settings->value(BEST_VALUE, 0).toInt());
  m_categorySelector->setCurrentIndex(settings->value(CATEGORY, 0).toInt());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(UPPER_THRESHOLD, m_seedThreshold->upperThreshold());
  settings->setValue(LOWER_THRESHOLD, m_seedThreshold->lowerThreshold());
  settings->setValue(XSIZE, m_settings->xSize());
  settings->setValue(YSIZE, m_settings->ySize());
  settings->setValue(ZSIZE, m_settings->zSize());
  settings->setValue(APPLY_ROI, m_settings->applyCategoryROI());
  settings->setValue(APPLY_CLOSE, m_settings->applyClose());
  settings->setValue(CLOSE_RADIUS, m_settings->closeRadius());
  settings->setValue(BEST_VALUE, m_settings->bestPixelValue());
  settings->setValue(CATEGORY, m_categorySelector->currentIndex());
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
void SeedGrowSegmentationTool::useBestPixelSelector(bool value)
{
  m_colorLabel->setVisible(false);
  m_colorSelector->setVisible(value);

  if(value)
  {
    setEventHandler(m_bestPixelSelector);
  }
  else
  {
    setEventHandler(m_pixelSelector);
  }
}

//-----------------------------------------------------------------------------
ChannelAdapterPtr SeedGrowSegmentationTool::inputChannel() const
{
  return getActiveChannel();
}

//-----------------------------------------------------------------------------
SelectorSPtr SeedGrowSegmentationTool::activeSelector() const
{
  return m_activeSelector;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onNewPixelValue(int value)
{
  m_settings->setBestPixelValue(value);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onCloseStateChanged(bool value)
{
  m_settings->setApplyClose(value);
  m_close->setVisible(value);
}
