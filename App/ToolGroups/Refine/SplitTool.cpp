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
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <Support/Settings/EspinaSettings.h>
#include <Support/ContextFactories.h>
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
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::Support::Widgets;
using namespace ESPINA::Support::ContextFactories;

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
SplitTool::SplitTool(Support::Context &context)
: m_context(context)
, m_factory(new TemporalPrototypes(TemporalRepresentation2DSPtr(), TemporalRepresentation3DSPtr()))
, m_toggle {Tool::createAction(":/espina/planar_split.svg",tr("Split segmentation"), this)}
, m_widgets{this}
, m_apply  {Tool::createButton(":/espina/tick.png", tr("Apply current state"))}
, m_widget {nullptr}
, m_handler{new SplitToolEventHandler()}
{
  registerFilterFactory(m_context, std::make_shared<SplitFilterFactory>());

  m_toggle->setCheckable(true);
  connect(m_toggle, SIGNAL(toggled(bool)),
          this,     SLOT(toggleWidgetsVisibility(bool)));

  initSplitWidgets();

  connect(m_handler.get(), SIGNAL(eventHandlerInUse(bool)),
          m_toggle,        SLOT(setChecked(bool)));
}

//------------------------------------------------------------------------
SplitTool::~SplitTool()
{
  delete m_apply;
  delete m_toggle;

//   if(m_widget)
//   {
//     //TODO m_context.viewState().removeWidget(m_widget);
//   }
//
//   if(m_context.viewState().eventHandler() == m_handler)
//   {
//     m_context.viewState().setEventHandler(nullptr);
//   }
}

//------------------------------------------------------------------------
QList<QAction *> SplitTool::actions() const
{
  QList<QAction *> actions;

  actions << m_toggle
          << &m_widgets;

  return actions;
}

//------------------------------------------------------------------------
void SplitTool::abortOperation()
{
  m_toggle->setChecked(false);
}

//------------------------------------------------------------------------
void SplitTool::onToolEnabled(bool enabled)
{
  m_toggle->setEnabled(enabled);
}


//-----------------------------------------------------------------------------
void SplitTool::initSplitWidgets()
{
  m_widgets.addWidget(m_apply);
  m_widgets.setVisible(false);

  connect(m_apply, SIGNAL(clicked(bool)),
          this,    SLOT(applyCurrentState()));
}

//------------------------------------------------------------------------
GUI::View::ViewState &SplitTool::viewState() const
{
  return m_context.viewState();
}

//------------------------------------------------------------------------
void SplitTool::showCuttingPlane()
{
  auto selectedSegs = getSelectedSegmentations(m_context);
  Q_ASSERT(selectedSegs.size() == 1);

  auto segmentation = selectedSegs.first();
  //     widget->setSegmentationBounds(segmentation->bounds());

  viewState().setEventHandler(m_handler);
  viewState().addTemporalRepresentations(m_factory);
}


//------------------------------------------------------------------------
void SplitTool::hideCuttingPlane()
{
  viewState().unsetEventHandler(m_handler);
  viewState().removeTemporalRepresentations(m_factory);
}

//------------------------------------------------------------------------
void SplitTool::toggleWidgetsVisibility(bool visible)
{
  if (visible)
  {
    showCuttingPlane();
  }
  else
  {
    hideCuttingPlane();

    emit splittingStopped();
  }

  m_toggle->blockSignals(true);
  m_toggle->setChecked(visible);
  m_toggle->blockSignals(false);

  m_widgets.setVisible(visible);
}


//------------------------------------------------------------------------
void SplitTool::applyCurrentState()
{
  auto widget = dynamic_cast<PlanarSplitWidget *>(m_widget.get());

  if (widget->planeIsValid())
  {
    auto selectedSeg = getSelection(m_context)->segmentations().first();

    InputSList inputs;
    inputs << selectedSeg->asInput();

    auto filter = m_context.factory()->createFilter<SplitFilter>(inputs, SPLIT_FILTER);

    auto spacing = selectedSeg->output()->spacing();
    auto bounds  = selectedSeg->bounds();

    int extent[6]{vtkMath::Round((bounds[0]+spacing[0]/2)/spacing[0]),
      vtkMath::Round((bounds[1]+spacing[0]/2)/spacing[0]),
      vtkMath::Round((bounds[2]+spacing[1]/2)/spacing[1]),
      vtkMath::Round((bounds[3]+spacing[1]/2)/spacing[1]),
      vtkMath::Round((bounds[4]+spacing[2]/2)/spacing[2]),
      vtkMath::Round((bounds[5]+spacing[2]/2)/spacing[2])};

      auto plane2stencil = vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
      plane2stencil->SetInput(widget->getImplicitPlane(spacing));
      plane2stencil->SetOutputOrigin(0,0,0);
      plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
      plane2stencil->SetOutputWholeExtent(extent);
      plane2stencil->Update();

      vtkSmartPointer<vtkImageStencilData> stencil = plane2stencil->GetOutput();

      filter->setStencil(stencil);

      Data data(filter, m_context.model()->smartPointer(selectedSeg));
      m_executingTasks.insert(filter.get(), data);

      connect(filter.get(), SIGNAL(finished()),
              this,         SLOT(createSegmentations()));

      Task::submit(filter);
  }
  else
  {
    auto title   = tr("Planar Split");
    auto message = tr("Operation has NO effect. "
                      "Defined plane does not split the selected "
                      "segmentation into 2 segmentations.");

    GUI::DefaultDialogs::InformationMessage(title, message);
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
        auto segmentation  = m_context.factory()->createSegmentation(m_executingTasks[filter].adapter, i);
        segmentation->setCategory(category);

        segmentationsList << segmentation;
        segmentations << segmentation.get();
      }

      getSelection(m_context)->set(segmentations);

      auto undoStack = m_context.undoStack();
      undoStack->beginMacro("Split Segmentation");
      undoStack->push(new RemoveSegmentations(m_executingTasks[filter].segmentation.get(), m_context.model()));
      undoStack->push(new AddSegmentations(segmentationsList, sample, m_context.model()));
      undoStack->endMacro();

      toggleWidgetsVisibility(false);
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