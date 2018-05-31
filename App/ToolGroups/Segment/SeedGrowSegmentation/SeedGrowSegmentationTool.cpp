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
#include "CustomROIWidget.h"
#include <Core/Utils/EspinaException.h>
#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <ToolGroups/Restrict/RestrictToolGroup.h>
#include <ToolGroups/Restrict/OrthogonalROITool.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/Utils/ModelUtils.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/PixelValueSelector.h>
#include <GUI/Widgets/NumericalInput.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/FilterRefiner.h>
#include <App/Settings/ROI/ROISettings.h>
#include <Support/Settings/Settings.h>
#include <Undo/AddSegmentations.h>

// Qt
#include <QAction>
#include <QUndoStack>
#include <QSettings>
#include <QMessageBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QThread>

#include "GUI/Model/ViewItemAdapter.h"
using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::Support;
using namespace ESPINA::Support::Widgets;

const Filter::Type SeedGrowSegmentationFilterFactory::SGS_FILTER    = "SeedGrowSegmentation";
const Filter::Type SeedGrowSegmentationFilterFactory::SGS_FILTER_V4 = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

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
const QString USE_BEST_PIXEL  = "Use best pixel selector";

//-----------------------------------------------------------------------------
FilterTypeList SeedGrowSegmentationFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SGS_FILTER;
  filters << SGS_FILTER_V4;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SeedGrowSegmentationFilterFactory::createFilter(InputSList          inputs,
                                                           const Filter::Type& filter,
                                                           SchedulerSPtr       scheduler) const
{
  if ((filter != SGS_FILTER) && (filter != SGS_FILTER_V4))
  {
    auto what    = QObject::tr("Unable to create filter: %1").arg(filter);
    auto details = QObject::tr("SeedGrowSegmentationFilterFactory::createFilter() -> Unknown filter: %1").arg(filter);
    throw EspinaException(what, details);
  }

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
                                                   FilterRefinerFactory         &filterRefiners,
                                                   Support::Context             &context)
: ProgressTool("1-GreyLevelSegmentation", ":/espina/grey_level_segmentation.svg", tr("Grey Level Segmentation"), context)
, m_categorySelector{new CategorySelector(context.model())}
, m_seedThreshold   {new SeedThreshold()}
, m_useBestPixel    {Styles::createToolButton(":espina/best_pixel_selector.svg", tr("Apply on best pixel"))}
, m_colorLabel      {new QLabel(tr("Pixel Color:"))}
, m_colorSelector   {new PixelValueSelector()}
, m_roi             {new CustomROIWidget()}
, m_applyClose      {Styles::createToolButton(":espina/morphological_close.svg", tr("Apply close"))}
, m_close           {new NumericalInput()}
, m_settings        {settings}
, m_sgsFactory      {new SeedGrowSegmentationFilterFactory()}
{
  setCheckable(true);
  setExclusive(true);

  getFactory()->registerFilterFactory(m_sgsFactory);

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
  abortTasks();

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
  m_categorySelector->setToolTip(tr("Segmentation category."));

  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(onCategoryChanged(CategoryAdapterSPtr)));

  addSettingsWidget(m_categorySelector);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::initROISelector()
{
  m_roi->setToolTip(tr("Region of interest for the tool."));
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
  m_useBestPixel->setToolTip(tr("Use the nearest pixel with the specified color as seed."));

  connect(m_useBestPixel, SIGNAL(toggled(bool)),
          this,           SLOT(useBestPixelSelector(bool)));

  m_colorLabel->setVisible(enabled);
  m_colorLabel->setToolTip(tr("Seed color."));

  m_colorSelector->setVisible(enabled);
  m_colorSelector->setToolTip(tr("Seed color."));

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
  m_applyClose->setToolTip(tr("Apply a morphological close algorithm after the reconstruction."));

  connect(m_applyClose, SIGNAL(toggled(bool)),
          this,         SLOT(onCloseStateChanged(bool)));

  m_close->setVisible(enabled);
  m_close->setLabelText(tr("Radius"));
  m_close->setSliderVisibility(false);
  m_close->setMinimum(1);
  m_close->setMaximum(10);
  m_close->setToolTip(tr("Close algorithm radius."));

  connect(m_close, SIGNAL(valueChanged(int)),
          this,    SLOT(onRadiusValueChanged(int)));

  addSettingsWidget(m_applyClose);
  addSettingsWidget(m_close);

  connect(m_settings,   SIGNAL(applyCloseChanged(bool)),
          m_applyClose, SLOT(setChecked(bool)));

  connect(m_settings, SIGNAL(closeRadiusChanged(int)),
          m_close,    SLOT(setValue(int)));
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::onRadiusValueChanged(int value)
{
  if(value != m_settings->closeRadius()) m_settings->setCloseRadius(value);
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

  auto currentROI = getContext().roiProvider()->currentROI();

  if (!currentROI && m_roi->applyROI())
  {
    // Create default ROI
    auto xSize = std::max(m_roi->value(Axis::X), (long long) 2);
    auto ySize = std::max(m_roi->value(Axis::Y), (long long) 2);
    auto zSize = std::max(m_roi->value(Axis::Z), (long long) 2);

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
    struct Data data;
    data.Filter = getFactory()->createFilter<SeedGrowSegmentationFilter>(channel, SeedGrowSegmentationFilterFactory::SGS_FILTER);
    data.Category = m_categorySelector->selectedCategory();

    data.Filter->setSeed(seed);
    data.Filter->setUpperThreshold(m_seedThreshold->upperThreshold());
    data.Filter->setLowerThreshold(m_seedThreshold->lowerThreshold());
    data.Filter->setDescription(tr("Grey Level Segmentation"));

    if(m_applyClose->isChecked())
    {
      data.Filter->setClosingRadius(m_close->value());
    }

    if(currentROI)
    {
      data.Filter->setROI(currentROI->clone());
    }

    m_tasks[data.Filter.get()] = data;

    showTaskProgress(data.Filter);

    connect(data.Filter.get(), SIGNAL(finished()),
            this,              SLOT(createSegmentation()));

    Task::submit(data.Filter);

    if (currentROI == getContext().roiProvider()->currentROI())
    {
      getContext().roiProvider()->clear();
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
    auto data  = m_tasks[filter];
    auto model = getModel();

    if (filter->numberOfOutputs() != 1)
    {
      auto message = QObject::tr("SeedGrowSegmentationTool::createSegmentation() -> Invalid number of outputs in filter: %1").arg(filter->numberOfOutputs());
      throw EspinaException(message, message);
    }

    auto segmentation = getFactory()->createSegmentation(data.Filter, 0);
    segmentation->setCategory(data.Category);
    segmentation->setNumber(firstUnusedSegmentationNumber(getModel()));

    SampleAdapterSList samples;
    samples << QueryAdapter::sample(inputChannel());
    Q_ASSERT(samples.size() == 1);

    auto undoStack = getUndoStack();
    undoStack->beginMacro(tr("Add segmentation '%1'.").arg(segmentation->data().toString()));
    undoStack->push(new AddSegmentations(segmentation, samples, model));
    undoStack->endMacro();

    auto sgsFilter = std::dynamic_pointer_cast<SeedGrowSegmentationFilter>(data.Filter);
    if(sgsFilter->isTouchingROI())
    {
      auto message = tr("The segmentation \"%1\" is incomplete because\nis touching the ROI or an edge of the channel.").arg(segmentation->data().toString());
      auto title   = tr("Grey Level Segmentation");

      GUI::DefaultDialogs::InformationMessage(message, title);
    }

    getSelection()->clear();
    getSelection()->set(toViewItemList(segmentation.get()));
  }

  m_tasks.remove(filter);
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

      xSize = settings.value(DEFAULT_ROI_X_SIZE_KEY, 500).toLongLong();
      ySize = settings.value(DEFAULT_ROI_Y_SIZE_KEY, 500).toLongLong();
      zSize = settings.value(DEFAULT_ROI_Z_SIZE_KEY, 500).toLongLong();
    }

    m_roi->setValue(Axis::X, xSize.toLongLong());
    m_roi->setValue(Axis::Y, ySize.toLongLong());
    m_roi->setValue(Axis::Z, zSize.toLongLong());
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_seedThreshold->setUpperThreshold(settings->value(UPPER_THRESHOLD, 30).toInt());
  m_seedThreshold->setLowerThreshold(settings->value(LOWER_THRESHOLD, 30).toInt());

  auto roiX = settings->value(XSIZE,500).toLongLong();
  m_roi->setValue(Axis::X, roiX);
  m_settings->setXSize(roiX);

  auto roiY = settings->value(YSIZE,500).toLongLong();
  m_roi->setValue(Axis::Y, roiY);
  m_settings->setYSize(roiY);

  auto roiZ = settings->value(ZSIZE,500).toLongLong();
  m_roi->setValue(Axis::Z, roiZ);
  m_settings->setZSize(roiZ);

  auto applyROI = settings->value(APPLY_ROI, true).toBool();
  m_roi->setApplyROI(applyROI);
  m_settings->setApplyCategoryROI(applyROI);

  auto applyClose = settings->value(APPLY_CLOSE, false).toBool();
  if(applyClose != m_applyClose->isChecked()) m_applyClose->setChecked(applyClose);

  auto radius = settings->value(CLOSE_RADIUS, 1).toInt();
  m_close->setValue(radius);

  auto value = settings->value(BEST_VALUE, 0).toInt();
  m_settings->setBestPixelValue(value);
  m_colorSelector->setValue(value);

  auto useBest = settings->value(USE_BEST_PIXEL, true).toBool();
  if(useBest != m_useBestPixel->isChecked()) m_useBestPixel->setChecked(useBest);

  m_categorySelector->setCurrentIndex(settings->value(CATEGORY, 0).toInt());
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  settings->setValue(UPPER_THRESHOLD, m_seedThreshold->upperThreshold());
  settings->setValue(LOWER_THRESHOLD, m_seedThreshold->lowerThreshold());
  settings->setValue(XSIZE,           m_settings->xSize());
  settings->setValue(YSIZE,           m_settings->ySize());
  settings->setValue(ZSIZE,           m_settings->zSize());
  settings->setValue(APPLY_ROI,       m_settings->applyCategoryROI());
  settings->setValue(APPLY_CLOSE,     m_settings->applyClose());
  settings->setValue(CLOSE_RADIUS,    m_settings->closeRadius());
  settings->setValue(BEST_VALUE,      m_settings->bestPixelValue());
  settings->setValue(CATEGORY,        m_categorySelector->currentIndex());
  settings->setValue(USE_BEST_PIXEL,  m_useBestPixel->isChecked());
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

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::abortTasks()
{
  for(auto task: m_tasks)
  {
    disconnect(task.Filter.get(), SIGNAL(finished()),
               this,              SLOT(createSegmentation()));

    task.Filter->abort();
    if(!task.Filter->thread()->wait(500))
    {
      task.Filter->thread()->terminate();
    }
  }

  m_tasks.clear();
}
