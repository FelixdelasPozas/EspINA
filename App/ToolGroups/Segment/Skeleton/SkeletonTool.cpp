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
#include <Core/Analysis/Output.h>
#include <Core/IO/DataFactory/RawDataFactory.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/ModelFactory.h>
//#include <GUI/Representations/SkeletonRepresentation.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/DoubleSpinBoxAction.h>
#include <Filters/SourceFilter.h>
#include <Undo/AddSegmentations.h>
#include <Undo/ModifyDataCommand.h>
#include <Undo/ModifySkeletonCommand.h>
#include <Undo/RemoveSegmentations.h>
#include "SkeletonToolStatusAction.h"

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
      m_fetchBehaviour = DataFactorySPtr{new RawDataFactory()};
    }
    sFilter->setDataFactory(m_fetchBehaviour);

    return sFilter;
  }

  //-----------------------------------------------------------------------------
  SkeletonTool::SkeletonTool(Support::Context &context)
  : m_context         (context)
  , m_categorySelector{new CategorySelector(context.model())}
  , m_toleranceWidget {new DoubleSpinBoxAction(this)}
  , m_toolStatus      {new SkeletonToolStatusAction(this)}
  , m_action          {new QAction(QIcon(":/espina/pencil.png"), tr("Manual creation of skeletons."), this)}
  {
    m_context.factory()->registerFilterFactory(std::make_shared<SourceFilterFactory>());

    m_action->setCheckable(true);

    connect(m_action, SIGNAL(triggered(bool)),
            this,     SLOT(initTool(bool)));

    connect(getSelection(m_context).get(), SIGNAL(selectionChanged()),
            this,                       SLOT(updateState()));

    connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
            this,               SLOT(categoryChanged(CategoryAdapterSPtr)));

    m_categorySelector->setVisible(false);

    m_toleranceWidget->setLabelText(tr("Points Tolerance"));
    m_toleranceWidget->setSuffix(tr(" nm"));
    m_toleranceWidget->setVisible(false);

    connect(m_toleranceWidget, SIGNAL(valueChanged(double)),
            this,              SLOT(toleranceValueChanged(double)));

    m_toolStatus->reset();
    m_toolStatus->setVisible(false);
  }

  //-----------------------------------------------------------------------------
  SkeletonTool::~SkeletonTool()
  {
    if(m_widget)
    {
      m_widget->setEnabled(false);
      m_widget = nullptr;
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::updateState()
  {
    if(!isEnabled()) return;

    auto selection = getSelection(m_context)->segmentations();
    auto validItem = (selection.size() <= 1);

    m_action->setEnabled(validItem);
    m_categorySelector->setEnabled(validItem);
    m_toleranceWidget->setEnabled(validItem);

    NmVector3 spacing;

    if(validItem && !selection.empty())
    {
      spacing = m_item->output()->spacing();
      m_item = selection.first();
      m_itemCategory = m_item->category();
    }
    else
    {
      auto activeChannel = getActiveChannel(m_context);

      if(activeChannel)
      {
        spacing = activeChannel->output()->spacing();
      }
      else
      {
        spacing = NmVector3{1,1,1};
      }

      m_item = nullptr;
      m_itemCategory = m_categorySelector->selectedCategory();

      if(m_widget)
      {
        initTool(false);
      }
      m_toleranceWidget->setSpinBoxMinimum(1);
    }

    m_toleranceWidget->setSpinBoxMinimum(std::max(spacing[0], std::max(spacing[1], spacing[2])));
    m_categorySelector->selectCategory(m_itemCategory);
  }

  //-----------------------------------------------------------------------------
  QList<QAction*> SkeletonTool::actions() const
  {
    QList<QAction *> actions;

    actions << m_action;
    actions << m_categorySelector;
    actions << m_toleranceWidget;
    actions << m_toolStatus;

    return actions;
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::updateReferenceItem()
  {
    auto selectedSegs = getSelection(m_context)->segmentations();

    if (selectedSegs.size() != 1)
    {
      m_item = nullptr;
      m_itemCategory = m_categorySelector->selectedCategory();
      m_toleranceWidget->setSpinBoxMinimum(1);
    }
    else
    {
      m_item = selectedSegs.first();
      m_itemCategory = m_item->category();

      auto spacing = m_item->output()->spacing();
      m_toleranceWidget->setSpinBoxMinimum(std::max(spacing[0], std::max(spacing[1], spacing[2])));
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::updateWidgetRepresentation()
  {
    auto skeleton = dynamic_cast<SkeletonData *>(sender());

    if(!m_widget)
    {
      disconnect(skeleton, SIGNAL(dataChanged()),
              this       , SLOT(updateWidgetRepresentation()));
    }
    else
    {
      auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
      widget->initialize(skeleton->skeleton());
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::initTool(bool value)
  {
    if (value)
    {
      auto activeChannel = getActiveChannel(m_context);

      if(!activeChannel) return;

      updateReferenceItem();
      auto spacing = activeChannel->output()->spacing();
      if(m_item)
      {
        spacing = m_item->output()->spacing();

        if(hasSkeletonData(m_item->output()))
        {
          auto skeleton = skeletonData(m_item->output());
          connect(skeleton.get(), SIGNAL(dataChanged()),
                  this          , SLOT(updateWidgetRepresentation()));
        }
      }

      auto minimumDistance = std::max(spacing[0], std::max(spacing[1], spacing[2]));

      auto selection = getSelection(m_context)->segmentations();
      auto color     = m_categorySelector->selectedCategory()->color();
      if(selection.size() == 1)
      {
        color = m_context.colorEngine()->color(selection.first());
      }

      auto widget = new SkeletonWidget();
      connect(widget, SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
              this  , SLOT(skeletonModification(vtkSmartPointer<vtkPolyData>)));

      connect(widget,       SIGNAL(status(SkeletonWidget::Status)),
              m_toolStatus, SLOT(setStatus(SkeletonWidget::Status)));

      m_toleranceWidget->setSpinBoxMinimum(minimumDistance);
      m_toleranceWidget->setStepping(minimumDistance);
      widget->setTolerance(minimumDistance);

      m_widget.reset(widget);

      m_handler = std::dynamic_pointer_cast<EventHandler>(m_widget);
      m_handler->setCursor(Qt::CrossCursor);

      connect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
              this,            SLOT(eventHandlerToogled(bool)));

      m_context.viewState().setEventHandler(m_handler);
      //TODO m_vm->setSelectionEnabled(false);
      // TODO URGENT m_vm->addWidget(m_widget);
      widget->setSpacing(spacing);
      widget->setRepresentationColor(color);

      if(m_item)
      {
        if(hasSkeletonData(m_item->output()))
        {
          Q_ASSERT(false);//TODO: disable item skeleton representation.
//           auto rep = m_item->representation(SkeletonRepresentation::TYPE);
//           if(rep)
//           {
//             rep->setVisible(false);
//           }

          widget->initialize(skeletonData(m_item->output())->skeleton());
        }
      }

      m_widget->setEnabled(true);

      connect(m_context.model().get(), SIGNAL(segmentationsRemoved(SegmentationAdapterSList)),
              this,                    SLOT(checkItemRemoval(SegmentationAdapterSList)));

    }
    else
    {
      if(!m_widget) return; // can be called twice on undo/redo action combinations.

      m_action->blockSignals(true);
      m_action->setChecked(false);
      m_action->blockSignals(false);

      disconnect(m_context.model().get(), SIGNAL(segmentationsRemoved(SegmentationAdapterSList)),
                 this,                    SLOT(checkItemRemoval(SegmentationAdapterSList)));

      disconnect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
                 this,            SLOT(eventHandlerToogled(bool)));

      m_widget->setEnabled(false);
      //TODO URGENT m_vm->removeWidget(m_widget);

      auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
      Q_ASSERT(widget);
      disconnect(widget, SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
                 this  , SLOT(skeletonModification(vtkSmartPointer<vtkPolyData>)));

      disconnect(widget,       SIGNAL(status(SkeletonWidget::Status)),
                 m_toolStatus, SLOT(setStatus(SkeletonWidget::Status)));
      m_toolStatus->reset();

      m_context.viewState().unsetEventHandler(m_handler);
      m_handler.reset();
      //TODO m_vm->setSelectionEnabled(true);
      m_widget.reset();

      if(m_item)
      {
        if(hasSkeletonData(m_item->output()))
        {
          Q_ASSERT(false);
//           auto rep = m_item->representation(SkeletonRepresentation::TYPE);
//           if(rep)
//           {
//             rep->setVisible(true);
//           }

          auto skeleton = skeletonData(m_item->output());
          disconnect(skeleton.get(), SIGNAL(dataChanged()),
                     this          , SLOT(updateWidgetRepresentation()));

          SegmentationAdapterList selection;
          selection << m_item;

          m_item->invalidateRepresentations();
          getSelection(m_context)->set(selection);
        }
      }
    }

    setControlsVisibility(value);
  }


  //-----------------------------------------------------------------------------
  void SkeletonTool::onToolEnabled(bool enabled)
  {
    if (m_widget)
    {
      m_widget->setEnabled(enabled);
    }

    m_action->setEnabled(enabled);
    m_categorySelector->setEnabled(enabled);
    m_toleranceWidget->setEnabled(enabled);
    m_toolStatus->setEnabled(enabled);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::setControlsVisibility(bool value)
  {
    m_categorySelector->setVisible(value);
    m_toleranceWidget->setVisible(value);
    m_toolStatus->setVisible(value);
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::toleranceValueChanged(double value)
  {
    if(m_widget)
    {
      auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
      widget->setTolerance(value);
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::checkItemRemoval(SegmentationAdapterSList segmentations)
  {
    if(!m_item) return;

    for(auto seg: segmentations)
    {
      if(seg.get() == m_item)
      {
        // need to activate the representation because the removal can be undoed.
        Q_ASSERT(false);//TODO
//         auto rep = m_item->representation(SkeletonRepresentation::TYPE);
//         if(rep)
//         {
//           rep->setVisible(true);
//         }

        auto skeleton = skeletonData(m_item->output());
        disconnect(skeleton.get(), SIGNAL(dataChanged()),
                   this          , SLOT(updateWidgetRepresentation()));

        m_item = nullptr;
        initTool(false);
        return;
      }
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::categoryChanged(CategoryAdapterSPtr category)
  {
    if(m_widget)
    {
      dynamic_cast<SkeletonWidget *>(m_widget.get())->setRepresentationColor(category->color());
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::eventHandlerToogled(bool toggled)
  {
    if (!toggled && m_widget)
    {
      initTool(false);
    }
  }

  //-----------------------------------------------------------------------------
  void SkeletonTool::skeletonModification(vtkSmartPointer<vtkPolyData> polyData)
  {
    auto m_undoStack = m_context.undoStack();
    auto model     = m_context.model();

    if(m_item)
    {
      if(hasSkeletonData(m_item->output()))
      {

        if(polyData->GetNumberOfLines() == 0)
        {
          if(m_item->output()->numberOfDatas() == 1)
          {
            m_undoStack->beginMacro(tr("Remove Segmentation"));
            m_undoStack->push(new RemoveSegmentations(m_item, model));
            m_undoStack->endMacro();

            m_item = nullptr;
            initTool(false);
          }
          else
          {
            m_undoStack->beginMacro(tr("Remove Skeleton from segmentation"));
            m_undoStack->push(new RemoveDataCommand(m_item->output(), SkeletonData::TYPE));
            m_undoStack->endMacro();
          }
        }
        else
        {
          auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());

          m_undoStack->beginMacro(tr("Modify segmentation's skeleton"));
          m_undoStack->push(new ModifySkeletonCommand(skeletonData(m_item->output()), widget->getSkeleton()));
          m_undoStack->endMacro();
        }
      }
      else
      {
        if(polyData->GetNumberOfLines() == 0) return;

        auto widget    = dynamic_cast<SkeletonWidget *>(m_widget.get());
        auto itemOuput = m_item->output();
        auto data      = std::make_shared<RawSkeleton>(widget->getSkeleton(), itemOuput->spacing());

        m_undoStack->beginMacro(tr("Add Skeleton to segmentation"));
        m_undoStack->push(new AddDataCommand(itemOuput, data));
        m_undoStack->endMacro();
      }
    }
    else
    {
      if(polyData->GetNumberOfLines() == 0) return;

      auto activeChannel = getActiveChannel(m_context);

      auto spacing  = activeChannel->output()->spacing();
      auto filter   = m_context.factory()->createFilter<SourceFilter>(InputSList(), SOURCE_FILTER);
      auto output   = std::make_shared<Output>(filter.get(), 0, spacing);
      auto skeleton = std::make_shared<RawSkeleton>(polyData, spacing);
      output->setData(skeleton);

      filter->addOutput(0, output);
      auto segmentation = m_context.factory()->createSegmentation(filter, 0);
      auto category = m_categorySelector->selectedCategory();
      Q_ASSERT(category);

      segmentation->setCategory(category);

      SampleAdapterSList samples;
      samples << QueryAdapter::sample(activeChannel);
      Q_ASSERT(samples.size() == 1);

      m_undoStack->beginMacro(tr("Add Segmentation"));
      m_undoStack->push(new AddSegmentations(segmentation, samples, model));
      m_undoStack->endMacro();

      m_item = segmentation.get();

      connect(skeleton.get(), SIGNAL(dataChanged()),
              this          , SLOT(updateWidgetRepresentation()));

      //TODO:
      {
      Q_ASSERT(false);

//       auto rep = m_item->representation(SkeletonRepresentation::TYPE);
//       if(rep)
//       {
//         rep->setVisible(false);
//       }
      }
    }

    if(m_item)
    {
      SegmentationAdapterList selection;
      selection << m_item;

      getSelection(m_context)->set(selection);
      m_item->invalidateRepresentations();
    }

    //TODO m_vm->updateViews();
  }

} // namespace ESPINA

