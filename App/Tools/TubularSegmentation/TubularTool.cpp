/*
 * SpineTool.cpp
 *
 *  Created on: Oct 24, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include <GUI/Pickers/PixelPicker.h>
#include <GUI/ViewManager.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Segmentation.h>
#include <Undo/AddSegmentation.h>
#include <GUI/vtkWidgets/TubularWidget.h>
#include <App/FilterInspectors/TubularSegmentation/NodesInformationDialog.h>
#include <App/FilterInspectors/TubularSegmentation/TubularFilterInspector.h>
#include <Core/EspinaTypes.h>
#include "TubularTool.h"

// Qt
#include <QEvent>
#include <QUndoStack>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  TubularTool::UpdateSegmentationNodes::UpdateSegmentationNodes(TubularSegmentationFilter::Pointer filter,
                                                                TubularSegmentationFilter::NodeList nodes)
  : m_filter(filter)
  , m_nodes(nodes)
  , m_prevNodes(filter->nodes())
  {
  }

  //-----------------------------------------------------------------------------
  void TubularTool::UpdateSegmentationNodes::redo()
  {
    m_filter->setNodes(m_nodes);
  }

//-----------------------------------------------------------------------------
  void TubularTool::UpdateSegmentationNodes::undo()
  {
    m_filter->setNodes(m_prevNodes);
  }

  //-----------------------------------------------------------------------------
  TubularTool::TubularTool(ViewManager *vm, QUndoStack *undo, EspinaModel *model)
  : m_channel(NULL)
  , m_seg(NULL)
  , m_model(model)
  , m_enabled(true)
  , m_inUse(false)
  , m_roundExtremes(false)
  , m_toolPicker(new PixelPicker())
  , m_widget(NULL)
  , m_viewManager(vm)
  , m_undoStack(undo)
  , m_source(NULL)
  {
    m_toolPicker->setPickable(IPicker::CHANNEL);
    m_toolPicker->setMultiSelection(false);
    m_toolPicker->setCursor(Qt::CrossCursor);

    connect(m_toolPicker.data(), SIGNAL(itemsPicked(IPicker::PickList)), this, SLOT(pixelSelected(IPicker::PickList)));
  }

  //-----------------------------------------------------------------------------
  TubularTool::~TubularTool()
  {
  }

  //-----------------------------------------------------------------------------
  QCursor TubularTool::cursor() const
  {
    return QCursor();
  }

  //-----------------------------------------------------------------------------
  bool TubularTool::enabled() const
  {
    return m_enabled;
  }

  //-----------------------------------------------------------------------------
  bool TubularTool::filterEvent(QEvent *e, EspinaRenderView *view)
  {
    // BUG Crash when no channel is picked on first click
    if (!m_inUse || !m_enabled)
      return false;

    Q_ASSERT(m_widget && m_toolPicker);

    if (m_source.isNull())
      if (!m_toolPicker->filterEvent(e, view))
        return false;

    return m_widget->filterEvent(e, view);
  }

  //-----------------------------------------------------------------------------
  void TubularTool::setInUse(bool value)
  {
    if (m_inUse == value)
      return;

    m_inUse = value;

    if (m_inUse)
    {
      if (!m_widget)
      {
        m_widget = new TubularWidget();
        m_widget->setRoundExtremes(m_roundExtremes);
        connect(m_widget, SIGNAL(nodesUpdated(TubularSegmentationFilter::NodeList)),
                this, SLOT(updateNodes(TubularSegmentationFilter::NodeList)));
      }
      m_viewManager->setSelectionEnabled(false);
      m_viewManager->addWidget(m_widget);
    }
    else
    {
      m_viewManager->removeWidget(m_widget);
      disconnect(m_widget, SIGNAL(nodesUpdated(TubularSegmentationFilter::NodeList)),
                 this, SLOT(updateNodes(TubularSegmentationFilter::NodeList)));
      delete m_widget;
      m_widget = NULL;

      // take care of lazy execution of filter
      if (!m_source.isNull() && m_source->getLazyExecution())
      {
        m_source->executeFilter();
        if (!m_seg)
        {
          FilterSPtr filteR;
          m_seg = m_model->factory()->createSegmentation(m_source, 0);
          m_undoStack->beginMacro("Create Tubular Segmentation");
          m_undoStack->push(new AddSegmentation(m_channel, m_source, m_seg, m_model->findTaxonomyElement(m_viewManager->activeTaxonomy()), m_model));

          SegmentationSList createdSegmentations;
          createdSegmentations << m_seg;
          m_model->emitSegmentationAdded(createdSegmentations);

          m_undoStack->endMacro();
        }
        m_seg->notifyModification();
      }

      m_channel.clear();
      m_seg.clear();
      m_source.clear();
      m_undoStack->clear();

      emit segmentationStopped();
    }
  }

  //-----------------------------------------------------------------------------
  void TubularTool::setEnabled(bool value)
  {
    m_enabled = value;
    m_widget->setEnabled(value);
  }

  //-----------------------------------------------------------------------------
  void TubularTool::updateNodes(TubularSegmentationFilter::NodeList nodes)
  {
    Q_ASSERT(m_source);

    if (nodes == m_source->nodes())
      return;

    SegmentationSList createdSegmentations;
    m_undoStack->beginMacro("Modify Tubular Segmentation Nodes");
    m_undoStack->push(new UpdateSegmentationNodes(m_source, nodes));

    if (!m_source->getLazyExecution())
    {
      if (!m_seg && m_source->outputs().size() == 1)
      {
        m_seg = m_model->factory()->createSegmentation(m_source, 0);
        m_undoStack->push(new AddSegmentation(m_channel, m_source, m_seg, m_model->findTaxonomyElement(m_viewManager->activeTaxonomy()), m_model));
        createdSegmentations << m_seg;
      }
      else
      {
        if (!m_seg.isNull() && m_source->outputs().size() == 1)
        {
          m_seg->notifyModification();
        }
      }
    }
    if (!createdSegmentations.isEmpty())
      m_model->emitSegmentationAdded(createdSegmentations);

    m_undoStack->endMacro();
  }

  //-----------------------------------------------------------------------------
  void TubularTool::Reset()
  {
    if (m_inUse)
    {
      this->setInUse(false);
      this->setEnabled(false);

      m_channel.clear();
      m_seg.clear();
      m_source.clear();
      m_widget = NULL;
    }
  }

  //-----------------------------------------------------------------------------
  void TubularTool::pixelSelected(IPicker::PickList msel)
  {
    if ((msel.size() != 1) || !m_source.isNull())
      return;

    // Only one element selected
    IPicker::PickedItem element = msel.first();
    Q_ASSERT(element.first->GetNumberOfPoints() == 1);
    // with one pixel
    PickableItemPtr input = element.second;

    if (CHANNEL != input->type())
      return;

    m_channel = m_model->findChannel(channelPtr(input));
    Filter::NamedInputs inputs;
    Filter::Arguments args;
    TubularSegmentationFilter::Parameters params(args);

    Nm spacing[3];
    m_channel->volume()->spacing(spacing);
    params.setSpacing(spacing);
    if (m_source.isNull())
    {
      m_source = TubularSegmentationFilter::Pointer(new TubularSegmentationFilter(inputs, args, TubularSegmentationFilter::FILTER_TYPE));

      Filter::FilterInspectorPtr inspector = Filter::FilterInspectorPtr(new TubularFilterInspector(m_source, m_undoStack, m_viewManager, IToolSPtr(NULL)));
      m_source->setFilterInspector(Filter::FilterInspectorPtr(inspector));
    }
    m_source->setTool(this);
  }

  //-----------------------------------------------------------------------------
  void TubularTool::showSpineInformation()
  {
    if (m_source.isNull())
      return;

    NodesInformationDialog *dialog = new NodesInformationDialog(m_model, m_undoStack, m_viewManager, m_source);

    connect(dialog, SIGNAL(finished(int)), dialog, SLOT(deleteLater()));

    dialog->show();
  }

  //-----------------------------------------------------------------------------
  bool TubularTool::getLazyExecution()
  {
    if (m_source.isNull())
      return false;
    else
      return m_source->getLazyExecution();
  }

  //-----------------------------------------------------------------------------
  void TubularTool::setLazyExecution(bool value)
  {
    // a change in laziness is not allowed if a segmentation has been
    // already created
    if (m_seg.isNull())
      m_source->setLazyExecution(value);
  }

  //-----------------------------------------------------------------------------
  bool TubularTool::getRoundedExtremes()
  {
    if (m_source.isNull())
      return false;

    return m_source->getRoundedExtremes();
  }

  //-----------------------------------------------------------------------------
  void TubularTool::setRoundedExtremes(bool value)
  {
    m_roundExtremes = value;
    if (!m_source.isNull())
      m_source->setRoundedExtremes(m_roundExtremes);

    if (m_widget != NULL)
      m_widget->setRoundExtremes(value);
  }

  //-----------------------------------------------------------------------------
  void TubularTool::setNodes(TubularSegmentationFilter::NodeList nodes)
  {
    if (m_widget)
      m_widget->setNodes(nodes);
  }

  //-----------------------------------------------------------------------------
  void TubularTool::setFilter(TubularSegmentationFilter::Pointer source)
  {
    m_source = source;
    m_roundExtremes = m_source->getRoundedExtremes();
    SegmentationSList segList = m_model->segmentations();
    foreach(SegmentationSPtr seg, segList)
    {
      if (seg->filter() == m_source)
      m_seg = seg;
    }
  }

  //-----------------------------------------------------------------------------
  bool TubularTool::isInUse()
  {
    return m_inUse;
  }
}
