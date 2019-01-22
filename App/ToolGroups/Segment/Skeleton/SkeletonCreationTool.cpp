/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <App/Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonCreationTool.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonToolWidget2D.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <Core/Analysis/Output.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/DoubleSpinBoxAction.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Model/Utils/ModelUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/Skeleton/vtkSkeletonWidgetRepresentation.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Undo/AddSegmentations.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/ModifySkeletonCommand.h>
#include <Undo/RemoveSegmentations.h>
#include "SkeletonToolsUtils.h"

// VTK
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkDoubleArray.h>

// Qt
#include <QUndoStack>
#include <QBitmap>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::View::Widgets::Skeleton;
using namespace ESPINA::Support::Representations::Utils;
using namespace ESPINA::SkeletonToolsUtils;

const Filter::Type SkeletonFilterFactory::SKELETON_FILTER = "SkeletonSource";

const QString MODIFY_HUE_SETTINGS_KEY = QString{"Modify same-hue strokes to avoid coincidences"};

//-----------------------------------------------------------------------------
const FilterTypeList SkeletonFilterFactory::providedFilters() const
{
  FilterTypeList filters;

  filters << SKELETON_FILTER;

  return filters;
}

//-----------------------------------------------------------------------------
FilterSPtr SkeletonFilterFactory::createFilter(InputSList          inputs,
                                               const Filter::Type& filter,
                                               SchedulerSPtr       scheduler) const
{
  if (SKELETON_FILTER != filter)
  {
    auto message = QObject::tr("Unknown filter type: %1.").arg(filter);
    auto details = QObject::tr("SkeletonFilterFactory::createFilter() -> ") + message;

    throw EspinaException(message, details);
  }

  auto sFilter = std::make_shared<SourceFilter>(inputs, SKELETON_FILTER, scheduler);
  if (!m_dataFactory)
  {
    m_dataFactory = std::make_shared<RawDataFactory>();
  }
  sFilter->setDataFactory(m_dataFactory);

  return sFilter;
}

//-----------------------------------------------------------------------------
SkeletonCreationTool::SkeletonCreationTool(Support::Context& context)
: ProgressTool("SkeletonTool", ":/espina/tubular.svg", tr("Manual creation of skeletons."), context)
, m_init              {false}
, m_item              {nullptr}
{
  initFilterFactory();
  initEventHandler();
  initRepresentationFactories();

  setCheckable(true);
  setExclusive(true);

  initParametersWidgets();

  connect(m_eventHandler.get(), SIGNAL(eventHandlerInUse(bool)),
          this                , SLOT(initTool(bool)));

  connect(m_eventHandler.get(), SIGNAL(selectedStroke(int)),
          this,                 SLOT(onStrokeChanged(int)), Qt::DirectConnection);

  registerSkeletonDataOperators();
}

//-----------------------------------------------------------------------------
SkeletonCreationTool::~SkeletonCreationTool()
{
  if(m_item != nullptr) initTool(false);
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::initParametersWidgets()
{
  m_categorySelector = new CategorySelector(getModel());
  m_categorySelector->setToolTip(tr("Category of the segmentation to be created."));

  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(onCategoryChanged(CategoryAdapterSPtr)));

  connect(this, SIGNAL(toggled(bool)), m_categorySelector, SLOT(setVisible(bool)));

  addSettingsWidget(m_categorySelector);

  auto label = new QLabel("Points distance:");
  label->setToolTip(tr("Manage distance between points"));

  connect(this, SIGNAL(toggled(bool)), label, SLOT(setVisible(bool)));

  addSettingsWidget(label);

  m_minWidget = new DoubleSpinBoxAction(this);
  m_minWidget->setToolTip(tr("Minimum distance between points."));

  m_minWidget->setLabelText(tr("Minimum"));
  m_minWidget->setSuffix(tr(" nm"));
  m_minWidget->setValue(0.0);

  connect(m_minWidget, SIGNAL(valueChanged(double)),
          this,        SLOT(onMinimumDistanceChanged(double)));

  connect(this, SIGNAL(toggled(bool)), m_minWidget, SLOT(setVisible(bool)));

  addSettingsWidget(m_minWidget->createWidget(nullptr));

  m_maxWidget = new DoubleSpinBoxAction(this);
  m_maxWidget->setToolTip(tr("Maximum distance between points."));

  m_maxWidget->setLabelText(tr("Maximum"));
  m_maxWidget->setSuffix(tr(" nm"));
  m_maxWidget->setValue(0.0);

  connect(m_maxWidget, SIGNAL(valueChanged(double)),
          this,        SLOT(onMaximumDistanceChanged(double)));

  connect(this, SIGNAL(toggled(bool)), m_maxWidget, SLOT(setVisible(bool)));

  addSettingsWidget(m_maxWidget->createWidget(nullptr));

  auto strokeLabel = new QLabel{tr("Stroke type:")};
  strokeLabel->setToolTip(tr("Select stroke type."));

  addSettingsWidget(strokeLabel);

  m_strokeCombo = new QComboBox();
  m_strokeCombo->setEditable(false);
  m_strokeCombo->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);
  m_strokeCombo->setToolTip(tr("Select stroke type."));

  connect(m_strokeCombo, SIGNAL(currentIndexChanged(int)),
          this,          SLOT(onStrokeChanged(int)), Qt::DirectConnection);

  addSettingsWidget(m_strokeCombo);

  m_strokeButton = createToolButton(":/espina/tag.svg", tr("Define the stroke types."));
  connect(m_strokeButton, SIGNAL(pressed()), this, SLOT(onStrokeConfigurationPressed()));

  addSettingsWidget(m_strokeButton);

  m_changeHueButton = createToolButton(":/espina/skeletonColors.svg", tr("Modify coincident color strokes to facilitate visualization during edition."));
  m_changeHueButton->setCheckable(true);
  m_changeHueButton->setChecked(false);
  connect(m_changeHueButton, SIGNAL(clicked(bool)), this, SLOT(onHueModificationsButtonClicked(bool)));

  addSettingsWidget(m_changeHueButton);

  m_nextButton = createToolButton(":/espina/next_tubular.svg", tr("Start a new skeleton."));
  m_nextButton->setCheckable(false);

  connect(m_nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));

  connect(this, SIGNAL(toggled(bool)), m_nextButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_nextButton);
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onResolutionChanged()
{
  auto channel = getActiveChannel();

  if(channel)
  {
    auto minimumValue = m_minWidget->value();
    auto maximumValue = m_maxWidget->value();

    auto spacing = channel->output()->spacing();
    auto minValue = std::min(spacing[0], std::min(spacing[1], spacing[2]));
    if(minValue == 0) minValue = 1;

    if(minimumValue == 0)
    {
      minimumValue = minValue * 5;
      maximumValue = minimumValue * 5;
    }
    else
    {
      // assume maximumValue != 0
      if(minimumValue < minValue)
      {
        minimumValue = minValue;
      }
    }

    m_minWidget->setValue(minimumValue);
    m_minWidget->setSpinBoxMinimum(minValue);
    m_minWidget->setSpinBoxMaximum(maximumValue);
    m_minWidget->setStepping(minValue);

    m_maxWidget->setValue(maximumValue);
    m_maxWidget->setSpinBoxMinimum(minimumValue);
    m_maxWidget->setSpinBoxMaximum(maximumValue*10);
    m_maxWidget->setStepping(minValue);

    m_eventHandler->setMinimumPointDistance(m_minWidget->value());
    m_eventHandler->setMaximumPointDistance(m_maxWidget->value());

    if(m_skeletonWidgets.empty())
    {
      m_init = true;
    }
    else
    {
      for(auto widget: m_skeletonWidgets)
      {
        widget->setSpacing(spacing);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onModelReset()
{
  if(isChecked())
  {
    setChecked(false);
  }

  disconnect(getContext().viewState().coordinateSystem().get(), SIGNAL(resolutionChanged(NmVector3)),
             this,                                              SLOT(onResolutionChanged()));
  disconnect(getModel().get(), SIGNAL(aboutToBeReset()),
             this,             SLOT(onModelReset()));

  m_init = false;
  STROKES.clear();
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::initTool(bool value)
{
  if (value)
  {
    getSelection()->clear();

    connect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
            this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

    if(!m_init)
    {
      onResolutionChanged();

      connect(getContext().viewState().coordinateSystem().get(), SIGNAL(resolutionChanged(NmVector3)),
              this,                                              SLOT(onResolutionChanged()));
      connect(getModel().get(), SIGNAL(aboutToBeReset()),
              this,             SLOT(onModelReset()));
    }

    m_item = getActiveChannel();

    if(!getViewState().hasTemporalRepresentation(m_factory)) getViewState().addTemporalRepresentations(m_factory);
    if(!getViewState().hasTemporalRepresentation(m_pointsFactory)) getViewState().addTemporalRepresentations(m_pointsFactory);

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

    SkeletonWidget2D::initializeData(nullptr);

    updateStrokes();

    for(auto widget: m_skeletonWidgets)
    {
      widget->updateRepresentation();
    }

    onStrokeChanged(m_strokeCombo->currentIndex());
  }
  else
  {
    disconnect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
               this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));

    if(m_init)
    {
      for(auto widget: m_skeletonWidgets)
      {
        disconnect(widget.get(), SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
                   this,         SLOT(onSkeletonModified(vtkSmartPointer<vtkPolyData>)));
      }

      m_skeletonWidgets.clear();
      m_pointWidgets.clear();

      if(getViewState().hasTemporalRepresentation(m_factory)) getViewState().removeTemporalRepresentations(m_factory);
      if(getViewState().hasTemporalRepresentation(m_pointsFactory)) getViewState().removeTemporalRepresentations(m_pointsFactory);

      if(m_item && m_item != getActiveChannel())
      {
        m_item->setBeingModified(false);
        m_item->clearTemporalRepresentation();
        m_item->invalidateRepresentations();
      }
      m_item = nullptr;

      disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
                 this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

      vtkSkeletonWidgetRepresentation::ClearRepresentation();
    }
  }

  getViewState().refresh();
  m_nextButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::initRepresentationFactories()
{

  auto settingsList     = getPoolSettings<SegmentationSkeletonPoolSettings>(getContext());
  auto skeletonSettings = std::dynamic_pointer_cast<SegmentationSkeletonPoolSettings>(settingsList.first());
  auto representation2D = std::make_shared<SkeletonToolWidget2D>(m_eventHandler, skeletonSettings);

  connect(representation2D.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,                   SLOT(onSkeletonWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  m_factory = std::make_shared<TemporalPrototypes>(representation2D, TemporalRepresentation3DSPtr(), tr("%1 - Skeleton Widget 2D").arg(id()));

  settingsList              = getPoolSettings<ConnectionPoolSettings>(getContext());
  auto connectionSettings   = std::dynamic_pointer_cast<ConnectionPoolSettings>(settingsList.first());
  auto pointsRepresentation = std::make_shared<ConnectionPointsTemporalRepresentation2D>(connectionSettings);

  connect(pointsRepresentation.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,                       SLOT(onPointWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  m_pointsFactory = std::make_shared<TemporalPrototypes>(pointsRepresentation, TemporalRepresentation3DSPtr(), tr("%1 - Points representations").arg(id()));
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onSkeletonWidgetCloned(TemporalRepresentation2DSPtr clone)
{
  auto skeletonWidget = std::dynamic_pointer_cast<SkeletonWidget2D>(clone);

  if(skeletonWidget)
  {
    auto category = m_categorySelector->selectedCategory();
    auto &strokes = STROKES[category->classificationName()];
    auto index    = std::min(std::max(0, m_strokeCombo->currentIndex()), strokes.size()-1);
    skeletonWidget->setStroke(strokes.at(index));
    skeletonWidget->setSpacing(getActiveChannel()->output()->spacing());
    skeletonWidget->setRepresentationTextColor(category->color());
    skeletonWidget->setStrokeHueModification(m_changeHueButton->isChecked());
    skeletonWidget->updateRepresentation();

    connect(skeletonWidget.get(), SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
            this,                 SLOT(onSkeletonModified(vtkSmartPointer<vtkPolyData>)), Qt::DirectConnection);

    connect(skeletonWidget.get(), SIGNAL(strokeChanged(const Core::SkeletonStroke)),
            this,                 SLOT(onStrokeChangedByWidget(const Core::SkeletonStroke)), Qt::DirectConnection);

    m_skeletonWidgets << skeletonWidget;
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onPointWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone)
{
  auto pointWidget = std::dynamic_pointer_cast<ConnectionPointsTemporalRepresentation2D>(clone);

  if(pointWidget)
  {
    connect(m_eventHandler.get(), SIGNAL(addConnectionPoint(const NmVector3)),
            pointWidget.get(),    SLOT(onConnectionPointAdded(const NmVector3)));

    connect(m_eventHandler.get(), SIGNAL(removeConnectionPoint(const NmVector3)),
            pointWidget.get(),    SLOT(onConnectionPointRemoved(const NmVector3)));

    connect(m_eventHandler.get(), SIGNAL(clearConnections()),
            pointWidget.get(),    SLOT(clearPoints()));

    m_pointWidgets << pointWidget;
  }
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::initFilterFactory()
{
  getFactory()->registerFilterFactory(std::make_shared<SkeletonFilterFactory>());
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onMinimumDistanceChanged(double value)
{
  m_eventHandler->setMinimumPointDistance(value);
  m_maxWidget->setSpinBoxMinimum(value);
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onMaximumDistanceChanged(double value)
{
  m_eventHandler->setMaximumPointDistance(value);
  m_minWidget->setSpinBoxMaximum(value);
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onSegmentationsRemoved(ViewItemAdapterSList segmentations)
{
  if(!m_init) return;

  for(auto seg: segmentations)
  {
    if(seg.get() == m_item)
    {
      initTool(false);
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onCategoryChanged(CategoryAdapterSPtr category)
{
  if(category)
  {
    if(!STROKES.contains(category->classificationName()))
    {
      STROKES[category->classificationName()] = defaultStrokes(category);
    }

    updateStrokes();

    onStrokeChanged(0);

    for(auto widget: m_skeletonWidgets)
    {
      widget->setRepresentationTextColor(category->color());
    }
  }

  if(m_item && m_item != getActiveChannel())
  {
    onNextButtonPressed();
  }
}

//-----------------------------------------------------------------------------
void SkeletonCreationTool::onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata)
{
  auto widget = dynamic_cast<SkeletonWidget2D *>(sender());
  auto model  = getModel();

  if(widget)
  {
    Q_ASSERT(polydata->GetNumberOfLines() != 0);

    ConnectionList connections;
    auto undoStack = getUndoStack();

    if(m_item != getActiveChannel())
    {
      // modification
      auto segmentation       = model->smartPointer(segmentationPtr(m_item));
      auto classificationName = segmentation->category()->classificationName();

      // insert connections if it's a dendrite or axon
      if(classificationName.startsWith("Dendrite") || classificationName.startsWith("Axon"))
      {
        connections = GUI::Model::Utils::connections(polydata, model);
        for(auto &connection: connections)
        {
          connection.item1 = segmentation;
          Q_ASSERT(connection.item1);
        }
      }

      WaitingCursor cursor;

      undoStack->beginMacro(tr("Modify skeleton '%1' by adding points.").arg(segmentation->data().toString()));
      undoStack->push(new ModifySkeletonCommand(segmentation, widget->getSkeleton(), connections));
      undoStack->endMacro();
    }
    else
    {
      // creation
      auto activeChannel = getActiveChannel();
      InputSList inputList;
      inputList << activeChannel->asInput();

      auto spacing  = activeChannel->output()->spacing();
      auto filter   = getFactory()->createFilter<SourceFilter>(inputList, SkeletonFilterFactory::SKELETON_FILTER);
      auto output   = std::make_shared<Output>(filter.get(), 0, spacing);
      auto skeleton = std::make_shared<RawSkeleton>(polydata, spacing);
      output->setData(skeleton);

      filter->addOutput(0, output);
      auto segmentation = getFactory()->createSegmentation(filter, 0);
      auto category = m_categorySelector->selectedCategory();
      Q_ASSERT(category);

      segmentation->setCategory(category);
      segmentation->setNumber(firstUnusedSegmentationNumber(model));
      segmentation->setTemporalRepresentation(std::make_shared<NullRepresentationPipeline>());

      SampleAdapterSList samples;
      samples << QueryAdapter::sample(activeChannel);
      Q_ASSERT(samples.size() == 1);

      if(category->classificationName().startsWith("Dendrite") || category->classificationName().startsWith("Axon"))
      {
        connections = GUI::Model::Utils::connections(polydata, model);
        for(auto &connection: connections)
        {
          connection.item1 = segmentation;
        }
      }

      {
        WaitingCursor cursor;

        undoStack->beginMacro(tr("Add segmentation '%1'.").arg(segmentation->data().toString()));
        undoStack->push(new AddSegmentations(segmentation, samples, model, connections));
        undoStack->endMacro();
      }

      m_item = segmentation.get();
      m_item->setBeingModified(true);

      SegmentationAdapterList selection;
      selection << segmentation.get();

      getSelection()->set(selection);
    }

    m_nextButton->setEnabled(m_item != getActiveChannel());

    std::for_each(m_pointWidgets.constBegin(), m_pointWidgets.constEnd(), [](ConnectionPointsTemporalRepresentation2DSPtr pointWidget) { pointWidget->clearPoints(); });
  }
  else
  {
    qWarning() << "onSkeletonModified received signal but couldn't identify the sender." << __FILE__ << __LINE__;
  }

  getViewState().refresh();
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onNextButtonPressed()
{
  auto category = m_categorySelector->selectedCategory();
  auto &strokes = STROKES[category->classificationName()];
  auto index    = std::min(std::max(0, m_strokeCombo->currentIndex()), strokes.size()-1);
  auto stroke   = strokes.at(index);

  if(!m_skeletonWidgets.isEmpty())
  {
    m_skeletonWidgets.first()->stop();

    // force the processing of the event from the widget that adds the skeleton to the model or updates the existing one.
    QApplication::processEvents();
  }

  SkeletonWidget2D::ClearRepresentation();

  for(auto widget: m_skeletonWidgets)
  {
    widget->setStroke(stroke);
    widget->setRepresentationTextColor(category->color());
    widget->updateRepresentation();
  }

  for(auto widget: m_pointWidgets)
  {
    widget->clearPoints();
  }

  if(m_item && m_item != getActiveChannel())
  {
    m_item->setBeingModified(false);
    m_item->clearTemporalRepresentation();
    m_item->invalidateRepresentations();

    m_item = getActiveChannel();
    m_nextButton->setEnabled(false);

    getViewState().refresh();
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::initEventHandler()
{
  m_eventHandler = std::make_shared<SkeletonToolsEventHandler>(getContext());
  m_eventHandler->setInterpolation(true);
  m_eventHandler->setCursor(Qt::CrossCursor);

  connect(m_eventHandler.get(), SIGNAL(checkStartNode(const NmVector3 &)),
          this,                 SLOT(onPointCheckRequested(const NmVector3 &)), Qt::DirectConnection);

  setEventHandler(m_eventHandler);
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onStrokeConfigurationPressed()
{
  auto category = m_categorySelector->selectedCategory();
  if(category)
  {
    auto index = m_strokeCombo->currentIndex();
    auto name  = category->classificationName();

    StrokeDefinitionDialog dialog(STROKES[name], category);
    dialog.exec();

    updateStrokes();
    index = std::min(std::max(0,index), STROKES[name].size() - 1);
    m_strokeCombo->setCurrentIndex(index);
    onStrokeChanged(index);
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  // used only to load SkeletonToolsUtils::STROKE values.
  loadStrokes(settings);

  m_changeHueButton->setChecked(settings->value(MODIFY_HUE_SETTINGS_KEY, false).toBool());

  updateStrokes();
  onStrokeChanged(0);
}

//--------------------------------------------------------------------
void SkeletonCreationTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  // used only to save SkeletonToolsUtils::STROKE values.
  if(!STROKES.isEmpty()) saveStrokes(settings);

  settings->setValue(MODIFY_HUE_SETTINGS_KEY, m_changeHueButton->isChecked());
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onStrokeChangedByWidget(const Core::SkeletonStroke stroke)
{
  if(m_item)
  {
    auto name     = m_categorySelector->selectedCategory()->classificationName();
    auto &strokes = STROKES[name];
    auto index    = std::min(std::max(0, strokes.indexOf(stroke)), strokes.size() - 1);

    m_strokeCombo->blockSignals(true);
    m_strokeCombo->setCurrentIndex(index);
    m_strokeCombo->blockSignals(false);
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onSelectionChanged(SegmentationAdapterList segmentations)
{
  if(!isChecked()) return;

  if(segmentations.size() != 1)
  {
    abortOperation();
  }
  else
  {
    auto seg = segmentations.first();
    auto segItem = segmentationPtr(m_item);

    if(!segItem || !seg || seg != segItem)
    {
      abortOperation();
    }
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onStrokeChanged(int index)
{
  auto category = m_categorySelector->selectedCategory();

  if(category)
  {
    auto name     = category->classificationName();
    auto &strokes = STROKES[name];

    index = std::min(std::max(0,index), strokes.size() - 1);

    auto stroke = strokes.at(index);

    for(auto widget: m_skeletonWidgets)
    {
      widget->setStroke(stroke);
      m_eventHandler->setStroke(stroke);
    }

    if(index != m_strokeCombo->currentIndex())
    {
      m_strokeCombo->blockSignals(true);
      m_strokeCombo->setCurrentIndex(index);
      m_strokeCombo->blockSignals(false);
    }
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onHueModificationsButtonClicked(bool value)
{
  for(auto widget: m_skeletonWidgets)
  {
    widget->setStrokeHueModification(value);
  }

  if(m_item) m_item->invalidateRepresentations();
}

//--------------------------------------------------------------------
void SkeletonCreationTool::updateStrokes()
{
  auto currentCategory = m_categorySelector->selectedCategory();

  if(currentCategory)
  {
    auto categoryName = currentCategory->classificationName();

    m_strokeCombo->blockSignals(true);
    m_strokeCombo->clear();

    auto strokes = STROKES[categoryName];

    // can happen if a category has been created
    if(strokes.size() == 0)
    {
      STROKES[categoryName] = defaultStrokes(currentCategory);
    }

    for(int i = 0; i < strokes.size(); ++i)
    {
      auto stroke = strokes.at(i);

      QPixmap original(ICONS.at(stroke.type));
      QPixmap copy(original.size());
      copy.fill(QColor::fromHsv(stroke.colorHue,255,255));
      copy.setMask(original.createMaskFromColor(Qt::transparent));

      m_strokeCombo->insertItem(i, QIcon(copy), stroke.name);
    }

    m_strokeCombo->blockSignals(false);

    m_eventHandler->setStrokesCategory(categoryName);

    if(m_strokeCombo->currentIndex() < 0 || m_strokeCombo->currentIndex() >= strokes.size())
    {
      m_strokeCombo->setCurrentIndex(0);
    }
  }
}

//--------------------------------------------------------------------
void SkeletonCreationTool::onPointCheckRequested(const NmVector3 &point)
{
  if(!m_skeletonWidgets.isEmpty())
  {
    auto skeletonWidget = std::dynamic_pointer_cast<SkeletonToolWidget2D>(m_skeletonWidgets.first());

    if(skeletonWidget)
    {
      m_eventHandler->setIsStartNode(skeletonWidget->isStartNode(point));
    }
  }
}
