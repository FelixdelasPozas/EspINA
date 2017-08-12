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
#include <App/ToolGroups/Segment/Skeleton/SkeletonTool.h>
#include <App/Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <Core/Analysis/Output.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Filters/SourceFilter.h>
#include <Extensions/SkeletonInformation/SkeletonInformation.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/DoubleSpinBoxAction.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/Skeleton/vtkSkeletonWidgetRepresentation.h>
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
using namespace ESPINA::GUI::View::Widgets::Skeleton;
using namespace ESPINA::SkeletonToolsUtils;

const Filter::Type SkeletonFilterFactory::SKELETON_FILTER = "SkeletonSource";

//-----------------------------------------------------------------------------
FilterTypeList SkeletonFilterFactory::providedFilters() const
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
SkeletonTool::SkeletonTool(Support::Context& context)
: ProgressTool      ("SkeletonTool", ":/espina/tubular.svg", tr("Manual creation of skeletons."), context)
, m_init            {false}
, m_item            {nullptr}
{
  initFilterFactory();
  initEventHandler();
  initRepresentationFactory();

  setCheckable(true);
  setExclusive(true);

  connect(m_eventHandler.get(), SIGNAL(eventHandlerInUse(bool)),
          this                , SLOT(initTool(bool)));

  initParametersWidgets();

  registerSkeletonDataOperators();
}

//-----------------------------------------------------------------------------
SkeletonTool::~SkeletonTool()
{
  if(m_item != nullptr) initTool(false);
}

//-----------------------------------------------------------------------------
void SkeletonTool::initParametersWidgets()
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

  connect(m_strokeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onStrokeTypeChanged(int)));

  addSettingsWidget(m_strokeCombo);

  m_strokeButton = createToolButton(":/espina/tag.svg", tr("Define the stroke types."));
  connect(m_strokeButton, SIGNAL(pressed()), this, SLOT(onStrokeConfigurationPressed()));

  addSettingsWidget(m_strokeButton);

  m_nextButton = createToolButton(":/espina/next_tubular.svg", tr("Start a new skeleton."));
  m_nextButton->setCheckable(false);

  connect(m_nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));

  connect(this, SIGNAL(toggled(bool)), m_nextButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_nextButton);
}

//-----------------------------------------------------------------------------
void SkeletonTool::onResolutionChanged()
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

    if(m_widgets.empty())
    {
      m_init = true;
    }
    else
    {
      for(auto widget: m_widgets)
      {
        widget->setSpacing(spacing);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::onModelReset()
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
void SkeletonTool::initTool(bool value)
{
  if (value)
  {
    if(!m_init)
    {
      onResolutionChanged();

      connect(getContext().viewState().coordinateSystem().get(), SIGNAL(resolutionChanged(NmVector3)),
              this,                                              SLOT(onResolutionChanged()));
      connect(getModel().get(), SIGNAL(aboutToBeReset()),
              this,             SLOT(onModelReset()));
    }

    getSelection()->clear();
    m_item = getActiveChannel();

    if(!getViewState().hasTemporalRepresentation(m_factory)) getViewState().addTemporalRepresentations(m_factory);

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

    for(auto widget: m_widgets)
    {
      widget->initialize(nullptr);
    }

    onStrokeTypeChanged(m_strokeCombo->currentIndex());
  }
  else
  {
    if(m_init)
    {
      for(auto widget: m_widgets)
      {
        disconnect(widget.get(), SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
                   this,         SLOT(onSkeletonModified(vtkSmartPointer<vtkPolyData>)));
      }

      m_widgets.clear();

      if(getViewState().hasTemporalRepresentation(m_factory)) getViewState().removeTemporalRepresentations(m_factory);

      if(m_item && m_item != getActiveChannel())
      {
        m_item->setBeingModified(false);
        m_item->clearTemporalRepresentation();
        m_item->invalidateRepresentations();
      }
      m_item = nullptr;

      disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
                 this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

      vtkSkeletonWidgetRepresentation::cleanup();
    }
  }

  getViewState().refresh();
  m_nextButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
void SkeletonTool::initRepresentationFactory()
{
  auto representation2D = std::make_shared<SkeletonWidget2D>(m_eventHandler);

  connect(representation2D.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,                   SLOT(onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  m_factory = std::make_shared<TemporalPrototypes>(representation2D, TemporalRepresentation3DSPtr(), id());
}

//-----------------------------------------------------------------------------
void SkeletonTool::onWidgetCloned(TemporalRepresentation2DSPtr clone)
{
  auto skeletonWidget = std::dynamic_pointer_cast<SkeletonWidget2D>(clone);

  if(skeletonWidget)
  {
    auto stroke = STROKES[m_categorySelector->selectedCategory()->classificationName()].at(m_strokeCombo->currentIndex());
    skeletonWidget->setStroke(stroke);
    skeletonWidget->setSpacing(getActiveChannel()->output()->spacing());

    connect(skeletonWidget.get(), SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
            this,                 SLOT(onSkeletonModified(vtkSmartPointer<vtkPolyData>)), Qt::DirectConnection);

    m_widgets << skeletonWidget;
  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::initFilterFactory()
{
  getFactory()->registerFilterFactory(std::make_shared<SkeletonFilterFactory>());
}

//-----------------------------------------------------------------------------
void SkeletonTool::onMinimumDistanceChanged(double value)
{
  m_eventHandler->setMinimumPointDistance(value);
  m_maxWidget->setSpinBoxMinimum(value);
}

//-----------------------------------------------------------------------------
void SkeletonTool::onMaximumDistanceChanged(double value)
{
  m_eventHandler->setMaximumPointDistance(value);
  m_minWidget->setSpinBoxMaximum(value);
}

//-----------------------------------------------------------------------------
void SkeletonTool::onSegmentationsRemoved(ViewItemAdapterSList segmentations)
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
void SkeletonTool::onCategoryChanged(CategoryAdapterSPtr category)
{
  if(m_item && m_item != getActiveChannel())
  {
    onNextButtonPressed();
  }

  if(category)
  {
    if(!STROKES.contains(category->classificationName()))
    {
      STROKES[category->classificationName()] = defaultStrokes(category);
    }

    updateStrokes();

    onStrokeTypeChanged(0);
  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata)
{
  auto widget = dynamic_cast<SkeletonWidget2D *>(sender());

  if(widget)
  {
    Q_ASSERT(polydata->GetNumberOfLines() != 0);

    auto undoStack = getUndoStack();
    auto model     = getModel();

    ConnectionList connections;

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

      undoStack->beginMacro(tr("Modify skeleton"));
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

      undoStack->beginMacro(tr("Add Segmentation"));
      undoStack->push(new AddSegmentations(segmentation, samples, model, connections));
      undoStack->endMacro();

      m_item = segmentation.get();
      m_item->setBeingModified(true);

      // SkeletonInformation extension has dynamic keys so it needs to be created in advance in case
      // raw information asks for keys later.
      auto informationExtension = getFactory()->createSegmentationExtension(SkeletonInformation::TYPE);
      segmentation->extensions()->add(informationExtension);

      SegmentationAdapterList selection;
      selection << segmentation.get();

      getSelection()->set(selection);
    }

    m_nextButton->setEnabled(m_item != getActiveChannel());
  }
  else
  {
    qWarning() << "onSkeletonModified received signal but couldn't identify the sender." << __FILE__ << __LINE__;
  }

  getViewState().refresh();
}

//--------------------------------------------------------------------
void SkeletonTool::onNextButtonPressed()
{
  if(m_item && m_item != getActiveChannel())
  {
    for(auto widget: m_widgets)
    {
      widget->stop();
      widget->initialize(nullptr);
    }

    m_item->setBeingModified(false);
    m_item->clearTemporalRepresentation();
    m_item->invalidateRepresentations();

    m_item = getActiveChannel();
    m_nextButton->setEnabled(false);

    getViewState().refresh();
  }
}

//--------------------------------------------------------------------
void SkeletonTool::onStrokeTypeChanged(int index)
{
  auto category = m_categorySelector->selectedCategory();

  if(category)
  {
    auto name = category->classificationName();
    index = std::min(std::max(0,index), STROKES[name].size() - 1);

    auto stroke = STROKES[name].at(index);

    for(auto widget: m_widgets)
    {
      widget->setStroke(stroke);
    }
  }
}

//--------------------------------------------------------------------
void SkeletonTool::initEventHandler()
{
  m_eventHandler = std::make_shared<SkeletonEventHandler>();
  m_eventHandler->setInterpolation(true);
  m_eventHandler->setCursor(Qt::CrossCursor);

  setEventHandler(m_eventHandler);
}

//--------------------------------------------------------------------
void SkeletonTool::onStrokeConfigurationPressed()
{
  auto category = m_categorySelector->selectedCategory();
  if(category)
  {
    auto index = m_strokeCombo->currentIndex();
    auto name  = category->classificationName();

    StrokeDefinitionDialog dialog(STROKES[name], category);
    dialog.exec();

    updateStrokes();
    index = std::min(index, STROKES[name].size() - 1);
    m_strokeCombo->setCurrentIndex(index);
    onStrokeTypeChanged(index);
  }
}

//--------------------------------------------------------------------
void SkeletonTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  // used only to load SkeletonToolsUtils::STROKE values.
  loadStrokes(settings);

  updateStrokes();
  onStrokeTypeChanged(0);
}

//--------------------------------------------------------------------
void SkeletonTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  // used only to save SkeletonToolsUtils::STROKE values.
  if(!STROKES.isEmpty()) saveStrokes(settings);
}

//--------------------------------------------------------------------
void SkeletonTool::updateStrokes()
{
  auto currentCategory = m_categorySelector->selectedCategory();

  if(currentCategory)
  {
    m_strokeCombo->blockSignals(true);
    m_strokeCombo->clear();

    auto strokes = STROKES[currentCategory->classificationName()];
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
  }
}
