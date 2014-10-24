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
#include <App/ToolGroups/Skeleton/SkeletonTool.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Output.h>
#include <Core/IO/FetchBehaviour/FetchRawData.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Representations/SkeletonRepresentation.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/SpinBoxAction.h>
#include <Filters/SourceFilter.h>
#include <Undo/AddSegmentations.h>

// VTK
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkLine.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  const Filter::Type SOURCE_FILTER = "SourceFilter";

  //-----------------------------------------------------------------------------
  FilterTypeList SourceFilterFactory::providedFilters() const
  {
    FilterTypeList filters;

    filters << SOURCE_FILTER;

    return filters;
  }

  //-----------------------------------------------------------------------------
  FilterSPtr SourceFilterFactory::createFilter(InputSList         inputs,
                                               const Filter::Type& filter,
                                               SchedulerSPtr       scheduler) const throw(Unknown_Filter_Exception)
  {
    if (SOURCE_FILTER != filter)
    {
      throw Unknown_Filter_Exception();
    }

    auto sFilter = FilterSPtr{new SourceFilter(inputs, SOURCE_FILTER, scheduler)};
    if (!m_fetchBehaviour)
    {
      m_fetchBehaviour = FetchBehaviourSPtr{new FetchRawData()};
    }
    sFilter->setFetchBehaviour(m_fetchBehaviour);

    return sFilter;
  }

  //-----------------------------------------------------------------------------
  SkeletonTool::SkeletonTool(ModelAdapterSPtr model, ModelFactorySPtr factory, ViewManagerSPtr viewManager, QUndoStack *undoStack)
  : m_vm              {viewManager}
  , m_model           {model}
  , m_factory         {factory}
  , m_undoStack       {undoStack}
  , m_enabled         {false}
  , m_categorySelector{new CategorySelector(model)}
  , m_toleranceBox    {new SpinBoxAction()}
  , m_action          {new QAction(QIcon(":/espina/pencil.png"), tr("Manual creation of skeletons."), this)}
  {
    m_factory->registerFilterFactory(FilterFactorySPtr{new SourceFilterFactory()});

    m_action->setCheckable(true);

    m_toleranceBox->setLabelText("Tolerance");
    m_toleranceBox->setSuffix(" nm");
    m_toleranceBox->setToolTip("Minimum distance between points.");
    m_toleranceBox->setSpinBoxMinimum(1);

    connect(m_action, SIGNAL(triggered(bool)),
            this,     SLOT(initTool(bool)), Qt::QueuedConnection);

    connect(m_vm->selection().get(), SIGNAL(selectionChanged()),
            this,                    SLOT(updateState()), Qt::QueuedConnection);

    connect(m_toleranceBox, SIGNAL(valueChanged(int)),
            this,           SLOT(toleranceChanged(int)), Qt::QueuedConnection);

    connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
            this,               SLOT(categoryChanged(CategoryAdapterSPtr)), Qt::QueuedConnection);

    setControlsVisibility(false);
  }
  
  //-----------------------------------------------------------------------------
  SkeletonTool::~SkeletonTool()
  {
    if(m_widget)
    {
      m_widget->setEnabled(false);
      m_widget = nullptr;
    }

    disconnect(m_action, SIGNAL(triggered(bool)),
               this,     SLOT(initTool(bool)));

    disconnect(m_vm->selection().get(), SIGNAL(selectionChanged()),
               this,                    SLOT(updateState()));

    disconnect(m_toleranceBox, SIGNAL(valueChanged(int)),
               this,           SLOT(toleranceChanged(int)));

    disconnect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
               this,               SLOT(categoryChanged(CategoryAdapterSPtr)));
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonTool::setEnabled(bool value)
  {
    m_enabled = value;

    if (m_widget)
      m_widget->setEnabled(value);

    m_action->setEnabled(value);
    m_categorySelector->setEnabled(value);
    m_toleranceBox->setEnabled(value);
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonTool::updateState()
  {
    auto selectedSegs = m_vm->selection()->segmentations();
    auto value = (selectedSegs.size() <= 1);

    m_action->setEnabled(value);
    m_categorySelector->setEnabled(value);
    m_toleranceBox->setEnabled(value);

    if(value && !selectedSegs.empty())
    {
      m_item = selectedSegs.first();
      m_itemCategory = m_item->category();
    }
    else
    {
      m_item = nullptr;
      m_itemCategory = m_categorySelector->selectedCategory();

      if(m_widget != nullptr)
        initTool(false);
    }
    m_categorySelector->selectCategory(m_itemCategory);
  }

  //-----------------------------------------------------------------------------
  QList<QAction*> SkeletonTool::actions() const
  {
    QList<QAction *> actions;
    actions << m_action;
    actions << m_categorySelector;
    actions << m_toleranceBox;

    return actions;
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::updateReferenceItem()
  {
    auto selectedSegs = m_vm->selection()->segmentations();

    if (selectedSegs.size() != 1)
    {
      m_item = nullptr;
      m_itemCategory = m_categorySelector->selectedCategory();
    }
    else
    {
      m_item = selectedSegs.first();
      m_itemCategory = m_item->category();
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::initTool(bool value)
  {
    if (value)
    {
      if(nullptr == m_vm->activeChannel())
        return;

      updateReferenceItem();
      NmVector3 spacing;
      if(m_item == nullptr)
      {
        spacing = m_vm->activeChannel()->output()->spacing();
      }
      else
      {
        spacing = m_item->output()->spacing();
      }

      auto minimumValue = std::ceil(std::max(spacing[0], std::max(spacing[1], spacing[2]))) + 1;
      if(m_toleranceBox->getSpinBoxMinimumValue() < minimumValue)
        m_toleranceBox->setSpinBoxMinimum(minimumValue);
      m_toleranceBox->setSpinBoxMaximum(100*minimumValue);

      QColor color;
      auto selection = m_vm->selection()->segmentations();
      if(selection.size() != 1)
        color = m_categorySelector->selectedCategory()->color();
      else
        color = m_vm->colorEngine()->color(selection.first());
      qDebug() << "color" << color;
      auto widget = new SkeletonWidget();
      widget->setTolerance(m_toleranceBox->value());

      m_widget = EspinaWidgetSPtr{widget};
      m_handler = std::dynamic_pointer_cast<EventHandler>(m_widget);
      connect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
              this,            SLOT(eventHandlerToogled(bool)), Qt::QueuedConnection);

      m_vm->setEventHandler(m_handler);
      m_vm->addWidget(m_widget);
      m_vm->setSelectionEnabled(false);
      widget->setSpacing(spacing);
      widget->setRepresentationColor(color);
      if(m_item != nullptr)
      {
        auto rep = m_item->representation(SkeletonRepresentation::TYPE);
        rep->setVisible(false);
        auto skeleton = skeletonData(m_item->output());
        if(skeleton != nullptr)
          widget->initialize(skeleton->skeleton());
      }
      m_widget->setEnabled(true);
    }
    else
    {
      m_action->blockSignals(true);
      m_action->setChecked(false);
      m_action->blockSignals(false);

      m_skeleton = dynamic_cast<SkeletonWidget *>(m_widget.get())->getSkeleton();

      disconnect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
                 this,            SLOT(eventHandlerToogled(bool)));

      m_widget->setEnabled(false);
      m_vm->removeWidget(m_widget);

      m_vm->unsetEventHandler(m_handler);
      m_handler = nullptr;
      m_vm->setSelectionEnabled(true);
      m_widget = nullptr;

      if(m_skeleton->GetNumberOfPoints() != 0)
        createSegmentation();
      else
        m_skeleton = nullptr;
    }

    setControlsVisibility(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::setControlsVisibility(bool value)
  {
    m_categorySelector->setVisible(value);
    m_toleranceBox->setVisible(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::toleranceChanged(int value)
  {
    if(nullptr == m_widget)
      return;

    auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
    widget->setTolerance(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::categoryChanged(CategoryAdapterSPtr category)
  {
    if(m_widget != nullptr)
    {
      dynamic_cast<SkeletonWidget *>(m_widget.get())->setRepresentationColor(category->color());
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::eventHandlerToogled(bool value)
  {
    if(value)
      return;

    if(m_widget != nullptr)
    {
      m_action->blockSignals(true);
      m_action->setChecked(false);
      m_action->blockSignals(false);

      m_skeleton = dynamic_cast<SkeletonWidget *>(m_widget.get())->getSkeleton();

      disconnect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
                 this,            SLOT(eventHandlerToogled(bool)));

      m_widget->setEnabled(false);
      m_vm->removeWidget(m_widget);
      m_handler = nullptr;
      m_vm->setSelectionEnabled(true);
      m_widget = nullptr;

      if(m_skeleton != nullptr && m_skeleton->GetNumberOfPoints() > 0)
        createSegmentation();
      else
        m_skeleton = nullptr;

      setControlsVisibility(value);
    }
  }


  //-----------------------------------------------------------------------------
  void SkeletonTool::createSegmentation()
  {
    if(m_item != nullptr)
    {
      auto oldSkeleton = skeletonData(m_item->output());
      if(oldSkeleton != nullptr)
      {
        auto polyData = oldSkeleton->skeleton();
        polyData->SetPoints(m_skeleton->GetPoints());
        polyData->SetLines(m_skeleton->GetLines());
        polyData->Modified();
      }
      else
      {
        auto spacing = m_item->output()->spacing();
        auto data = SkeletonDataSPtr{new RawSkeleton{m_skeleton, spacing, m_item->output()}};
        m_item->output()->setData(data);
      }
      m_vm->updateSegmentationRepresentations(m_item);
      m_item->representation(SkeletonRepresentation::TYPE)->setVisible(true);
    }
    else
    {
      auto spacing = m_vm->activeChannel()->output()->spacing();
      auto adapter = m_factory->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);
      auto filter  = adapter->get();
      auto output = OutputSPtr(new Output(filter.get(), 0));
      output->setData(SkeletonDataSPtr{ new RawSkeleton{m_skeleton, spacing, output}});

      filter->addOutput(0, output);
      auto segmentation = m_factory->createSegmentation(adapter, 0);
      auto category = m_categorySelector->selectedCategory();
      Q_ASSERT(category);

      segmentation->setCategory(category);

      SampleAdapterSList samples;
      samples << QueryAdapter::sample(m_vm->activeChannel());
      Q_ASSERT(samples.size() == 1);

      m_undoStack->beginMacro(tr("Add Segmentation"));
      m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));
      m_undoStack->endMacro();

      m_vm->updateSegmentationRepresentations(segmentation.get());
    }

    m_vm->updateViews();
  }
} // namespace EspINA

