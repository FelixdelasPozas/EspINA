/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "SplitTool.h"

#include <Core/IO/DataFactory/MarchingCubesFromFetchedVolumetricData.h>
#include <Filters/SplitFilter.h>
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

namespace ESPINA
{
  const Filter::Type SPLIT_FILTER    = "SplitFilter";
  const Filter::Type SPLIT_FILTER_V4 = "EditorToolBar::SplitFilter";

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
    if (!providedFilters().contains(type)) throw Unknown_Filter_Exception();

    auto filter = std::make_shared<SplitFilter>(inputs, type, scheduler);

    if (!m_dataFactory)
    {
      m_dataFactory = std::make_shared<MarchingCubesFromFetchedVolumetricData>();
    }
    filter->setDataFactory(m_dataFactory);

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
  , m_widget           {nullptr}
  , m_handler          {new SplitToolEventHandler()}
  {
    m_planarSplitAction->setCheckable(true);
    m_planarSplitAction->setChecked(false);
    m_applyButton->setVisible(false);
    m_applyButton->setCheckable(false);

    connect(m_planarSplitAction, SIGNAL(triggered(bool)),
            this,                SLOT(initTool(bool)));
    connect(m_applyButton, SIGNAL(triggered()),
            this,          SLOT(applyCurrentState()));
    connect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
            this,            SLOT(initTool(bool)));

    m_factory->registerFilterFactory(std::make_shared<SplitFilterFactory>());
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
  QList<QAction *> SplitTool::actions() const
  {
    QList<QAction *> actions;

    actions << m_planarSplitAction;
    actions << m_applyButton;

    return actions;
  }

  //------------------------------------------------------------------------
  void SplitTool::abortOperation()
  {
    initTool(false);
  }

  //------------------------------------------------------------------------
  void SplitTool::initTool(bool value)
  {
    if(value)
    {
      if (m_widget) return;

      auto widget = PlanarSplitWidget::New();
      m_widget = EspinaWidgetSPtr{widget};
      m_viewManager->setEventHandler(m_handler);
      m_viewManager->setSelectionEnabled(false);
      m_viewManager->addWidget(m_widget);

      auto selectedSegs = m_viewManager->selection()->segmentations();
      Q_ASSERT(selectedSegs.size() == 1);
      auto segmentation = selectedSegs.first();
      widget->setSegmentationBounds(segmentation->bounds());
      m_widget->setEnabled(true);

      m_viewManager->updateViews();
    }
    else
    {
      if(m_widget == nullptr) return;

      m_widget->setEnabled(false);
      m_viewManager->removeWidget(m_widget);
      m_viewManager->unsetEventHandler(m_handler);
      m_viewManager->setSelectionEnabled(true);
      m_viewManager->updateViews();

      m_widget = nullptr;

      emit splittingStopped();
    }

    m_planarSplitAction->blockSignals(true);
    m_planarSplitAction->setChecked(value);
    m_planarSplitAction->blockSignals(false);
    m_applyButton->setVisible(value);
  }

  //------------------------------------------------------------------------
  void SplitTool::applyCurrentState()
  {
    auto widget      = dynamic_cast<PlanarSplitWidget *>(m_widget.get());
    auto selectedSeg = m_viewManager->selection()->segmentations().first();

    if (widget->planeIsValid())
    {
      InputSList inputs;
      inputs << selectedSeg->asInput();

      auto filter = m_factory->createFilter<SplitFilter>(inputs, SPLIT_FILTER);

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

      filter->setStencil(stencil);

      Data data(filter, m_model->smartPointer(selectedSeg));
      m_executingTasks.insert(filter.get(), data);

      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(createSegmentations()));

      Task::submit(filter);
    }
    else
    {
      QMessageBox warning;
      warning.setWindowModality(Qt::WindowModal);
      warning.setWindowTitle(tr("ESPINA"));
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

    auto filter = dynamic_cast<FilterPtr>(sender());
    Q_ASSERT(m_executingTasks.keys().contains(filter));

    if(!filter->isAborted())
    {
      if (filter->numberOfOutputs() == 2)
      {
        auto sample = QueryAdapter::samples(m_executingTasks.value(filter).segmentation);
        auto category = m_executingTasks.value(filter).segmentation->category();

        SegmentationAdapterList  segmentations;
        SegmentationAdapterSList segmentationsList;

        for(auto i: {0, 1})
        {
          auto segmentation  = m_factory->createSegmentation(m_executingTasks[filter].adapter, i);
          segmentation->setCategory(category);

          segmentationsList << segmentation;
          segmentations << segmentation.get();
        }

        m_viewManager->selection()->set(segmentations);

        m_undoStack->beginMacro("Split Segmentation");
        m_undoStack->push(new RemoveSegmentations(m_executingTasks[filter].segmentation.get(), m_model));
        m_undoStack->push(new AddSegmentations(segmentationsList, sample, m_model));
        m_undoStack->endMacro();

        initTool(false);
      }
      else
      {
        QApplication::restoreOverrideCursor();
        QMessageBox warning;
        warning.setWindowModality(Qt::WindowModal);
        warning.setWindowTitle(tr("ESPINA"));
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
  void SplitTool::onToolEnabled(bool enabled)
  {
    m_planarSplitAction->setEnabled(enabled);
  }


  //-----------------------------------------------------------------------------
  SplitToolEventHandler::SplitToolEventHandler()
  {
    setCursor(Qt::CrossCursor);
  }

  //------------------------------------------------------------------------
  bool SplitToolEventHandler::filterEvent(QEvent *e, RenderView *view)
  {
    // passive handler
    return false;
  }

} // namespace ESPINA
