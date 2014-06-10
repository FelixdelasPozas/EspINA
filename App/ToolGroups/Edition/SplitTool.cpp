/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "SplitTool.h"
#include <Core/IO/FetchBehaviour/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/SplitFilter.h>
#include <GUI/Model/FilterAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <Support/Settings/EspinaSettings.h>
#include <Undo/AddSegmentations.h>
#include <Undo/RemoveSegmentations.h>

// VTK
#include <vtkImageStencilData.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>
#include <vtkPlane.h>

// Qt
#include <QAction>
#include <QApplication>
#include <QToolButton>
#include <QMessageBox>
#include <QDebug>

namespace EspINA
{
  const Filter::Type SPLIT_FILTER    = "SplitFilter";
  const Filter::Type SPLIT_FILTER_V4 = "SplitFilterV4"; // TODO: put correct string for split filter v4

  //-----------------------------------------------------------------------------
  FilterTypeList SplitTool::SplitFilterFactory::providedFilters() const
  {
    FilterTypeList filters;

    filters << SPLIT_FILTER;
    filters << SPLIT_FILTER_V4;

    return filters;
  }

  //-----------------------------------------------------------------------------
  FilterSPtr SplitTool::SplitFilterFactory::createFilter(InputSList          inputs,
                                                         const Filter::Type& type,
                                                         SchedulerSPtr       scheduler) const throw (Unknown_Filter_Exception)
  {
    if (type != SPLIT_FILTER && type != SPLIT_FILTER_V4) throw Unknown_Filter_Exception();

    auto filter = FilterSPtr{new SplitFilter(inputs, type, scheduler)};

    if (!m_fetchBehaviour)
    {
      m_fetchBehaviour = FetchBehaviourSPtr{new MarchingCubesFromFetchedVolumetricData()};
    }
    filter->setFetchBehaviour(m_fetchBehaviour);

    return filter;
  }

  //------------------------------------------------------------------------
  SplitTool::SplitTool(ModelAdapterSPtr model,
                       ModelFactorySPtr factory,
                       ViewManagerSPtr  viewManager,
                       QUndoStack      *undoStack)
  : m_planarSplitAction{new QAction(QIcon(":/espina/planar_split.svg"),tr("Split segmentation"), nullptr)}
  , m_applyButton      {new QAction(QIcon(":/espina/tick.png"), tr("Apply current state"), nullptr)}
  , m_model            {model}
  , m_factory          {factory}
  , m_viewManager      {viewManager}
  , m_undoStack        {undoStack}
  , m_enabled          {false}
  , m_widget           {nullptr}
  , m_handler          {SplitToolEventHandlerSPtr{new SplitToolEventHandler()}}
  {
    m_planarSplitAction->setCheckable(true);
    m_planarSplitAction->setChecked(false);
    m_applyButton->setVisible(false);
    m_applyButton->setCheckable(false);
    connect(m_planarSplitAction, SIGNAL(triggered(bool)), this, SLOT(initTool(bool)), Qt::QueuedConnection);
    connect(m_applyButton, SIGNAL(triggered()), this, SLOT(applyCurrentState()), Qt::QueuedConnection);
    m_factory->registerFilterFactory(FilterFactorySPtr(new SplitFilterFactory()));
  }

  //------------------------------------------------------------------------
  SplitTool::~SplitTool()
  {
    delete m_planarSplitAction;

    if(m_widget)
      m_viewManager->removeWidget(m_widget);

    if(m_viewManager->eventHandler() == m_handler)
      m_viewManager->setEventHandler(nullptr);
  }

  //------------------------------------------------------------------------
  void SplitTool::setEnabled(bool value)
  {
    m_enabled = value;
    m_planarSplitAction->setEnabled(value);
  }

  //------------------------------------------------------------------------
  bool SplitTool::enabled() const
  {
    return m_enabled;
  }

  //------------------------------------------------------------------------
  QList<QAction *> SplitTool::actions() const
  {
    QList<QAction *> actions;

    actions << m_planarSplitAction;
    actions << m_applyButton;

    return actions;
  }

  //------------------------------------------------------------------------
  void SplitTool::initTool(bool value)
  {
    if(value)
    {
      auto widget = PlanarSplitWidget::New();
      m_widget = EspinaWidgetSPtr{widget};
      m_viewManager->addWidget(m_widget);
      m_viewManager->setSelectionEnabled(false);
      m_viewManager->setEventHandler(m_handler);
      m_applyButton->setVisible(true);

      auto selectedSegs = m_viewManager->selection()->segmentations();
      Q_ASSERT(selectedSegs.size() == 1);
      auto segmentation = selectedSegs.first();
      widget->setSegmentationBounds(segmentation->bounds());
      m_widget->setEnabled(true);

      m_viewManager->updateViews();
    }
    else
    {
      if(m_widget == nullptr)
        return;

      m_viewManager->setSelectionEnabled(true);
      m_planarSplitAction->setChecked(false);
      m_applyButton->setVisible(false);
      m_widget->setEnabled(false);
      m_viewManager->removeWidget(m_widget);
      m_viewManager->setEventHandler(nullptr);
      m_widget = nullptr;
      m_viewManager->updateViews();

      emit splittingStopped();
    }
  }

  //------------------------------------------------------------------------
  void SplitTool::applyCurrentState()
  {
    auto selectedSeg = m_viewManager->selection()->segmentations().first();
    auto widget = dynamic_cast<PlanarSplitWidget *>(m_widget.get());
    if (widget->planeIsValid())
    {
      InputSList inputs;
      inputs << selectedSeg->asInput();

      auto adapter = m_factory->createFilter<SplitFilter>(inputs, SPLIT_FILTER);
      auto filter = adapter.get();

      auto spacing = selectedSeg->output()->spacing();
      auto bounds = selectedSeg->bounds();
      int extent[6]{vtkMath::Round((bounds[0]+spacing[0]/2)/spacing[0]),
                    vtkMath::Round((bounds[1]+spacing[0]/2)/spacing[0]),
                    vtkMath::Round((bounds[2]+spacing[1]/2)/spacing[1]),
                    vtkMath::Round((bounds[3]+spacing[1]/2)/spacing[1]),
                    vtkMath::Round((bounds[4]+spacing[2]/2)/spacing[2]),
                    vtkMath::Round((bounds[5]+spacing[2]/2)/spacing[2])};

      vtkSmartPointer<vtkImplicitFunctionToImageStencil> plane2stencil = vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
      plane2stencil->SetInput(widget->getImplicitPlane(spacing));
      plane2stencil->SetOutputOrigin(0,0,0);
      plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
      plane2stencil->SetOutputWholeExtent(extent);
      plane2stencil->Update();

      vtkSmartPointer<vtkImageStencilData> stencil = vtkSmartPointer<vtkImageStencilData>::New();
      stencil = plane2stencil->GetOutput();

      filter->get()->setStencil(stencil);

      struct Data data(adapter, m_model->smartPointer(selectedSeg));
      m_executingTasks.insert(adapter.get(), data);

      connect(adapter.get(), SIGNAL(finished()), this, SLOT(createSegmentations()));
      adapter->submit();
    }
    else
    {
      QMessageBox warning;
      warning.setWindowModality(Qt::WindowModal);
      warning.setWindowTitle(tr("EspINA"));
      warning.setIcon(QMessageBox::Warning);
      warning.setText(tr("Operation has NO effect. The defined plane does not split the selected segmentation into 2 segmentations."));
      warning.setStandardButtons(QMessageBox::Yes);
      warning.exec();
      return;
    }
  }

  //------------------------------------------------------------------------
  void SplitTool::createSegmentations()
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto filter = qobject_cast<FilterAdapterPtr>(sender());
    Q_ASSERT(m_executingTasks.keys().contains(filter));

    if(!filter->isAborted())
    {
      if (filter->numberOfOutputs() == 2)
      {
        auto sample = QueryAdapter::samples(m_executingTasks.value(filter).segmentation);
        auto category = m_executingTasks.value(filter).segmentation->category();

        SegmentationAdapterSList segmentationsList;
        SegmentationAdapterList segmentations;

        for(auto i: {0, 1})
        {
          auto segmentation  = m_factory->createSegmentation(m_executingTasks[filter].adapter, i);
          segmentation->setCategory(category);

          segmentationsList << segmentation;
          segmentations << segmentation.get();
        }

        m_undoStack->beginMacro("Split Segmentation");
        m_undoStack->push(new RemoveSegmentations(m_executingTasks[filter].segmentation.get(), m_model));
        m_undoStack->push(new AddSegmentations(segmentationsList, sample, m_model));
        m_undoStack->endMacro();

        initTool(false);
        m_viewManager->updateSegmentationRepresentations(segmentations);
        m_viewManager->selection()->set(segmentations);
        m_viewManager->updateViews();
      }
      else
      {
        QApplication::restoreOverrideCursor();
        QMessageBox warning;
        warning.setWindowModality(Qt::WindowModal);
        warning.setWindowTitle(tr("EspINA"));
        warning.setIcon(QMessageBox::Warning);
        warning.setText(tr("Operation has NO effect. The defined plane does not split the selected segmentation into 2 segmentations."));
        warning.setStandardButtons(QMessageBox::Yes);
        warning.exec();
        return;
      }
    }

    QApplication::restoreOverrideCursor();
    m_executingTasks.remove(filter);
  }

  //------------------------------------------------------------------------
  bool SplitToolEventHandler::filterEvent(QEvent *e, RenderView *view)
  {
    // passive handler
    return false;
  }


} // namespace EspINA
