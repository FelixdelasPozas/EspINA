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
#include <Core/IO/DataFactory/FetchRawData.h>
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
      m_fetchBehaviour = DataFactorySPtr{new FetchRawData()};
    }
    sFilter->setDataFactory(m_fetchBehaviour);

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
  , m_action          {new QAction(QIcon(":/espina/pencil.png"), tr("Manual creation of skeletons."), this)}
  {
    m_factory->registerFilterFactory(FilterFactorySPtr{new SourceFilterFactory()});

    m_action->setCheckable(true);

    connect(m_action, SIGNAL(triggered(bool)),
            this,     SLOT(initTool(bool)), Qt::QueuedConnection);

    connect(m_vm->selection().get(), SIGNAL(selectionChanged()),
            this,                    SLOT(updateState()), Qt::QueuedConnection);

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

    disconnect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
               this,               SLOT(categoryChanged(CategoryAdapterSPtr)));
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonTool::setEnabled(bool value)
  {
    m_enabled = value;

    if (m_widget != nullptr)
      m_widget->setEnabled(value);

    m_action->setEnabled(value);
    m_categorySelector->setEnabled(value);
  }
  
  //-----------------------------------------------------------------------------
  void SkeletonTool::updateState()
  {
    auto selectedSegs = m_vm->selection()->segmentations();
    auto enabled = (selectedSegs.size() <= 1);

    m_action->setEnabled(enabled);
    m_categorySelector->setEnabled(enabled);

    if(enabled && !selectedSegs.empty())
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
      if(nullptr == m_vm->activeChannel()) return;

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

      auto toleranceValue = std::floor(std::max(spacing[0], std::max(spacing[1], spacing[2]))) + 1;

      QColor color;
      auto selection = m_vm->selection()->segmentations();
      if(selection.size() != 1)
        color = m_categorySelector->selectedCategory()->color();
      else
        color = m_vm->colorEngine()->color(selection.first());

      auto widget = new SkeletonWidget();
      widget->setTolerance(toleranceValue);

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
        if(rep != RepresentationSPtr())
          rep->setVisible(false);

        if(hasSkeletonData(m_item->output()))
          widget->initialize(skeletonData(m_item->output())->skeleton());
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

      if(m_item != nullptr)
      {
        auto rep = m_item->representation(SkeletonRepresentation::TYPE);
        if(rep != RepresentationSPtr())
          rep->setVisible(true);
      }

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
  void SkeletonTool::eventHandlerToogled(bool toggled)
  {
    if (!toggled && m_widget != nullptr)
    {
      initTool(false);
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::createSegmentation()
  {
    if(m_item != nullptr)
    {
      if(hasSkeletonData(m_item->output()))
      {
        auto polyData = skeletonData(m_item->output())->skeleton();

        polyData->SetPoints(m_skeleton->GetPoints());
        polyData->SetLines(m_skeleton->GetLines());
        polyData->Modified();
      }
      else
      {
        auto spacing = m_item->output()->spacing();
        auto data    = std::make_shared<RawSkeleton>(m_skeleton, spacing, m_item->output());

        m_item->output()->setData(data);
      }
    }
    else
    {
      auto spacing = m_vm->activeChannel()->output()->spacing();
      auto filter  = m_factory->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);
      auto output  = OutputSPtr(new Output(filter.get(), 0, spacing));

      output->setData(std::make_shared<RawSkeleton>(m_skeleton, spacing, output));

      filter->addOutput(0, output);

      auto category     = m_categorySelector->selectedCategory();
      auto segmentation = m_factory->createSegmentation(filter, 0);
      Q_ASSERT(category);

      segmentation->setCategory(category);

      SampleAdapterSList samples;
      samples << QueryAdapter::sample(m_vm->activeChannel());
      Q_ASSERT(samples.size() == 1);

      m_undoStack->beginMacro(tr("Add Segmentation"));
      m_undoStack->push(new AddSegmentations(segmentation, samples, m_model));
      m_undoStack->endMacro();

      m_item = segmentation.get();
    }

    m_skeleton = nullptr;

    SegmentationAdapterList selection;
    selection << m_item;

    m_vm->selection()->set(selection);
    m_vm->updateSegmentationRepresentations(m_item);
    m_vm->updateViews();
  }
} // namespace EspINA

