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

// EspINA
#include "common/editor/ImageLogicCommand.h"
#include "common/editor/FreeFormSource.h"
#include "common/editor/ClosingFilter.h"
#include "common/editor/ErodeFilter.h"
#include "common/editor/DilateFilter.h"
#include "common/editor/OpeningFilter.h"
#include "common/editor/FillHolesCommand.h"
#include "common/editor/FillHolesFilter.h"
#include "common/editor/ContourSelector.h"
#include "common/editor/ContourSource.h"
#include "common/editor/ContourWidget.h"
#include "common/editor/vtkTube.h"
#include "common/model/Channel.h"
#include "common/model/EspinaFactory.h"
#include "common/model/EspinaModel.h"
#include "common/gui/ActionSelector.h"
#include "common/gui/ViewManager.h"
#include "common/selection/PickableItem.h"
#include "common/undo/RemoveSegmentation.h"
#include "frontend/toolbar/editor/Settings.h"
#include "frontend/toolbar/editor/SettingsPanel.h"

// Qt
#include <QAction>

// VTK
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
  explicit FreeFormCommand(Channel         *channel,
                           Filter          *filter,
                           Segmentation    *seg,
                           TaxonomyElement *taxonomy,
                           EspinaModel     *model
                          )
  : m_model   (model)
  , m_channel (channel)
  , m_filter  (filter)
  , m_seg     (seg)
  , m_taxonomy(taxonomy)
  {
    m_sample = m_channel->sample();
    Q_ASSERT(m_sample);
  }

  virtual void redo()
  {
    m_model->addFilter(m_filter);
    m_model->addRelation(m_channel, m_filter, Channel::LINK);
    m_seg->setTaxonomy(m_taxonomy);
    m_model->addSegmentation(m_seg);
    m_model->addRelation(m_filter, m_seg, CREATELINK);
    m_model->addRelation(m_sample, m_seg, "where");
    m_model->addRelation(m_channel, m_seg, Channel::LINK);
    m_seg->initializeExtensions();
  }

  virtual void undo()
  {
    m_model->removeRelation(m_channel, m_seg, Channel::LINK);
    m_model->removeRelation(m_sample, m_seg, "where");
    m_model->removeRelation(m_filter, m_seg, CREATELINK);
    m_model->removeSegmentation(m_seg);
    m_model->removeRelation(m_channel, m_filter, Channel::LINK);
    m_model->removeFilter(m_filter);
  }

private:
  EspinaModel  *m_model;
  Sample       *m_sample;
  Channel      *m_channel;
  Filter       *m_filter;
  Segmentation *m_seg;
  TaxonomyElement *m_taxonomy;
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
  static const QString INPUTLINK; //TODO 2012-10-05 Move to CODEFilter ?
  typedef QPair<Filter *, unsigned int> Connection;
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
                       unsigned int radius,
                       EspinaModel *model
                      )
  : m_model(model)
  , m_segmentations(inputs)
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
    for(unsigned int i=0; i<m_newConnections.size(); i++)
    {
      Segmentation *seg        = m_segmentations[i];
      Connection oldConnection = m_oldConnections[i];
      Connection newConnection = m_newConnections[i];

      m_model->removeRelation(oldConnection.first, seg, CREATELINK);
      m_model->addFilter(newConnection.first);
      m_model->addRelation(oldConnection.first, newConnection.first, INPUTLINK);
      m_model->addRelation(newConnection.first, seg, CREATELINK);
      seg->changeFilter(newConnection.first, newConnection.second);
      seg->notifyModification(true);
    }
  }

  virtual void undo()
  {
    for(unsigned int i=0; i<m_newConnections.size(); i++)
    {
      Segmentation *seg        = m_segmentations[i];
      Connection oldConnection = m_oldConnections[i];
      Connection newConnection = m_newConnections[i];

      m_model->removeRelation(newConnection.first, seg, CREATELINK);
      m_model->removeRelation(oldConnection.first, newConnection.first, INPUTLINK);
      m_model->removeFilter(newConnection.first);
      m_model->addRelation(oldConnection.first, seg, CREATELINK);
      seg->changeFilter(oldConnection.first, oldConnection.second);
      seg->notifyModification(true);
    }
  }

private:
  EspinaModel *m_model;
  QList<Connection> m_oldConnections, m_newConnections;
  QList<Segmentation *> m_segmentations;
};

const QString EditorToolBar::CODECommand::INPUTLINK = "Input";

//----------------------------------------------------------------------------
EditorToolBar::EditorToolBar(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *vm,
                             QWidget* parent)
: QToolBar(parent)
, m_actionGroup(new ActionSelector(this))
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_settings(new Settings())
, m_brush(new BrushSelector())
, m_contourSelector(new ContourSelector())
, m_currentSource(NULL)
, m_currentSeg(NULL)
{
  setObjectName("EditorToolBar");
  setWindowTitle("Editor Tool Bar");

  initFactoryExtension(m_model->factory());

  m_model->factory()->registerSettingsPanel(new EditorToolBar::SettingsPanel(m_settings));

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

  m_brush->setPickable(IPicker::CHANNEL);
  m_brush->changeState(BrushSelector::CREATING);
  connect(m_brush, SIGNAL(itemsPicked(IPicker::PickList)),
          this, SLOT(drawSegmentation(IPicker::PickList)));
  connect(m_brush, SIGNAL(selectionAborted()),
          this, SLOT(stopDrawing()));
  connect(m_brush, SIGNAL(stateChanged(BrushSelector::State)),
          this, SLOT(stateChanged(BrushSelector::State)));

  m_contourSelector->setPickable(IPicker::CHANNEL);
}

//----------------------------------------------------------------------------
void EditorToolBar::initFactoryExtension(EspinaFactory* factory)
{
  factory->registerFilter(this, ClosingFilter::TYPE);
  factory->registerFilter(this, OpeningFilter::TYPE);
  factory->registerFilter(this, DilateFilter::TYPE);
  factory->registerFilter(this, ErodeFilter::TYPE);
  factory->registerFilter(this, FreeFormSource::TYPE);
  factory->registerFilter(this, ImageLogicFilter::TYPE);
  factory->registerFilter(this, FillHolesFilter::TYPE);
  factory->registerFilter(this, ContourSource::TYPE);
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
      m_brush->changeState(BrushSelector::DRAWING);
      m_brush->setColor(m_currentSeg->taxonomy()->color());
    }
    else
    {
      m_currentSeg = NULL;
      m_currentSource = NULL;
      m_brush->changeState(BrushSelector::CREATING);
      m_brush->setColor(m_viewManager->activeTaxonomy()->color());
    }

    m_brush->setRadius(m_settings->brushRadius());
    m_viewManager->setPicker(m_brush);
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::drawSegmentation(IPicker::PickList pickedList)
{
  if (pickedList.size() == 0)
    return;

  IPicker::WorldRegion region = pickedList.first().first;
  if (region.size() < 5)
    return;

  PickableItem *pickedItem = pickedList.first().second;
  Q_ASSERT(ModelItem::CHANNEL == pickedItem->type());
  Channel *channel = dynamic_cast<Channel *>(pickedItem);
  double spacing[3];
  channel->spacing(spacing);

  double points[5][3];
  double *center = points[0];
  double *right  = points[1];
  double *top    = points[2];
//   double *left   = points[3];
//   double *bottom = points[4];

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
      return;

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

  if (!m_currentSeg && m_currentSource)
  {
    // prevent this case
    if (BrushSelector::ERASING == m_brush->state())
      return;

    // Create a new segmentation
    m_currentSource->draw(0, brush, bounds);
    m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, 0);
    TaxonomyElement *tax = m_viewManager->activeTaxonomy();
    m_undoStack->push(new FreeFormCommand(channel, m_currentSource, m_currentSeg, tax, m_model));
    m_brush->changeState(BrushSelector::DRAWING);
  }
  else
  {
    unsigned int output = m_currentSeg->outputNumber();
    switch (m_brush->state())
    {
      case BrushSelector::DRAWING:
        m_currentSource->draw(output, brush, bounds);
        break;
      case BrushSelector::ERASING:
        m_currentSource->draw(output, brush, bounds, 0);
        break;
      case BrushSelector::CREATING:
      default:
        Q_ASSERT(FALSE);
        break;
    }
  }

  m_viewManager->updateViews();

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
  m_brush->changeState(BrushSelector::DRAWING);
  m_actionGroup->blockSignals(false);
}

//----------------------------------------------------------------------------
void EditorToolBar::stateChanged(BrushSelector::State state)
{
  switch (state)
  {
    case BrushSelector::DRAWING:
      if (m_pencilDisc == m_actionGroup->getCurrentAction())
        m_actionGroup->setIcon(QIcon(":/espina/pencil2D.png"));
      else
        m_actionGroup->setIcon(QIcon(":/espina/pencil3D.png"));
      break;
    case BrushSelector::ERASING:
      if (m_pencilDisc == m_actionGroup->getCurrentAction())
        m_actionGroup->setIcon(QIcon(":/espina/eraser2D.png"));
      else
        m_actionGroup->setIcon(QIcon(":/espina/eraser3D.png"));
      break;
    case BrushSelector::CREATING:
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
    m_undoStack->beginMacro("Combine Segmentations");
    m_undoStack->push(new ImageLogicCommand(input,
                                            ImageLogicFilter::ADDITION,
                                            m_model,
                                            m_viewManager->activeTaxonomy()));
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::substractSegmentations()
{
  SegmentationList input = selectedSegmentations();

  if (input.size() > 1)
  {
    m_undoStack->beginMacro("Substract Segmentations");
    m_undoStack->push(new ImageLogicCommand(input,
                                            ImageLogicFilter::SUBSTRACTION,
                                            m_model,
                                            m_viewManager->activeTaxonomy()));
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::erodeSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->erodeRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::ERODE, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::dilateSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->dilateRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::DILATE, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::openSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->openRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::OPEN, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::closeSegmentations()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->closeRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::CLOSE, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::fillHoles()
{
  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    m_undoStack->push(new FillHolesCommand(input, m_model));
  }
}

//----------------------------------------------------------------------------
SegmentationList EditorToolBar::selectedSegmentations()
{
  SegmentationList selection;

  foreach(PickableItem *item, m_viewManager->selection())
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
    m_viewManager->addWidget(m_contourWidget);
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

    m_viewManager->setPicker(m_contourSelector);
  }
}

void EditorToolBar::cancelDrawOperation()
{
  this->m_actionGroup->cancel();

  // additional contour cleaning
  if (this->m_contour == this->m_actionGroup->getCurrentAction())
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_contourWidget->setEnabled(false);

    if (0 != m_contourWidget->GetContoursNumber())
    {
      Channel *channel = m_viewManager->activeChannel();
      double spacing[3];
      channel->spacing(spacing);

      if (!m_currentSource && !m_currentSeg)
      {
        Filter::NamedInputs inputs;
        Filter::Arguments args;
        FreeFormSource::Parameters params(args);
        params.setSpacing(spacing);
        m_currentSource = new ContourSource(inputs, args);
      }

      if (!m_currentSeg && m_currentSource)
      {
        m_currentSource->draw(0, NULL, 0, AXIAL);
        m_currentSeg = m_model->factory()->createSegmentation(m_currentSource, 0);
        TaxonomyElement *tax = m_viewManager->activeTaxonomy();
        m_undoStack->push(new FreeFormCommand(channel, m_currentSource, m_currentSeg, tax, m_model));
      }

      QMap<PlaneType, QMap<Nm, vtkPolyData*> > contours = m_contourWidget->GetContours();
      QMap<Nm, vtkPolyData*>::iterator it = contours[AXIAL].begin();
      while (it != contours[AXIAL].end())
      {
        m_currentSource->draw(0, it.value(), it.key(), AXIAL);
        ++it;
      }

      it = contours[CORONAL].begin();
      while (it != contours[CORONAL].end())
      {
        m_currentSource->draw(0, it.value(), it.key(), CORONAL);
        ++it;
      }

      it = contours[SAGITTAL].begin();
      while (it != contours[SAGITTAL].end())
      {
        m_currentSource->draw(0, it.value(), it.key(), SAGITTAL);
        ++it;
      }

      ContourSource *filter = dynamic_cast<ContourSource *>(m_currentSource);
      filter->signalAsModified();
    }

    m_viewManager->removeWidget(m_contourWidget);
    delete m_contourWidget;
    m_viewManager->updateViews();
    QApplication::restoreOverrideCursor();
  }

  m_viewManager->setPicker(NULL);
  m_currentSource = NULL;
  m_currentSeg = NULL;
}

void EditorToolBar::startDrawOperation(QAction *action)
{
  if (!m_viewManager->activeChannel()
   || !m_viewManager->activeTaxonomy())
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
