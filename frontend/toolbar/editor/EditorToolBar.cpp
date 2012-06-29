/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "EditorToolBar.h"

#include <selection/SelectionManager.h>
#include <selection/SelectableItem.h>
#include <selection/PixelSelector.h>

#include <QAction>
#include <EspinaCore.h>
#include <model/Channel.h>
#include <model/EspinaFactory.h>
#include <editor/ImageLogicCommand.h>
#include <editor/PencilSelector.h>
#include <editor/FreeFormSource.h>
#include <editor/ClosingFilter.h>
#include <editor/ErodeFilter.h>
#include <editor/DilateFilter.h>
#include <editor/OpeningFilter.h>
#include <undo/RemoveSegmentation.h>
#include <gui/EspinaView.h>
#include "frontend/toolbar/editor/MorphologicalSettingsPanel.h"

//----------------------------------------------------------------------------
class EditorToolBar::FreeFormCommand
: public QUndoCommand
{
public:
  explicit FreeFormCommand(Channel * channel,
                           FreeFormSource *filter,
                           Segmentation *seg,
                           TaxonomyNode *taxonomy)
  : m_channel (channel)
  , m_filter  (filter)
  , m_seg(seg)
  , m_taxonomy(taxonomy)
  {
    ModelItem::Vector samples = m_channel->relatedItems(ModelItem::IN, Channel::STAINLINK);
    Q_ASSERT(samples.size() > 0);
    m_sample = dynamic_cast<Sample *>(samples.first());
  }

  virtual void redo()
  {
    QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

    model->addFilter(m_filter);
    model->addRelation(m_channel, m_filter, "Channel");
    m_seg->setTaxonomy(m_taxonomy);
    model->addSegmentation(m_seg);
    model->addRelation(m_filter, m_seg, CREATELINK);
    model->addRelation(m_sample, m_seg, "where");
    model->addRelation(m_channel, m_seg, "Channel");
    m_seg->initialize();
  }

  virtual void undo()
  {
    QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

    model->removeRelation(m_channel, m_seg, "Channel");
    model->removeRelation(m_sample, m_seg, "where");
    model->removeRelation(m_filter, m_seg, CREATELINK);
    model->removeSegmentation(m_seg);
    model->removeRelation(m_channel, m_filter, "Channel");
    model->removeFilter(m_filter);
  }

private:
  Sample         *m_sample;
  Channel        *m_channel;
  FreeFormSource *m_filter;
  Segmentation   *m_seg;
  TaxonomyNode   *m_taxonomy;
};

//----------------------------------------------------------------------------
class EditorToolBar::DrawCommand
: public QUndoCommand
{
public:
  explicit DrawCommand(FreeFormSource *source,
		      vtkSliceView::VIEW_PLANE plane,
		      QVector3D center, int radius)
  : m_source(source)
  , m_plane(plane)
  , m_center(center)
  , m_radius(radius)
  {
  }

  virtual void redo()
  {
    m_source->draw(m_plane, m_center, m_radius);
  }
  virtual void undo()
  {
    m_source->erase(m_plane, m_center, m_radius);
  }

private:
  FreeFormSource            *m_source;
  vtkSliceView::VIEW_PLANE m_plane;
  QVector3D                  m_center;
  int                        m_radius;
};

//----------------------------------------------------------------------------
class EditorToolBar::EraseCommand
: public QUndoCommand
{
public:
  explicit EraseCommand(FreeFormSource *source,
		       vtkSliceView::VIEW_PLANE plane,
		       QVector3D center, int radius)
  : m_source(source)
  , m_plane(plane)
  , m_center(center)
  , m_radius(radius)
  {
  }

  virtual void redo()
  {
    m_source->erase(m_plane, m_center, m_radius);
  }
  virtual void undo()
  {
    m_source->draw(m_plane, m_center, m_radius);
  }

private:
  FreeFormSource            *m_source;
  vtkSliceView::VIEW_PLANE m_plane;
  QVector3D                  m_center;
  int                        m_radius;
};



//----------------------------------------------------------------------------
class EditorToolBar::CODECommand :
public QUndoCommand
{
  static const QString INPUTLINK;
public:
  enum Operation
  {
    CLOSE,
    OPEN,
    DILATE,
    ERODE
  };
public:
  explicit CODECommand(QList<Segmentation *> inputs,
                       Operation op,
                       unsigned int radius)
  : m_segmentations(inputs)
  {
    foreach(Segmentation *seg, m_segmentations)
    {
      Filter *filter;
      Filter::NamedInputs inputs;
      Filter::Arguments args;
      MorphologicalEditionFilter::Parameters params(args);
      params.setRadius(radius);
      inputs[INPUTLINK] = seg->filter();
      args[Filter::INPUTS] = INPUTLINK + "_" + QString::number(seg->outputNumber());
      switch (op)
      {
        case CLOSE:
          filter = new ClosingFilter(inputs, args);
          break;
        case OPEN:
          filter = new OpeningFilter(inputs, args);
          break;
        case DILATE:
          filter = new DilateFilter(inputs, args);
          break;
        case ERODE:
          filter = new ErodeFilter(inputs, args);
          break;
      }
      filter->update();
      m_newConnections << Connection(filter, 0);
      m_oldConnections << Connection(seg->filter(), seg->outputNumber());
    }
  }

  virtual void redo()
  {
    QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

    for(unsigned int i=0; i<m_newConnections.size(); i++)
    {
      Segmentation *seg        = m_segmentations[i];
      Connection oldConnection = m_oldConnections[i];
      Connection newConnection = m_newConnections[i];

      model->removeRelation(oldConnection.first, seg, CREATELINK);
      model->addFilter(newConnection.first);
      model->addRelation(oldConnection.first, newConnection.first, "Input");
      model->addRelation(newConnection.first, seg, CREATELINK);
      seg->changeFilter(newConnection.first, newConnection.second);
      seg->notifyModification(true);
    }
  }

  virtual void undo()
  {
    QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

    for(unsigned int i=0; i<m_newConnections.size(); i++)
    {
      Segmentation *seg        = m_segmentations[i];
      Connection oldConnection = m_oldConnections[i];
      Connection newConnection = m_newConnections[i];

      model->removeRelation(newConnection.first, seg, CREATELINK);
      model->removeRelation(oldConnection.first, newConnection.first, "Input");
      model->removeFilter(newConnection.first);
      model->addRelation(oldConnection.first, seg, CREATELINK);
      seg->changeFilter(oldConnection.first, oldConnection.second);
      seg->notifyModification(true);
    }
  }

private:
  typedef QPair<Filter *, unsigned int> Connection;
  QList<Connection> m_oldConnections, m_newConnections;
  QList<Segmentation *> m_segmentations;
};

const QString EditorToolBar::CODECommand::INPUTLINK = "Input";

//----------------------------------------------------------------------------
EditorToolBar::EditorToolBar(QWidget* parent)
: QToolBar(parent)
, m_draw(addAction(tr("Draw segmentations")))
, m_addition(addAction(tr("Combine Selected Segmentations")))
, m_substraction(addAction(tr("Substract Selected Segmentations")))
, m_erode(addAction(tr("Erode Selected Segmentations")))
, m_dilate(addAction(tr("Dilate Selected Segmentations")))
, m_open(addAction(tr("Open Selected Segmentations")))
, m_close(addAction(tr("Close Selected Segmentations")))
, m_pencilSelector(new PencilSelector())
, m_currentSource(NULL)
, m_currentSeg(NULL)
{
  setObjectName("EditorToolBar");
  setWindowTitle("Editor Tool Bar");
  EspinaFactory *factory = EspinaFactory::instance();
  factory->registerFilter(ClosingFilter::TYPE,  this);
  factory->registerFilter(OpeningFilter::TYPE,  this);
  factory->registerFilter(DilateFilter::TYPE,   this);
  factory->registerFilter(ErodeFilter::TYPE,    this);
  factory->registerFilter(FreeFormSource::TYPE, this);
  factory->registerFilter(ImageLogicFilter::TYPE, this);

  factory->registerSettingsPanel(new MorphologicalFiltersPreferences());

  m_draw->setIcon(QIcon(":/espina/pencil.png"));
  m_draw->setCheckable(true);
  connect(m_draw, SIGNAL(triggered(bool)),
	 this, SLOT(startDrawing(bool)));

  m_addition->setIcon(QIcon(":/espina/add.svg"));
  connect(m_addition, SIGNAL(triggered(bool)),
	 this, SLOT(combineSegmentations()));

  m_substraction->setIcon(QIcon(":/espina/remove.svg"));
  connect(m_substraction, SIGNAL(triggered(bool)),
	 this, SLOT(substractSegmentations()));

  m_erode->setIcon(QIcon(":/espina/erode.png"));
  connect(m_erode, SIGNAL(triggered(bool)),
	 this, SLOT(erodeSegmentations()));

  m_dilate->setIcon(QIcon(":/espina/dilate.png"));
  connect(m_dilate, SIGNAL(triggered(bool)),
	 this, SLOT(dilateSegmentations()));

  m_open->setIcon(QIcon(":/espina/open.png"));
  connect(m_open, SIGNAL(triggered(bool)),
	 this, SLOT(openSegmentations()));

  m_close->setIcon(QIcon(":/espina/close.png"));
  connect(m_close, SIGNAL(triggered(bool)),
	 this, SLOT(closeSegmentations()));

  m_pencilSelector->setSelectable(SelectionHandler::EspINA_Channel);
  connect(m_pencilSelector, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
	 this, SLOT(drawSegmentation(SelectionHandler::MultiSelection)));
  connect(m_pencilSelector, SIGNAL(selectionAborted()),
	 this, SLOT(stopDrawing()));
  connect(m_pencilSelector, SIGNAL(stateChanged(PencilSelector::State)),
	 this, SLOT(stateChanged(PencilSelector::State)));
}

//----------------------------------------------------------------------------
Filter* EditorToolBar::createFilter(const QString filter, Filter::NamedInputs inputs, const ModelItem::Arguments args)
{
  if (ClosingFilter::TYPE == filter)
    return new ClosingFilter(inputs, args);
  if (OpeningFilter::TYPE == filter)
    return new OpeningFilter(inputs, args);
  if (DilateFilter::TYPE == filter)
    return new DilateFilter(inputs, args);
  if (ErodeFilter::TYPE == filter)
    return new ErodeFilter(inputs, args);
  if (FreeFormSource::TYPE == filter)
    return new FreeFormSource(inputs, args);
  if (ImageLogicFilter::TYPE == filter)
    return new ImageLogicFilter(inputs, args);
  else
    Q_ASSERT(false);

  return NULL;
}

//----------------------------------------------------------------------------
void EditorToolBar::startDrawing(bool draw)
{
  if (draw)
    SelectionManager::instance()->setSelectionHandler(m_pencilSelector);
  else
  {
    SelectionManager::instance()->setSelectionHandler(NULL);
    m_currentSource = NULL;
    m_currentSeg = NULL;
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::drawSegmentation(SelectionHandler::MultiSelection msel)
{
  if (msel.size() == 0)
    return;

  SelectionHandler::VtkRegion region = msel.first().first;
  if (region.size() != 2)
    return;

  QVector3D center = region[0];
  QVector3D p = region[1];
  int extent[6];
  extent[0] = center.x() - 5*m_pencilSelector->radius();
  extent[1] = center.x() + 5*m_pencilSelector->radius();
  extent[2] = center.y() - 5*m_pencilSelector->radius();
  extent[3] = center.y() + 5*m_pencilSelector->radius();
  extent[4] = center.z() - 5*m_pencilSelector->radius();
  extent[5] = center.z() + 5*m_pencilSelector->radius();

  SelectableItem *selectedItem = msel.first().second;
  Q_ASSERT(ModelItem::CHANNEL == selectedItem->type());
  int channelExtent[6];
  Channel *channel = dynamic_cast<Channel *>(selectedItem);
  channel->extent(channelExtent);
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    extent[min] = std::max(extent[min], channelExtent[min]);
    extent[max] = std::min(extent[max], channelExtent[max]);
  }

  if (!m_currentSource)
  {
    Filter::NamedInputs inputs;
    Filter::Arguments args;
    FreeFormSource::Parameters params(args);
    double spacing[3];
    channel->spacing(spacing);
    params.setSpacing(spacing);
    m_currentSource = new FreeFormSource(inputs, args);
  }

  int radius = abs(center.x() - p.x());

  QSharedPointer<QUndoStack> undo = EspinaCore::instance()->undoStack();
  if (m_pencilSelector->state() == PencilSelector::DRAWING)
    m_currentSource->draw(vtkSliceView::AXIAL, center, radius);
    //undo->push(new DrawCommand(m_currentSource, vtkSliceView::AXIAL, center, radius));
  else if (m_pencilSelector->state() == PencilSelector::ERASING)
    m_currentSource->erase(vtkSliceView::AXIAL, center, radius);
    //undo->push(new EraseCommand(m_currentSource, vtkSliceView::AXIAL, center, radius));
  else
    Q_ASSERT(false);

  if (!m_currentSeg && m_currentSource->numberOutputs() == 1)
  {
    m_currentSeg = EspinaFactory::instance()->createSegmentation(m_currentSource, 0);
    TaxonomyNode *tax = EspinaCore::instance()->activeTaxonomy();
    undo->push(new FreeFormCommand(channel, m_currentSource, m_currentSeg, tax));
  }

  if (m_currentSeg)
    m_currentSeg->notifyModification(true);
}

//----------------------------------------------------------------------------
void EditorToolBar::stopDrawing()
{
  m_draw->blockSignals(true);
  m_draw->setChecked(false);
  m_pencilSelector->changeState(PencilSelector::DRAWING);
  m_draw->blockSignals(false);
}

//----------------------------------------------------------------------------
void EditorToolBar::stateChanged(PencilSelector::State state)
{
  switch (state)
  {
    case PencilSelector::DRAWING:
      m_draw->setIcon(QIcon(":/espina/pencil.png"));
      break;
    case PencilSelector::ERASING:
      m_draw->setIcon(QIcon(":/espina/eraser.png"));
      break;
  };
}

//----------------------------------------------------------------------------
void EditorToolBar::combineSegmentations()
{
  QList<Segmentation *> input = selectedSegmentations();

  if (input.size() >= 2)
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->beginMacro("Combine Segmentations");
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::ADDITION));
    undo->push(new RemoveSegmentation(input));
    undo->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::substractSegmentations()
{
  QList<Segmentation *> input = selectedSegmentations();

  if (input.size() >= 2)
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->beginMacro("Substract Segmentations");
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::SUBSTRACTION));
    undo->push(new RemoveSegmentation(input));
    undo->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::erodeSegmentations()
{
  QList<Segmentation *> input = selectedSegmentations();
  if (input.size() > 0)
  {
    EspinaCore::instance()->undoStack()->push(new CODECommand(input, CODECommand::ERODE, 3));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::dilateSegmentations()
{
  QList<Segmentation *> input = selectedSegmentations();
  if (input.size() > 0)
  {
    EspinaCore::instance()->undoStack()->push(new CODECommand(input, CODECommand::DILATE, 3));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::openSegmentations()
{
  QList<Segmentation *> input = selectedSegmentations();
  if (input.size() > 0)
  {
    EspinaCore::instance()->undoStack()->push(new CODECommand(input, CODECommand::OPEN, 3));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::closeSegmentations()
{
  QList<Segmentation *> input = selectedSegmentations();
  if (input.size() > 0)
  {
    EspinaCore::instance()->undoStack()->push(new CODECommand(input, CODECommand::CLOSE, 3));
  }
}

//----------------------------------------------------------------------------
QList< Segmentation* > EditorToolBar::selectedSegmentations()
{
  QSharedPointer<EspinaModel> model = EspinaCore::instance()->model();

  QList<Segmentation *> selection;
  foreach(Segmentation *seg, model->segmentations())
  {
    if (seg->isSelected())
    {
      selection << seg;
    }
  }

  return selection;
}
