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
#include <GUI/View/Widgets/Skeleton/SkeletonWidget.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/DoubleSpinBoxAction.h>
#include <GUI/ColorEngines/ColorEngine.h>
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
, m_categorySelector{new CategorySelector(context.model())}
, m_toleranceWidget {new DoubleSpinBoxAction(this)}
, m_eventHandler    {std::make_shared<SkeletonPointTracker>()}
, m_item            {nullptr}
{
  this->getFactory()->registerFilterFactory(std::make_shared<SkeletonFilterFactory>());

  setCheckable(true);
  setExclusive(true);
  setEventHandler(m_eventHandler);

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(initTool(bool)));

  connect(getSelection().get(), SIGNAL(selectionChanged()),
          this,                 SLOT(updateState()));

  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(categoryChanged(CategoryAdapterSPtr)));

  m_toleranceWidget->setLabelText(tr("Points Tolerance"));
  m_toleranceWidget->setSuffix(tr(" nm"));

  connect(m_toleranceWidget, SIGNAL(valueChanged(double)),
          this,              SLOT(toleranceValueChanged(double)));

  addSettingsWidget(m_categorySelector);
  addSettingsWidget(m_toleranceWidget->createWidget(nullptr));
}

//-----------------------------------------------------------------------------
SkeletonTool::~SkeletonTool()
{
  // TODO: 27-05-2015 SkeletonTool/Widget refactorization
//  if(m_widget)
//  {
//    m_widget->setEnabled(false);
//    m_widget = nullptr;
//  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::updateState()
{
  if(!isEnabled()) return;

  auto selection = getSelectedSegmentations();
  auto validItem = (selection.size() <= 1);

  setEnabled(validItem);
  m_categorySelector->setEnabled(validItem);
  m_toleranceWidget->setEnabled(validItem);

  NmVector3 spacing;

  if(validItem && !selection.empty())
  {
    spacing = m_item->output()->spacing();
    m_item = selection.first();
    m_categorySelector->selectCategory(m_item->category());
  }
  else
  {
    m_item = nullptr;

    auto activeChannel = getActiveChannel();

    if(activeChannel)
    {
      spacing = activeChannel->output()->spacing();
    }
    else
    {
      spacing = NmVector3{1,1,1};
    }

    //TODO: 27-05-2015 SkeletonTool/Widget refactorization
//    if(m_widget)
//    {
//      initTool(false);
//    }
  }

  m_toleranceWidget->setSpinBoxMinimum(std::max(spacing[0], std::max(spacing[1], spacing[2])));
}

//-----------------------------------------------------------------------------
void SkeletonTool::updateReferenceItem()
{
  auto selectedSegs = getSelectedSegmentations();

  NmVector3 spacing;
  if (selectedSegs.size() != 1)
  {
    m_item = nullptr;

    auto activeChannel = getActiveChannel();

    if(activeChannel)
    {
      spacing = activeChannel->output()->spacing();
    }
    else
    {
      spacing = NmVector3{1,1,1};
    }
  }
  else
  {
    m_item = selectedSegs.first();
    m_categorySelector->selectCategory(m_item->category());

    spacing = m_item->output()->spacing();
  }

  m_toleranceWidget->setSpinBoxMinimum(std::max(spacing[0], std::max(spacing[1], spacing[2])));
}

//-----------------------------------------------------------------------------
void SkeletonTool::updateWidgetRepresentation()
{
  auto skeleton = dynamic_cast<SkeletonData *>(sender());

  // TODO: 27-05-2015 SkeletonTool/Widget refactorization
//  if(!m_widget)
//  {
//    disconnect(skeleton, SIGNAL(dataChanged()),
//               this       , SLOT(updateWidgetRepresentation()));
//  }
//  else
//  {
//    auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
//    widget->initialize(skeleton->skeleton());
//  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::initTool(bool value)
{
  if (value)
  {
    auto activeChannel = getActiveChannel();

    if(!activeChannel) return;

    updateReferenceItem();
    auto spacing = activeChannel->output()->spacing();
    if(m_item)
    {
      spacing = m_item->output()->spacing();

      if(hasSkeletonData(m_item->output()))
      {
//         auto skeleton = readLockSkeleton(m_item->output());
//         connect(skeleton->get(), SIGNAL(dataChanged()),
//                 this           , SLOT(updateWidgetRepresentation()));
      }
    }

    auto minimumDistance = std::max(spacing[0], std::max(spacing[1], spacing[2]));

    auto selection = getSelectedSegmentations();
    auto color     = m_categorySelector->selectedCategory()->color();
    if(selection.size() == 1)
    {
      color = getContext().colorEngine()->color(selection.first());
    }

    auto widget = new SkeletonWidget(m_eventHandler);
    connect(widget, SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
            this  , SLOT(skeletonModification(vtkSmartPointer<vtkPolyData>)));

    m_toleranceWidget->setSpinBoxMinimum(minimumDistance);
    m_toleranceWidget->setStepping(minimumDistance);
    widget->setTolerance(minimumDistance);

    // TODO: 27-05-2015 SkeletonTool/Widget refactorization
//    m_widget.reset(widget);
//
//    m_handler = std::dynamic_pointer_cast<EventHandler>(m_widget);
//    m_handler->setCursor(Qt::CrossCursor);

    activateEventHandler();
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

        widget->initialize(readLockSkeleton(m_item->output())->skeleton());
      }
    }

//    m_widget->setEnabled(true);

    connect(getModel().get(), SIGNAL(segmentationsRemoved(SegmentationAdapterSList)),
            this,                    SLOT(checkItemRemoval(SegmentationAdapterSList)));

  }
  else
  {
//    if(!m_widget) return; // can be called twice on undo/redo action combinations.

    blockSignals(true);
    setChecked(false);
    blockSignals(false);

    disconnect(getModel().get(), SIGNAL(segmentationsRemoved(SegmentationAdapterSList)),
               this,                    SLOT(checkItemRemoval(SegmentationAdapterSList)));

//    m_widget->setEnabled(false);
    //TODO URGENT m_vm->removeWidget(m_widget);

//    auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
//    Q_ASSERT(widget);
//    disconnect(widget, SIGNAL(modified(vtkSmartPointer<vtkPolyData>)),
//               this  , SLOT(skeletonModification(vtkSmartPointer<vtkPolyData>)));
//
//    disconnect(widget,       SIGNAL(status(SkeletonWidget::Status)),
//               m_toolStatus, SLOT(setStatus(SkeletonWidget::Status)));

    deactivateEventHandler();

    //TODO m_vm->setSelectionEnabled(true);
    //m_widget.reset();

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

//         auto skeleton = readLockSkeleton(m_item->output());
//         disconnect(skeleton.get(), SIGNAL(dataChanged()),
//                    this          , SLOT(updateWidgetRepresentation()));

        SegmentationAdapterList selection;
        selection << m_item;

        m_item->invalidateRepresentations();
        getSelection()->set(selection);
      }
    }
  }

  setControlsVisibility(value);
}


//-----------------------------------------------------------------------------
void SkeletonTool::onToolGroupActivated()
{
  // TODO: 27-05-2015 SkeletonTool/Widget refactorization
//  if (m_widget)
//  {
//    m_widget->setEnabled(enabled);
//  }

  bool enabled = true;
  setEnabled(enabled);
  m_categorySelector->setEnabled(enabled);
  m_toleranceWidget->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void SkeletonTool::setControlsVisibility(bool value)
{
  m_categorySelector->setVisible(value);
  m_toleranceWidget->setVisible(value);
}

//-----------------------------------------------------------------------------
void SkeletonTool::toleranceValueChanged(double value)
{
  // TODO: 27-05-2015 SkeletonTool/Widget refactorization
//  if(m_widget)
//  {
//    auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());
//    widget->setTolerance(value);
//  }
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

//       auto skeleton = readLockSkeleton(m_item->output());
//       disconnect(skeleton.get(), SIGNAL(dataChanged()),
//                  this          , SLOT(updateWidgetRepresentation()));

      m_item = nullptr;
      initTool(false);
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::categoryChanged(CategoryAdapterSPtr category)
{
  // TODO: 27-05-2015 SkeletonTool/Widget refactorization
//  if(m_widget)
//  {
//    dynamic_cast<SkeletonWidget *>(m_widget.get())->setRepresentationColor(category->color());
//  }
}

//-----------------------------------------------------------------------------
void SkeletonTool::skeletonModification(vtkSmartPointer<vtkPolyData> polyData)
{
  auto undoStack = getUndoStack();
  auto model       = getModel();

  if(m_item)
  {
    if(hasSkeletonData(m_item->output()))
    {
      if(polyData->GetNumberOfLines() == 0)
      {
        if(m_item->output()->numberOfDatas() == 1)
        {
          undoStack->beginMacro(tr("Remove Segmentation"));
          undoStack->push(new RemoveSegmentations(m_item, model));
          undoStack->endMacro();

          m_item = nullptr;
          initTool(false);
        }
        else
        {
          undoStack->beginMacro(tr("Remove Skeleton from segmentation"));
          undoStack->push(new RemoveDataCommand(m_item->output(), SkeletonData::TYPE));
          undoStack->endMacro();
        }
      }
      else
      {
//        auto widget = dynamic_cast<SkeletonWidget *>(m_widget.get());

//         m_undoStack->beginMacro(tr("Modify segmentation's skeleton"));
//         m_undoStack->push(new ModifySkeletonCommand(readLockSkeleton(m_item->output()), widget->getSkeleton()));
//         m_undoStack->endMacro();
      }
    }
    else
    {
      if(polyData->GetNumberOfLines() == 0) return;

//      auto widget    = dynamic_cast<SkeletonWidget *>(m_widget.get());
//      auto itemOuput = m_item->output();
//      auto data      = std::make_shared<RawSkeleton>(widget->getSkeleton(), itemOuput->spacing());

//      m_undoStack->beginMacro(tr("Add Skeleton to segmentation"));
//      m_undoStack->push(new AddDataCommand(itemOuput, data));
//      m_undoStack->endMacro();
    }
  }
  else
  {
    if(polyData->GetNumberOfLines() == 0) return;

    auto activeChannel = getActiveChannel();
    InputSList inputList;
    inputList << activeChannel->asInput();

    auto spacing  = activeChannel->output()->spacing();
    auto filter   = getFactory()->createFilter<SourceFilter>(inputList, SkeletonFilterFactory::SKELETON_FILTER);
    auto output   = std::make_shared<Output>(filter.get(), 0, spacing);
    auto skeleton = std::make_shared<RawSkeleton>(polyData, spacing);
    output->setData(skeleton);

    filter->addOutput(0, output);
    auto segmentation = getFactory()->createSegmentation(filter, 0);
    auto category = m_categorySelector->selectedCategory();
    Q_ASSERT(category);

    segmentation->setCategory(category);

    SampleAdapterSList samples;
    samples << QueryAdapter::sample(activeChannel);
    Q_ASSERT(samples.size() == 1);

    undoStack->beginMacro(tr("Add Segmentation"));
    undoStack->push(new AddSegmentations(segmentation, samples, model));
    undoStack->endMacro();

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

    getSelection()->set(selection);
    m_item->invalidateRepresentations();
  }

  //TODO m_vm->updateViews();
}
