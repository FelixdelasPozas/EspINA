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

#include "frontend/toolbar/editor/Settings.h"
#include "frontend/toolbar/editor/SettingsPanel.h"

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
#include <editor/FillHolesCommand.h>
#include <editor/FillHolesFilter.h>
#include <editor/vtkTube.h>
#include <undo/RemoveSegmentation.h>
#include <gui/EspinaView.h>

#include <vtkSphere.h>

enum BrushType {
  CIRCULAR,
  RECTANGULAR,
  SPHERICAL
};

//----------------------------------------------------------------------------
class EditorToolBar::FreeFormCommand
: public QUndoCommand
{
public:
  explicit FreeFormCommand(Channel * channel,
                           Filter *filter,
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
    model->addRelation(m_channel, m_filter, Channel::LINK);
    m_seg->setTaxonomy(m_taxonomy);
    model->addSegmentation(m_seg);
    model->addRelation(m_filter, m_seg, CREATELINK);
    model->addRelation(m_sample, m_seg, "where");
    model->addRelation(m_channel, m_seg, Channel::LINK);
    m_seg->initializeExtensions();
  }

  virtual void undo()
  {
    QSharedPointer<EspinaModel> model(EspinaCore::instance()->model());

    model->removeRelation(m_channel, m_seg, Channel::LINK);
    model->removeRelation(m_sample, m_seg, "where");
    model->removeRelation(m_filter, m_seg, CREATELINK);
    model->removeSegmentation(m_seg);
    model->removeRelation(m_channel, m_filter, Channel::LINK);
    model->removeFilter(m_filter);
  }

private:
  Sample       *m_sample;
  Channel      *m_channel;
  Filter       *m_filter;
  Segmentation *m_seg;
  TaxonomyNode *m_taxonomy;
};

//----------------------------------------------------------------------------
// FIXME
class EditorToolBar::DrawCommand
: public QUndoCommand
{
public:
  explicit DrawCommand(FreeFormSource *source,
		      PlaneType plane,
		      QVector3D center, int radius)
  : m_source(source)
  , m_plane(plane)
  , m_center(center)
  , m_radius(radius)
  {
  }

  virtual void redo()
  {
//     m_source->draw(m_plane, m_center, m_radius);
  }
  virtual void undo()
  {
//     m_source->draw(m_plane, m_center, m_radius);
  }

private:
  FreeFormSource *m_source;
  PlaneType       m_plane;
  QVector3D       m_center;
  int             m_radius;
};

//----------------------------------------------------------------------------
//FIXME
class EditorToolBar::EraseCommand
: public QUndoCommand
{
public:
  explicit EraseCommand(FreeFormSource *source,
		       PlaneType plane,
		       QVector3D center, int radius)
  : m_source(source)
  , m_plane(plane)
  , m_center(center)
  , m_radius(radius)
  {
  }

  virtual void redo()
  {
//     m_source->draw(m_plane, m_center, m_radius);
  }
  virtual void undo()
  {
//     m_source->draw(m_plane, m_center, m_radius);
  }

private:
  FreeFormSource *m_source;
  PlaneType       m_plane;
  QVector3D       m_center;
  int             m_radius;
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
      model->addRelation(oldConnection.first, newConnection.first, INPUTLINK);
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
      model->removeRelation(oldConnection.first, newConnection.first, INPUTLINK);
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
, m_fill(addAction(tr("Fill Holes in Selected Segmentations")))
, m_settings(new Settings())
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
  factory->registerFilter(FillHolesFilter::TYPE, this);

  factory->registerSettingsPanel(new EditorToolBar::SettingsPanel(m_settings));

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

  m_fill->setIcon(QIcon(":/espina/fillHoles.svg"));
  connect(m_fill, SIGNAL(triggered(bool)),
          this, SLOT(fillHoles()));

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
  if (FillHolesFilter::TYPE == filter)
    return new FillHolesFilter(inputs, args);
  else
    Q_ASSERT(false);

  return NULL;
}

//----------------------------------------------------------------------------
void EditorToolBar::startDrawing(bool draw)
{
  if (draw)
  {
    SegmentationList selSegs = selectedSegmentations();
    if (selSegs.size() == 1)
    {
      m_currentSeg = selSegs.first();
      m_currentSource = m_currentSeg->filter();
//       qDebug() << "Editing" << m_currentSeg->data().toString();
    }
    m_pencilSelector->setRadius(m_settings->brushRadius());
    SelectionManager::instance()->setSelectionHandler(m_pencilSelector);
  }
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
  if (region.size() < 3)
    return;

  SelectableItem *selectedItem = msel.first().second;
  Q_ASSERT(ModelItem::CHANNEL == selectedItem->type());
  Channel *channel = dynamic_cast<Channel *>(selectedItem);
  double spacing[3];
  channel->spacing(spacing);

  double points[5][3];
  double *center = points[0];
  double *right  = points[1];
  double *top    = points[2];
  double *left   = points[3];
  double *bottom = points[4];

  for (int i=0; i<5; i++)
  {
    points[i][0] = int(region[i].x()/spacing[0]+0.5)*spacing[0];
    points[i][1] = int(region[i].y()/spacing[1]+0.5)*spacing[1];
    points[i][2] = int(region[i].z()/spacing[2]+0.5)*spacing[2];
  }

  PlaneType selectedPlane;
  if (center[0] == right[0] && right[0] == top[0])
    selectedPlane = SAGITTAL;
  else if (right[1] == center[1] && right[1] == top[1])
    selectedPlane = CORONAL;
  else if (center[2] == right[2] && right[2] == top[2])
    selectedPlane = AXIAL;

  double baseCenter[3], topCenter[3];
  for (int i=0; i<3; i++)
    baseCenter[i] = topCenter[i] = center[i];

  topCenter[selectedPlane] += 0.5*spacing[selectedPlane];

  int radius = 0;
  if (selectedPlane != SAGITTAL)
    radius = fabs(region[0].x() - region[1].x());
  else
    radius = fabs(region[0].y() - region[2].y());

//   qDebug() << "Regions:" << region[0] << region[1] << region[2];
//   qDebug() << "Puntos:" <<center[0] << right[0] << top[0] << left[0] << bottom[0];
//   qDebug() << "Centro:" << baseCenter[0] << baseCenter[1];
//   qDebug() << "Radio:" << radius;

  vtkSmartPointer<vtkImplicitFunction> brush;

  BrushType brushType = CIRCULAR;
  switch (brushType)
  {
    case CIRCULAR:
    {
      vtkSmartPointer<vtkTube> circularBrush = vtkSmartPointer<vtkTube>::New();
      circularBrush->SetBaseCenter(baseCenter);
      circularBrush->SetBaseRadius(radius);
      circularBrush->SetTopCenter(topCenter);
      circularBrush->SetTopRadius(radius);
      brush = circularBrush;
    }
      break;
    case SPHERICAL:
    {
      vtkSmartPointer<vtkSphere> sphericalBrush = vtkSmartPointer<vtkSphere>::New();
      sphericalBrush->SetCenter(baseCenter);
      sphericalBrush->SetRadius(radius);
      brush = sphericalBrush;
    }
      break;
    default:
      Q_ASSERT(false);
  };

  double bounds[6];
  channel->bounds(bounds);
  bounds[0] = std::max(center[0] - radius, bounds[0]);
  bounds[1] = std::min(center[0] + radius, bounds[1]);
  bounds[2] = std::max(center[1] - radius, bounds[2]);
  bounds[3] = std::min(center[1] + radius, bounds[3]);
  bounds[4] = std::max(center[2] - radius, bounds[4]);
  bounds[5] = std::min(center[2] + radius, bounds[5]);

  if (!m_currentSource && !m_currentSeg)
  {
    Filter::NamedInputs inputs;
    Filter::Arguments args;
    FreeFormSource::Parameters params(args);
    params.setSpacing(spacing);
    m_currentSource = new FreeFormSource(inputs, args);
  }

  QSharedPointer<QUndoStack> undo = EspinaCore::instance()->undoStack();
  if (!m_currentSeg && m_currentSource)
  {
    // Create a new segmentation
    if (m_pencilSelector->state() == PencilSelector::DRAWING)
    {
      m_currentSource->draw(0, brush, bounds);
      m_currentSeg = EspinaFactory::instance()->createSegmentation(m_currentSource, 0);
      TaxonomyNode *tax = SelectionManager::instance()->activeTaxonomy();
      undo->push(new FreeFormCommand(channel, m_currentSource, m_currentSeg, tax));
    }
  } else
  {
    unsigned int output = m_currentSeg->outputNumber();
    if (m_pencilSelector->state() == PencilSelector::DRAWING)
      m_currentSource->draw(output, brush, bounds);
    else if (m_pencilSelector->state() == PencilSelector::ERASING)
      m_currentSource->draw(output, brush, bounds, 0);
    else
      Q_ASSERT(false);
  }

  if (m_currentSeg)
    m_currentSeg->notifyModification(true);


//   if (m_pencilSelector->state() == PencilSelector::DRAWING)
//     m_currentSource->draw(selectedPlane, center, radius);
//     //undo->push(new DrawCommand(m_currentSource, AXIAL, center, radius));
//   else if (m_pencilSelector->state() == PencilSelector::ERASING)
//     m_currentSource->erase(selectedPlane, center, radius);
//     //undo->push(new EraseCommand(m_currentSource, AXIAL, center, radius));
//   else
//     Q_ASSERT(false);
// 
//   if (m_currentSeg)
//     m_currentSeg->notifyModification(true);
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
  SegmentationList input = selectedSegmentations();

  if (input.size() > 1)
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->beginMacro("Combine Segmentations");
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::ADDITION));
    undo->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::substractSegmentations()
{
  SegmentationList input = selectedSegmentations();

  if (input.size() > 1)
  {
    QSharedPointer<QUndoStack> undo(EspinaCore::instance()->undoStack());
    undo->beginMacro("Substract Segmentations");
    undo->push(new ImageLogicCommand(input, ImageLogicFilter::SUBSTRACTION));
    undo->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::erodeSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->erodeRadius();
    CODECommand * command = new CODECommand(input, CODECommand::ERODE, r);
    EspinaCore::instance()->undoStack()->push(command);
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::dilateSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->dilateRadius();
    CODECommand * command = new CODECommand(input, CODECommand::DILATE, r);
    EspinaCore::instance()->undoStack()->push(command);
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::openSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->openRadius();
    CODECommand * command = new CODECommand(input, CODECommand::OPEN, r);
    EspinaCore::instance()->undoStack()->push(command);
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::closeSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->closeRadius();
    CODECommand * command = new CODECommand(input, CODECommand::CLOSE, r);
    EspinaCore::instance()->undoStack()->push(command);
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::fillHoles()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    EspinaCore::instance()->undoStack()->push(new FillHolesCommand(input));
  }
}


//----------------------------------------------------------------------------
SegmentationList EditorToolBar::selectedSegmentations()
{
  SegmentationList selection;

  foreach(SelectableItem *item, SelectionManager::instance()->selection())
  {
    if (ModelItem::SEGMENTATION == item->type())
      selection << dynamic_cast<Segmentation *>(item);
  }

  return selection;
}
