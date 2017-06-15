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
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/DoubleSpinBoxAction.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <Undo/AddSegmentations.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/ModifySkeletonCommand.h>
#include <Undo/RemoveSegmentations.h>

// VTK
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkLine.h>

// Qt
#include <QUndoStack>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

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

    m_init = true;
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

    m_item = getActiveChannel();

    if(!getViewState().hasTemporalRepresentation(m_factory)) getViewState().addTemporalRepresentations(m_factory);

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

    for(auto widget: m_widgets)
    {
      widget->initialize(nullptr);
    }
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
        m_item->clearTemporalRepresentation();
        m_item->invalidateRepresentations();
      }
      m_item = nullptr;

      disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
                 this,             SLOT(onSegmentationsRemoved(ViewItemAdapterSList)));

      getViewState().refresh();
    }
  }
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
    skeletonWidget->setRepresentationColor(m_categorySelector->selectedCategory()->color());
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
  for(auto widget: m_widgets)
  {
    widget->setRepresentationColor(category->color());
  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata)
{
  auto widget = dynamic_cast<SkeletonWidget2D *>(sender());

  if(widget)
  {
    auto undoStack = getUndoStack();
    auto model     = getModel();

    if(m_item != getActiveChannel())
    {
      // modification
      auto segmentation = segmentationPtr(m_item);

      if(polydata->GetNumberOfLines() == 0)
      {
        undoStack->beginMacro(tr("Remove Segmentation"));
        undoStack->push(new RemoveSegmentations(segmentation, model));
        undoStack->endMacro();

        m_item = getActiveChannel();
      }
      else
      {
        undoStack->beginMacro(tr("Modify skeleton"));
        undoStack->push(new ModifySkeletonCommand(segmentation, widget->getSkeleton()));
        undoStack->endMacro();
      }
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

      undoStack->beginMacro(tr("Add Segmentation"));
      undoStack->push(new AddSegmentations(segmentation, samples, model));
      undoStack->endMacro();

      m_item = segmentation.get();

      SegmentationAdapterList selection;
      selection << segmentation.get();

      getSelection()->set(selection);
    }
  }
  else
  {
    qWarning() << "onSkeletonModified received signal but couldn't identify the sender." << __FILE__ << __LINE__;
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
