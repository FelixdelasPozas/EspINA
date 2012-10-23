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
#include "common/model/Channel.h"
#include "common/model/EspinaFactory.h"
#include "common/model/EspinaModel.h"
#include "common/gui/ActionSelector.h"
#include "common/gui/ViewManager.h"
#include "common/tools/PickableItem.h"
#include "common/undo/RemoveSegmentation.h"
#include <undo/AddSegmentation.h>
#include "frontend/toolbar/editor/Settings.h"
#include "frontend/toolbar/editor/SettingsPanel.h"
#include "CircularBrush.h"
#include "SphericalBrush.h"
#include "FilledContour.h"

// Qt
#include <QAction>

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
, m_drawToolSelector(new ActionSelector(this))
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_settings(new Settings())
, m_currentSource(NULL)
, m_currentSeg(NULL)
{
  setObjectName("EditorToolBar");
  setWindowTitle("Editor Tool Bar");

  initFactoryExtension(m_model->factory());

  m_model->factory()->registerSettingsPanel(new EditorToolBar::SettingsPanel(m_settings));

  // draw with a disc
  QAction *discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                                   tr("Drew segmentations using a disc"),
                                   m_drawToolSelector);
  CircularBrush *circularBrush = new CircularBrush(m_model, m_undoStack, m_viewManager);
  connect(circularBrush, SIGNAL(stopDrawing()),
          this, SLOT(cancelDrawOperation()));
  m_drawTools[discTool] = circularBrush;
  m_drawToolSelector->addAction(discTool);

  // draw with a sphere
  QAction *sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                                    tr("Draw segmentations using a sphere"),
                                    m_drawToolSelector);
  SphericalBrush *sphericalBrush = new SphericalBrush(m_model, m_undoStack, m_viewManager);
  connect(sphericalBrush, SIGNAL(stopDrawing()),
          this, SLOT(cancelDrawOperation()));
  m_drawTools[sphereTool] = sphericalBrush;
  m_drawToolSelector->addAction(sphereTool);

  // draw with contour
  QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
                                       tr("Draw segmentations using contours"),
                                       m_drawToolSelector);
  m_drawTools[contourTool] = new FilledContour();
  m_drawToolSelector->addAction(contourTool);

  m_drawToolSelector->setCheckable(true);
  addAction(m_drawToolSelector);
  connect(m_drawToolSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changeDrawTool(QAction*)));
  connect(m_drawToolSelector, SIGNAL(actionCanceled()),
          this, SLOT(cancelDrawOperation()));
  m_drawToolSelector->setDefaultAction(discTool);

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
  /* TODO 2012-10-18
  if (m_actionGroup->isChecked())
  {
    m_contourWidget = new ContourWidget();

    SegmentationList selSegs = selectedSegmentations();
    if (selSegs.size() == 1)
    {
      m_currentSeg = selSegs.first();
      m_currentSource = m_currentSeg->filter();
      m_contourWidget->setPolygonColor(m_viewManager->color(m_currentSeg));
    }
    else
    {
      m_currentSeg = NULL;
      m_currentSource = NULL;
      m_contourWidget->setPolygonColor(m_viewManager->activeTaxonomy()->color());
    }

    //TODO 2012-10-17 m_viewManager->setActiveTool(m_contourSelector);
    m_viewManager->addWidget(m_contourWidget);
    m_contourWidget->setEnabled(true);
  }
  */
}

void EditorToolBar::cancelDrawOperation()
{
  m_drawToolSelector->cancel();

  QAction *activeAction = m_drawToolSelector->getCurrentAction();
  ITool *activeTool = m_drawTools[activeAction];
  m_viewManager->unsetActiveTool(activeTool);

  /*
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
    //TODO 2012-10-17 m_viewManager->unsetPicker(m_contourSelector);
    delete m_contourWidget;
    m_viewManager->updateViews();
    QApplication::restoreOverrideCursor();

  }
  else
  {
    // only for paint operations
    //TODO 2012-10-17 m_viewManager->unsetPicker(m_brush);
  }
  */

  m_currentSource = NULL;
  m_currentSeg = NULL;
}

//----------------------------------------------------------------------------
void EditorToolBar::changeDrawTool(QAction *action)
{
  Q_ASSERT(m_drawTools.contains(action));
  m_viewManager->setActiveTool(m_drawTools[action]);
}