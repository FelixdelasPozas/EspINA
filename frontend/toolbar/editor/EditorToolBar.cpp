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
#include <editor/split/SplitFilter.h>
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
#include "split/PlanarSplitTool.h"

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
    QApplication::setOverrideCursor(Qt::WaitCursor);

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

    QApplication::restoreOverrideCursor();
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
      // TODO 2012-11-05 Extensesions need to be updated when
      // notifyModification method is called (at least with true)
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
, m_splitToolSelector(new ActionSelector(this))
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_settings(new Settings())
{
  setObjectName("EditorToolBar");
  setWindowTitle("Editor Tool Bar");

  initFactoryExtension(m_model->factory());

  m_model->factory()->registerSettingsPanel(new EditorToolBar::SettingsPanel(m_settings));

  initDrawTools();
  initSplitTools();
  initMorphologicalTools();
  initCODETools();
  initFillTool();

  updateAvailableOperations();
}

//----------------------------------------------------------------------------
void EditorToolBar::initFactoryExtension(EspinaFactory* factory)
{
  factory->registerFilter(this, SplitFilter::TYPE);
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
  if (SplitFilter::TYPE == filter)
    return new SplitFilter(inputs, args);
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
void EditorToolBar::changeCircularBrushMode(Brush::BrushMode mode)
{
  QString icon;
  if (Brush::BRUSH == mode)
    icon = ":/espina/pencil2D.png";
  else
    icon = ":/espina/eraser2D.png";

  m_drawToolSelector->setIcon(QIcon(icon));
}

//----------------------------------------------------------------------------
void EditorToolBar::changeSphericalBrushMode(Brush::BrushMode mode)
{
  QString icon;
  if (Brush::BRUSH == mode)
    icon = ":/espina/pencil2D.png";
  else
    icon = ":/espina/eraser2D.png";

  m_drawToolSelector->setIcon(QIcon(icon));
}

//----------------------------------------------------------------------------
void EditorToolBar::changeDrawTool(QAction *action)
{
  Q_ASSERT(m_drawTools.contains(action));
  m_viewManager->setActiveTool(m_drawTools[action]);
}

//----------------------------------------------------------------------------
void EditorToolBar::cancelDrawOperation()
{
  m_drawToolSelector->cancel();

  QAction *activeAction = m_drawToolSelector->getCurrentAction();
  ITool *activeTool = m_drawTools[activeAction];
  m_viewManager->unsetActiveTool(activeTool);
}

//----------------------------------------------------------------------------
void EditorToolBar::changeSplitTool(QAction *action)
{
  Q_ASSERT(m_splitTools.contains(action));
  m_viewManager->setActiveTool(m_splitTools[action]);
}

//----------------------------------------------------------------------------
void EditorToolBar::cancelSplitOperation()
{
  m_splitToolSelector->cancel();

  QAction *activeAction = m_splitToolSelector->getCurrentAction();
  ITool *activeTool = m_splitTools[activeAction];
  m_viewManager->unsetActiveTool(activeTool);
}

//----------------------------------------------------------------------------
void EditorToolBar::combineSegmentations()
{
  m_viewManager->unsetActiveTool();

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
  m_viewManager->unsetActiveTool();

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
void EditorToolBar::closeSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->closeRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::CLOSE, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::openSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->openRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::OPEN, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::dilateSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->dilateRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::DILATE, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::erodeSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->erodeRadius();
    m_undoStack->push(new CODECommand(input, CODECommand::ERODE, r, m_model));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::fillHoles()
{
  m_viewManager->unsetActiveTool();

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
void EditorToolBar::initDrawTools()
{
  // draw with a disc
  QAction *discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                                  tr("Draw segmentations using a disc"),
                                  m_drawToolSelector);

  CircularBrush *circularBrush = new CircularBrush(m_model,
                                                   m_undoStack,
                                                   m_viewManager);
  connect(circularBrush, SIGNAL(stopDrawing()),
          this, SLOT(cancelDrawOperation()));
  connect(circularBrush, SIGNAL(brushModeChanged(Brush::BrushMode)),
          this, SLOT(changeCircularBrushMode(Brush::BrushMode)));
  m_drawTools[discTool] = circularBrush;
  m_drawToolSelector->addAction(discTool);

  // draw with a sphere
  QAction *sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                                    tr("Draw segmentations using a sphere"),
                                    m_drawToolSelector);

  SphericalBrush *sphericalBrush = new SphericalBrush(m_model,
                                                      m_undoStack,
                                                      m_viewManager);
  connect(sphericalBrush, SIGNAL(stopDrawing()),
          this, SLOT(cancelDrawOperation()));
  connect(sphericalBrush, SIGNAL(brushModeChanged(Brush::BrushMode)),
          this, SLOT(changeSphericalBrushMode(Brush::BrushMode)));
  m_drawTools[sphereTool] = sphericalBrush;
  m_drawToolSelector->addAction(sphereTool);

  // draw with contour
  QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
                                       tr("Draw segmentations using contours"),
                                       m_drawToolSelector);

  m_drawTools[contourTool] = new FilledContour(m_model,
                                               m_undoStack,
                                               m_viewManager);
  m_drawToolSelector->addAction(contourTool);

  // Add Draw Tool Selector to Editor Tool Bar
  m_drawToolSelector->setCheckable(true);
  addAction(m_drawToolSelector);
  connect(m_drawToolSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changeDrawTool(QAction*)));
  connect(m_drawToolSelector, SIGNAL(actionCanceled()),
          this, SLOT(cancelDrawOperation()));
  m_drawToolSelector->setDefaultAction(discTool);

}

//----------------------------------------------------------------------------
void EditorToolBar::initSplitTools()
{
  QAction *planarSplit = new QAction(QIcon(":/espina/planar_split.svg"),
                                    tr("Split Segmentations using an orthogonal plane"),
                                    m_splitToolSelector);

  PlanarSplitTool *planarSplitTool = new PlanarSplitTool(m_model, m_undoStack, m_viewManager);
  connect(planarSplitTool, SIGNAL(splittingStopped()),
          this, SLOT(cancelSplitOperation()));
  m_splitTools[planarSplit] = planarSplitTool;
  m_splitToolSelector->addAction(planarSplit);

  // Add Split Tool Selector to Editor Tool Bar
  addAction(m_splitToolSelector);
  connect(m_splitToolSelector, SIGNAL(triggered(QAction*)),
          this, SLOT(changeSplitTool(QAction*)));
  connect(m_splitToolSelector, SIGNAL(actionCanceled()),
          this, SLOT(cancelSplitOperation()));

  m_splitToolSelector->setDefaultAction(planarSplit);
}

//----------------------------------------------------------------------------
void EditorToolBar::initMorphologicalTools()
{
  m_addition = addAction(tr("Combine Selected Segmentations"));
  m_addition->setIcon(QIcon(":/espina/add.svg"));
  connect(m_addition, SIGNAL(triggered(bool)),
          this, SLOT(combineSegmentations()));

  m_substraction = addAction(tr("Subtract Selected Segmentations"));
  m_substraction->setIcon(QIcon(":/espina/remove.svg"));
  connect(m_substraction, SIGNAL(triggered(bool)),
          this, SLOT(substractSegmentations()));
}

//----------------------------------------------------------------------------
void EditorToolBar::initCODETools()
{
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
}

//----------------------------------------------------------------------------
void EditorToolBar::initFillTool()
{
  m_fill = addAction(tr("Fill Holes in Selected Segmentations"));
  m_fill->setIcon(QIcon(":/espina/fillHoles.svg"));
  connect(m_fill, SIGNAL(triggered(bool)),
          this, SLOT(fillHoles()));

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection)),
          this, SLOT(updateAvailableOperations()));
}

//----------------------------------------------------------------------------
void EditorToolBar::updateAvailableOperations()
{
  SegmentationList segs = selectedSegmentations();

  bool one = segs.size() == 1;
  QString oneToolTip;
  if (!one)
    oneToolTip = tr(" (This tool requires just one segmentation to be selected)");

  bool atLeastTwo = segs.size()  > 1;
  QString atLeastTwoToolTip;
  if (!atLeastTwo)
    atLeastTwoToolTip = tr(" (This tool requires at least two segmentation to be selected)");

  bool several    = segs.size()  > 0;
  QString severalToolTip;
  if (!several)
    severalToolTip = tr(" (This tool requires at least one segmentation to be selected)");

  m_splitToolSelector->setEnabled(one);
  m_splitToolSelector->setToolTip(tr("Split Segmentations using an orthogonal plane") + oneToolTip);

  m_addition->setEnabled(atLeastTwo);
  m_addition->setToolTip(tr("Combine Selected Segmentations") + atLeastTwoToolTip);
  m_substraction->setEnabled(atLeastTwo);
  m_substraction->setToolTip(tr("Subtract Selected Segmentations") + atLeastTwoToolTip);

  m_close->setEnabled(several);
  m_close->setToolTip(tr("Close Selected Segmentations") + severalToolTip);
  m_open->setEnabled(several);
  m_open->setToolTip(tr("Open Selected Segmentations") + severalToolTip);
  m_dilate->setEnabled(several);
  m_dilate->setToolTip(tr("Dilate Selected Segmentations") + severalToolTip);
  m_erode->setEnabled(several);
  m_erode->setToolTip(tr("Erode Selected Segmentations") + severalToolTip);

  m_fill->setEnabled(several);
  m_fill->setToolTip(tr("Fill Holes in Selected Segmentations") + severalToolTip);
}

//----------------------------------------------------------------------------
void EditorToolBar::resetState()
{
  if (m_drawToolSelector->isChecked())
    cancelDrawOperation();

  if (m_splitToolSelector->isChecked())
    cancelSplitOperation();
}
