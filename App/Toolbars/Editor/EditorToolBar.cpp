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

#include "Settings/Editor/SettingsPanel.h"
#include "Toolbars/Editor/Settings.h"
#include "Tools/Brushes/CircularBrush.h"
#include "Tools/Brushes/SphericalBrush.h"
#include "Tools/Contour/FilledContour.h"
#include "Tools/PlanarSplit/PlanarSplitTool.h"

// EspINA
#include <App/FilterInspectors/CODE/CODEFilterInspector.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/PickableItem.h>
#include <Filters/ClosingFilter.h>
#include <Filters/ContourSource.h>
#include <Filters/DilateFilter.h>
#include <Filters/ErodeFilter.h>
#include <Filters/FillHolesFilter.h>
#include <Filters/FreeFormSource.h>
#include <Filters/OpeningFilter.h>
#include <Filters/SplitFilter.h>
#include <GUI/Pickers/ContourPicker.h>
#include <GUI/QtWidget/ActionSelector.h>
#include <GUI/ViewManager.h>
#include <GUI/vtkWidgets/ContourWidget.h>
#include <Undo/FillHolesCommand.h>
#include <Undo/ImageLogicCommand.h>
#include <Undo/SplitUndoCommand.h>
#include <Undo/RemoveSegmentation.h>

// Qt
#include <QAction>
#include <QMessageBox>

using namespace EspINA;

namespace EspINA
{
  //----------------------------------------------------------------------------
  class EditorToolBar::CODECommand :
  public QUndoCommand
  {
  public:
    static const Filter::FilterType CLOSING_FILTER_TYPE;
    static const Filter::FilterType OPENING_FILTER_TYPE;
    static const Filter::FilterType DILATE_FILTER_TYPE;
    static const Filter::FilterType ERODE_FILTER_TYPE;

  private:
    static const QString INPUTLINK; //TODO 2012-10-05 Move to CODEFilter ?
    typedef QPair<FilterSPtr, Filter::OutputId> Connection;

  public:
    enum Operation
    {
      CLOSE,
      OPEN,
      DILATE,
      ERODE
    };

  public:
    explicit CODECommand(SegmentationList inputs,
                         Operation        op,
                         unsigned int     radius,
                         EspinaModel      *model,
                         ViewManager      *viewManager)
    : m_model      (model      )
    , m_viewManager(viewManager)
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);

      foreach(SegmentationPtr seg, inputs)
      {
        MorphologicalEditionFilter *filter;
        Filter::FilterInspectorPtr filterInspector;

        Filter::NamedInputs inputs;
        Filter::Arguments args;
        MorphologicalEditionFilter::Parameters params(args);
        params.setRadius(radius);
        inputs[INPUTLINK] = seg->filter();
        args[Filter::INPUTS] = Filter::NamedInput(INPUTLINK, seg->outputId());
        switch (op)
        {
          case CLOSE:
            filter = new ClosingFilter(inputs, args, CLOSING_FILTER_TYPE);
            filterInspector = Filter::FilterInspectorPtr(new CODEFilterInspector("Close", filter));
            break;
          case OPEN:
            filter = new OpeningFilter(inputs, args, OPENING_FILTER_TYPE);
            filterInspector = Filter::FilterInspectorPtr(new CODEFilterInspector("Open", filter));
            break;
          case DILATE:
            filter = new DilateFilter(inputs, args, DILATE_FILTER_TYPE);
            filterInspector = Filter::FilterInspectorPtr(new CODEFilterInspector("Dilate", filter));
            break;
          case ERODE:
            filter = new ErodeFilter(inputs, args, ERODE_FILTER_TYPE);
            filterInspector = Filter::FilterInspectorPtr(new CODEFilterInspector("Erode", filter));
            break;
        }
        filter->setFilterInspector(filterInspector);
        filter->update();

        if (filter->isOutputEmpty())
        {
          m_removedSegmentations << seg;
          m_removedSegmentationsCommands.append(new RemoveSegmentation(seg, m_model, m_viewManager));
          delete filter;
          continue;
        }

        m_segmentations << m_model->findSegmentation(seg);
        m_newConnections << Connection(FilterSPtr(filter), 0);
        m_oldConnections << Connection(seg->filter(), seg->outputId());
      }

      QApplication::restoreOverrideCursor();
    }

    virtual void redo()
    {
      SegmentationList segmentations;
      for(int i=0; i<m_newConnections.size(); i++)
      {
        SegmentationSPtr seg      = m_segmentations[i];
        Connection oldConnection = m_oldConnections[i];
        Connection newConnection = m_newConnections[i];

        segmentations << seg.data();

        m_model->removeRelation(oldConnection.first, seg, Filter::CREATELINK);
        m_model->addFilter(newConnection.first);
        m_model->addRelation(oldConnection.first, newConnection.first, INPUTLINK);
        m_model->addRelation(newConnection.first, seg, Filter::CREATELINK);

        seg->changeFilter(newConnection.first, newConnection.second);
        seg->volume()->markAsModified();
      }
      m_viewManager->updateSegmentationRepresentations(segmentations);

      foreach(RemoveSegmentation *command, m_removedSegmentationsCommands)
        command->redo();
    }

    virtual void undo()
    {
      SegmentationList segmentations;
      for(int i=0; i<m_newConnections.size(); i++)
      {
        SegmentationSPtr seg      = m_segmentations[i];
        Connection oldConnection = m_oldConnections[i];
        Connection newConnection = m_newConnections[i];

        segmentations << seg.data();

        m_model->removeRelation(newConnection.first, seg, Filter::CREATELINK);
        m_model->removeRelation(oldConnection.first, newConnection.first, INPUTLINK);
        m_model->removeFilter(newConnection.first);
        m_model->addRelation(oldConnection.first, seg, Filter::CREATELINK);

        seg->changeFilter(oldConnection.first, oldConnection.second);
        seg->volume()->markAsModified();
      }
      m_viewManager->updateSegmentationRepresentations(segmentations);

      foreach(RemoveSegmentation *command, m_removedSegmentationsCommands)
        command->undo();
    }

    SegmentationList getRemovedSegmentations()
    {
      return m_removedSegmentations;
    }

    ~CODECommand()
    {
      foreach(RemoveSegmentation *command, m_removedSegmentationsCommands)
        delete command;

      m_newConnections.clear();
      m_oldConnections.clear();
    }

  private:
    EspinaModel *m_model;
    ViewManager *m_viewManager;

    QList<Connection> m_oldConnections, m_newConnections;
    SegmentationSList m_segmentations;
    QList<RemoveSegmentation *> m_removedSegmentationsCommands;
    SegmentationList m_removedSegmentations;
  };

  const Filter::FilterType EditorToolBar::CODECommand::CLOSING_FILTER_TYPE = "EditorToolBar::ClosingFilter";
  const Filter::FilterType EditorToolBar::CODECommand::OPENING_FILTER_TYPE = "EditorToolBar::OpeningFilter";
  const Filter::FilterType EditorToolBar::CODECommand::DILATE_FILTER_TYPE  = "EditorToolBar::DilateFilter";
  const Filter::FilterType EditorToolBar::CODECommand::ERODE_FILTER_TYPE   = "EditorToolBar::ErodeFilter";

} // namespace EspINA

const QString EditorToolBar::CODECommand::INPUTLINK = "Input";

//----------------------------------------------------------------------------
EditorToolBar::EditorToolBar(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *vm,
                             QWidget* parent)
: IToolBar(parent)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_drawToolSelector(new ActionSelector(this))
, m_splitToolSelector(new ActionSelector(this))
, m_settings(new Settings())
, editorSettings(new SettingsPanel(m_settings))
{
  setObjectName("EditorToolBar");

  setWindowTitle(tr("Editor Tool Bar"));

  initFactoryExtension(m_model->factory());

  initDrawTools();
  initSplitTools();
  initMorphologicalTools();
  initCODETools();
  initFillTool();

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateAvailableOperations()));
  updateAvailableOperations();
}

//----------------------------------------------------------------------------
EditorToolBar::~EditorToolBar()
{
  qDebug() << "********************************************************";
  qDebug() << "              Destroying Editor ToolbBar";
  qDebug() << "********************************************************";
  delete m_settings;
}


//----------------------------------------------------------------------------
void EditorToolBar::initToolBar(EspinaModel *model,
                                QUndoStack  *undoStack,
                                ViewManager *viewManager)
{

}

//----------------------------------------------------------------------------
void EditorToolBar::initFactoryExtension(EspinaFactoryPtr factory)
{
  factory->registerFilter(this, SplitUndoCommand::FILTER_TYPE);
  factory->registerFilter(this, CODECommand::CLOSING_FILTER_TYPE);
  factory->registerFilter(this, CODECommand::OPENING_FILTER_TYPE);
  factory->registerFilter(this, CODECommand::DILATE_FILTER_TYPE);
  factory->registerFilter(this, CODECommand::ERODE_FILTER_TYPE);
  factory->registerFilter(this, Brush::FREEFORM_SOURCE_TYPE);
  factory->registerFilter(this, ImageLogicCommand::FILTER_TYPE);
  factory->registerFilter(this, FillHolesCommand::FILTER_TYPE);
  factory->registerFilter(this, FilledContour::FILTER_TYPE);

  factory->registerSettingsPanel(editorSettings.data());
}

//----------------------------------------------------------------------------
FilterSPtr EditorToolBar::createFilter(const QString              &filter,
                                      const Filter::NamedInputs  &inputs,
                                      const ModelItem::Arguments &args)
{
  Filter *res = NULL;
  Filter::FilterInspector *filterInspector = NULL;

  if (SplitUndoCommand::FILTER_TYPE == filter)
    res = new SplitFilter(inputs, args, SplitUndoCommand::FILTER_TYPE);

  else if (CODECommand::CLOSING_FILTER_TYPE == filter)
  {
    MorphologicalEditionFilter *mf = new ClosingFilter(inputs, args, CODECommand::CLOSING_FILTER_TYPE);
    filterInspector = new CODEFilterInspector(tr("Close"), mf);
    res = mf;
  }

  else if (CODECommand::OPENING_FILTER_TYPE == filter)
  {
    MorphologicalEditionFilter *mf = new OpeningFilter(inputs, args, CODECommand::OPENING_FILTER_TYPE);
    filterInspector = new CODEFilterInspector(tr("Open"), mf);
    res = mf;
  }

  else if (CODECommand::DILATE_FILTER_TYPE == filter)
  {
    MorphologicalEditionFilter *mf = new DilateFilter(inputs, args, CODECommand::DILATE_FILTER_TYPE);
    filterInspector = new CODEFilterInspector(tr("Dilate"), mf);
    res = mf;
  }

  else if (CODECommand::ERODE_FILTER_TYPE == filter)
  {
    MorphologicalEditionFilter *mf = new ErodeFilter(inputs, args, CODECommand::ERODE_FILTER_TYPE);
    filterInspector = new CODEFilterInspector(tr("Erode"), mf);
    res = mf;
  }

  else if (Brush::FREEFORM_SOURCE_TYPE == filter)
  {
    res = new FreeFormSource(inputs, args, Brush::FREEFORM_SOURCE_TYPE);
  }

  else if (ImageLogicCommand::FILTER_TYPE == filter)
    res = new ImageLogicFilter(inputs, args, ImageLogicCommand::FILTER_TYPE);

  else if (FillHolesCommand::FILTER_TYPE == filter)
    res = new FillHolesFilter(inputs, args, FillHolesCommand::FILTER_TYPE);

  else if (FilledContour::FILTER_TYPE == filter)
  {
    res = new ContourSource(inputs, args, FilledContour::FILTER_TYPE);
    filterInspector = new ContourFilterInspector(res);
  }

  if (filterInspector != NULL)
    res->setFilterInspector(Filter::FilterInspectorPtr(filterInspector));

  return FilterSPtr(res);
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
    icon = ":/espina/pencil3D.png";
  else
    icon = ":/espina/eraser3D.png";

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
  IToolSPtr activeTool = m_drawTools[activeAction];
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
  IToolSPtr activeTool = m_splitTools[activeAction];
  m_viewManager->unsetActiveTool(activeTool);
}

//----------------------------------------------------------------------------
void EditorToolBar::combineSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 1)
  {
    m_viewManager->clearSelection(true);
    m_undoStack->beginMacro("Combine Segmentations");
    m_undoStack->push(
      new ImageLogicCommand(input,
                            ImageLogicFilter::ADDITION,
                            m_viewManager->activeTaxonomy(),
                            m_model));
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::substractSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 1)
  {
    m_viewManager->clearSelection(true);
    m_undoStack->beginMacro("Substract Segmentations");
    m_undoStack->push(new ImageLogicCommand(input,
                                            ImageLogicFilter::SUBSTRACTION,
                                            m_viewManager->activeTaxonomy(),
                                            m_model));
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::closeSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 0)
  {
    CODECommand *closeCommand = new CODECommand(input, CODECommand::CLOSE, m_settings->closeRadius(), m_model, m_viewManager);
    if (closeCommand->getRemovedSegmentations().size() > 0)
    {
      QMessageBox info;
      info.setWindowTitle("Close Segmentations");
      info.setIcon(QMessageBox::Warning);
      QString message(tr("The following segmentations will be deleted by the close operation:\n"));
      foreach(SegmentationPtr seg, closeCommand->getRemovedSegmentations())
        message += QString("  - ") + seg->data().toString() + QString("\n");
      message += tr("\nDo you want to continue with the operation?");
      info.setText(message);
      info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (info.exec() == QMessageBox::No)
      {
        delete closeCommand;
        return;
      }
    }

    m_undoStack->beginMacro(tr("Close Segmentation"));
    m_undoStack->push(closeCommand);
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::openSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 0)
  {
    CODECommand *openCommand = new CODECommand(input, CODECommand::OPEN, m_settings->openRadius(), m_model, m_viewManager);
    if (openCommand->getRemovedSegmentations().size() > 0)
    {
      QMessageBox info;
      info.setWindowTitle("Open Segmentations");
      info.setIcon(QMessageBox::Warning);
      QString message(tr("The following segmentations will be deleted by the open operation:\n"));
      foreach(SegmentationPtr seg, openCommand->getRemovedSegmentations())
        message += QString("  - ") + seg->data().toString() + QString("\n");
      message += tr("\nDo you want to continue with the operation?");
      info.setText(message);
      info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (info.exec() == QMessageBox::No)
      {
        delete openCommand;
        return;
      }
    }

    m_undoStack->beginMacro(tr("Open Segmentation"));
    m_undoStack->push(openCommand);
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::dilateSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 0)
  {
    int r = m_settings->dilateRadius();
    m_undoStack->beginMacro(tr("Dilate Segmentation"));
    m_undoStack->push(new CODECommand(input, CODECommand::DILATE, r, m_model, m_viewManager));
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::erodeSegmentations()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 0)
  {
    CODECommand *erodeCommand = new CODECommand(input, CODECommand::ERODE, m_settings->erodeRadius(), m_model, m_viewManager);
    if(erodeCommand->getRemovedSegmentations().size() > 0)
    {
      QMessageBox info;
      info.setWindowTitle("Erode Segmentations");
      info.setIcon(QMessageBox::Warning);
      QString message(tr("The following segmentations will be deleted by the erode operation:\n"));
      foreach(SegmentationPtr seg, erodeCommand->getRemovedSegmentations())
        message += QString("  - ") + seg->data().toString() + QString("\n");
      message += tr("\nDo you want to continue with the operation?");
      info.setText(message);
      info.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (info.exec() == QMessageBox::No)
      {
        delete erodeCommand;
        return;
      }
    }

    m_undoStack->beginMacro(tr("Erode Segmentation"));
    m_undoStack->push(erodeCommand);
    m_undoStack->endMacro();
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::fillHoles()
{
  m_viewManager->unsetActiveTool();

  SegmentationList input = m_viewManager->selectedSegmentations();
  if (input.size() > 0)
  {
    m_undoStack->push(new FillHolesCommand(input, m_model, m_viewManager));
  }
}

//----------------------------------------------------------------------------
void EditorToolBar::initDrawTools()
{
  // draw with a disc
  QAction *discTool = new QAction(QIcon(":/espina/pencil2D.png"),
                                  tr("Draw segmentations using a disc"),
                                  m_drawToolSelector);

  CircularBrushSPtr circularBrush(new CircularBrush(m_model,
                                                    m_undoStack,
                                                    m_viewManager) );
  connect(circularBrush.data(), SIGNAL(stopDrawing()),
          this, SLOT(cancelDrawOperation()));
  connect(circularBrush.data(), SIGNAL(brushModeChanged(Brush::BrushMode)),
          this, SLOT(changeCircularBrushMode(Brush::BrushMode)));

  m_drawTools[discTool] =  circularBrush;
  m_drawToolSelector->addAction(discTool);

  // draw with a sphere
  QAction *sphereTool = new QAction(QIcon(":espina/pencil3D.png"),
                                    tr("Draw segmentations using a sphere"),
                                    m_drawToolSelector);

  SphericalBrushSPtr sphericalBrush(new SphericalBrush(m_model,
                                                       m_undoStack,
                                                       m_viewManager));
  connect(sphericalBrush.data(), SIGNAL(stopDrawing()),
          this, SLOT(cancelDrawOperation()));
  connect(sphericalBrush.data(), SIGNAL(brushModeChanged(Brush::BrushMode)),
          this, SLOT(changeSphericalBrushMode(Brush::BrushMode)));

  m_drawTools[sphereTool] = sphericalBrush;
  m_drawToolSelector->addAction(sphereTool);

  // draw with contour
  QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
                                       tr("Draw segmentations using contours"),
                                       m_drawToolSelector);

  FilledContourSPtr contour(new FilledContour(m_model,
                                              m_undoStack,
                                              m_viewManager));

  m_drawTools[contourTool] = contour;
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

  PlanarSplitToolSPtr planarSplitTool(new PlanarSplitTool(m_model,
                                                          m_undoStack,
                                                          m_viewManager));
  connect(planarSplitTool.data(), SIGNAL(splittingStopped()),
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
}

//----------------------------------------------------------------------------
void EditorToolBar::updateAvailableOperations()
{
  SegmentationList segs = m_viewManager->selectedSegmentations();

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
void EditorToolBar::reset()
{
  if (m_drawToolSelector->isChecked())
    cancelDrawOperation();

  if (m_splitToolSelector->isChecked())
    cancelSplitOperation();
}
