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
#include <common/gui/ActionSelector.h>
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
#include <editor/ContourSelector.h>
#include <undo/RemoveSegmentation.h>
#include <gui/EspinaView.h>

#include <vtkSphere.h>

enum BrushType {
  CIRCULAR,
  RECTANGULAR,
  SPHERICAL,
  CONTOUR
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
, m_actionGroup(new ActionSelector(this))
, m_settings(new Settings())
, m_pencilSelector(new PencilSelector())
, m_contourSelector(new ContourSelector())
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

  // draw with a disc
  m_pencilDisc = new QAction(QIcon(":/espina/pencil2D.png"), tr("Drew segmentations using a disc"), m_actionGroup);
  m_actionGroup->addAction(m_pencilDisc);

  // draw with a sphere
  m_pencilSphere = new QAction(QIcon(":espina/pencil3D.png"), tr("Draw segmentations using a sphere"), m_actionGroup);
  m_actionGroup->addAction(m_pencilSphere);

  // draw with contour
  m_contour = new QAction(QIcon(":espina/lasso.png"), tr("Draw segmentations using contours"), m_actionGroup);
  m_actionGroup->addAction(m_contour);

  m_actionGroup->setCheckable(true);
  addAction(m_actionGroup);
  connect(m_actionGroup, SIGNAL(actionCanceled()), this, SLOT(cancelDrawOperation()));
  connect(m_actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(startDrawOperation(QAction*)));
  m_actionGroup->setDefaultAction(m_pencilDisc);

  m_addition = addAction(tr("Combine Selected Segmentations"));
  m_addition->setIcon(QIcon(":/espina/add.svg"));
  connect(m_addition, SIGNAL(triggered(bool)),
          this, SLOT(combineSegmentations()));

  m_substraction = addAction(tr("Subtract Selected Segmentations"));
  m_substraction->setIcon(QIcon(":/espina/remove.svg"));
  connect(m_substraction, SIGNAL(triggered(bool)),
          this, SLOT(substractSegmentations()));

  m_erode = addAction(tr("Erode Selected Segmentations"));
  m_erode->setIcon(QIcon(":/espina/erode.png"));
  connect(m_erode, SIGNAL(triggered(bool)),
          this, SLOT(erodeSegmentations()));

  m_dilate = addAction(tr("Dilate Selected Segmentations"));
  m_dilate->setIcon(QIcon(":/espina/dilate.png"));
  connect(m_dilate, SIGNAL(triggered(bool)),
          this, SLOT(dilateSegmentations()));

  m_open = addAction(tr("Open Selected Segmentations"));
  m_open->setIcon(QIcon(":/espina/open.png"));
  connect(m_open, SIGNAL(triggered(bool)),
          this, SLOT(openSegmentations()));

  m_close = addAction(tr("Close Selected Segmentations"));
  m_close->setIcon(QIcon(":/espina/close.png"));
  connect(m_close, SIGNAL(triggered(bool)),
          this, SLOT(closeSegmentations()));

  m_fill = addAction(tr("Fill Holes in Selected Segmentations"));
  m_fill->setIcon(QIcon(":/espina/fillHoles.svg"));
  connect(m_fill, SIGNAL(triggered(bool)),
          this, SLOT(fillHoles()));

  m_pencilSelector->setSelectable(SelectionHandler::EspINA_Channel);
  m_pencilSelector->changeState(PencilSelector::CREATING);
  connect(m_pencilSelector, SIGNAL(selectionChanged(SelectionHandler::MultiSelection)),
          this, SLOT(drawSegmentation(SelectionHandler::MultiSelection)));
  connect(m_pencilSelector, SIGNAL(selectionAborted()),
          this, SLOT(stopDrawing()));
  connect(m_pencilSelector, SIGNAL(stateChanged(PencilSelector::State)),
          this, SLOT(stateChanged(PencilSelector::State)));

  m_contourSelector->setSelectable(SelectionHandler::EspINA_Channel);
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
void EditorToolBar::startPencilDrawing()
{
  if (m_actionGroup->isChecked())
  {
    SegmentationList selSegs = selectedSegmentations();
    if (selSegs.size() == 1)
    {
      m_currentSeg = selSegs.first();
      m_currentSource = m_currentSeg->filter();
      m_pencilSelector->changeState(PencilSelector::DRAWING);
      m_pencilSelector->setColor(m_currentSeg->taxonomy()->color());
    }
    else
    {
      m_currentSeg = NULL;
      m_currentSource = NULL;
      m_pencilSelector->changeState(PencilSelector::CREATING);
      m_pencilSelector->setColor(SelectionManager::instance()->activeTaxonomy()->color());
    }

    m_pencilSelector->setRadius(m_settings->brushRadius());
    SelectionManager::instance()->setSelectionHandler(m_pencilSelector);
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::drawSegmentation(SelectionHandler::MultiSelection msel)
{
  if (msel.size() == 0)
    return;

  SelectionHandler::VtkRegion region = msel.first().first;
  if (region.size() < 5)
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

  vtkSmartPointer<vtkImplicitFunction> brush;
  BrushType brushType = CIRCULAR;

  if (m_actionGroup->getCurrentAction() == m_pencilDisc)
    brushType = CIRCULAR;
  else
    if (m_actionGroup->getCurrentAction() == m_pencilSphere)
      brushType = SPHERICAL;
    else
      if (m_actionGroup->getCurrentAction() == m_contour)
        brushType = CONTOUR;

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
    case CONTOUR:
    {
      // TODO: make contour implicit function
      vtkSmartPointer<vtkTube> circularBrush = vtkSmartPointer<vtkTube>::New();
      circularBrush->SetBaseCenter(baseCenter);
      circularBrush->SetBaseRadius(radius);
      circularBrush->SetTopCenter(topCenter);
      circularBrush->SetTopRadius(radius);
      brush = circularBrush;
    }
      break;
    default:
      Q_ASSERT(false);
      break;
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
    Q_ASSERT (PencilSelector::CREATING == m_pencilSelector->state());

    m_currentSource->draw(0, brush, bounds);
    m_currentSeg = EspinaFactory::instance()->createSegmentation(m_currentSource, 0);
    TaxonomyNode *tax = SelectionManager::instance()->activeTaxonomy();
    undo->push(new FreeFormCommand(channel, m_currentSource, m_currentSeg, tax));
    m_pencilSelector->changeState(PencilSelector::DRAWING);
  }
  else
  {
    unsigned int output = m_currentSeg->outputNumber();
    switch (m_pencilSelector->state())
    {
      case PencilSelector::DRAWING:
        m_currentSource->draw(output, brush, bounds);
        break;
      case PencilSelector::ERASING:
        m_currentSource->draw(output, brush, bounds, 0);
        break;
      case PencilSelector::CREATING:
      default:
        Q_ASSERT(FALSE);
        break;
    }
  }

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
  m_actionGroup->blockSignals(true);
  m_actionGroup->setChecked(false);
  m_pencilSelector->changeState(PencilSelector::DRAWING);
  m_actionGroup->blockSignals(false);
}

//----------------------------------------------------------------------------
void EditorToolBar::stateChanged(PencilSelector::State state)
{
  switch (state)
  {
    case PencilSelector::DRAWING:
      if (m_pencilDisc == m_actionGroup->getCurrentAction())
        m_actionGroup->setIcon(QIcon(":/espina/pencil2D.png"));
      else
        m_actionGroup->setIcon(QIcon(":/espina/pencil3D.png"));
      break;
    case PencilSelector::ERASING:
      if (m_pencilDisc == m_actionGroup->getCurrentAction())
        m_actionGroup->setIcon(QIcon(":/espina/eraser2D.png"));
      else
        m_actionGroup->setIcon(QIcon(":/espina/eraser3D.png"));
      break;
    case PencilSelector::CREATING:
      break;
    default:
      Q_ASSERT(false);
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

//----------------------------------------------------------------------------
void EditorToolBar::startContourDrawing()
{
  if (m_actionGroup->isChecked())
  {
    m_contourWidget = new ContourWidget();
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    view->addWidget(m_contourWidget);
    m_contourWidget->setEnabled(true);

    SegmentationList selSegs = selectedSegmentations();
    if (selSegs.size() == 1)
    {
      m_currentSeg = selSegs.first();
      m_currentSource = m_currentSeg->filter();
    }
    else
    {
      m_currentSeg = NULL;
      m_currentSource = NULL;
    }

    SelectionManager::instance()->setSelectionHandler(m_contourSelector);
  }
}

void EditorToolBar::cancelDrawOperation()
{
  this->m_actionGroup->cancel();

  SelectionManager::instance()->setSelectionHandler(NULL);
  m_currentSource = NULL;
  m_currentSeg = NULL;

  // additional contour cleaning
  if (this->m_contour == this->m_actionGroup->getCurrentAction())
  {
    EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
    m_contourWidget->setEnabled(false);
    view->removeWidget(m_contourWidget);
    delete m_contourWidget;
    view->forceRender();
  }
}

void EditorToolBar::startDrawOperation(QAction *action)
{
  if (!SelectionManager::instance()->activeChannel()
   || !SelectionManager::instance()->activeTaxonomy()) 
  {
    m_actionGroup->setChecked(false);
    return;
  }
  
  if (m_pencilDisc == action || m_pencilSphere == action)
    startPencilDrawing();
  else
    if (m_contour == action)
      startContourDrawing();
}
