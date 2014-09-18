/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "ManualROITool.h"
#include "ROITools.h"
#include <GUI/Widgets/SliderAction.h>
#include <GUI/Selectors/CircularBrushSelector.h>
#include <GUI/Selectors/SphericalBrushSelector.h>
#include <Tools/Brushes/CircularBrushROISelector.h>
#include <Tools/Brushes/SphericalBrushROISelector.h>
#include <Undo/ROIUndoCommand.h>

// Qt
#include <QDebug>
#include <QAction>

using namespace ESPINA;

//-----------------------------------------------------------------------------
ManualROITool::ManualROITool(ModelAdapterSPtr model,
                             ViewManagerSPtr  viewManager,
                             QUndoStack      *undoStack,
                             ROIToolsGroup   *toolGroup)
: ManualEditionTool{model, viewManager}
, m_undoStack      {undoStack}
, m_toolGroup      {toolGroup}
{

  disconnect(m_circularBrushSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
             this,                          SLOT(  drawStroke(Selector::Selection)));
  disconnect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
             this,                          SLOT(      selectorInUse(bool)));
  disconnect(m_circularBrushSelector.get(), SIGNAL(radiusChanged(int)),
             this,                          SLOT(  radiusChanged(int)));
  disconnect(m_circularBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
             this,                          SLOT(  drawingModeChanged(bool)));
  disconnect(m_sphericalBrushSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
             this,                           SLOT(  drawStroke(Selector::Selection)));
  disconnect(m_sphericalBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
             m_drawToolSelector,             SLOT(         setChecked(bool)));
  disconnect(m_sphericalBrushSelector.get(), SIGNAL(radiusChanged(int)),
             this,                           SLOT(  radiusChanged(int)));
  disconnect(m_sphericalBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
             this,                           SLOT(  drawingModeChanged(bool)));
  disconnect(m_drawToolSelector, SIGNAL(   triggered(QAction*)),
             this,               SLOT(changeSelector(QAction*)));
  disconnect(m_drawToolSelector, SIGNAL(actionCanceled()),
             this,               SLOT(   unsetSelector()));
  disconnect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
             this,               SLOT(  categoryChanged(CategoryAdapterSPtr)));

  delete m_drawToolSelector;

  m_drawToolSelector = new ActionSelector();

  // draw with a disc
  m_discTool = new QAction(QIcon(":/espina/voi_brush2D.svg"),
                           tr("Modify ROI drawing 2D discs"),
                           m_drawToolSelector);

  m_circularBrushSelector = CircularBrushROISelectorSPtr(new CircularBrushROISelector());
  connect(m_circularBrushSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,                          SLOT(  drawStroke(Selector::Selection)));
  connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_drawToolSelector,            SLOT(         setChecked(bool)));
  connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          this,                          SLOT(      selectorInUse(bool)));
  connect(m_circularBrushSelector.get(), SIGNAL(radiusChanged(int)),
          this,                          SLOT(  radiusChanged(int)));
  connect(m_circularBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
          this,                          SLOT(  drawingModeChanged(bool)));

  m_drawTools[m_discTool] = m_circularBrushSelector;
  m_drawToolSelector->addAction(m_discTool);


  // draw with a sphere
  m_sphereTool = new QAction(QIcon(":/espina/voi_brush3D.svg"),
                             tr("Modify ROI drawing 3D spheres"),
                             m_drawToolSelector);

  m_sphericalBrushSelector = SphericalBrushROISelectorSPtr(new SphericalBrushROISelector());
  connect(m_sphericalBrushSelector.get(), SIGNAL(itemsSelected(Selector::Selection)),
          this,                           SLOT(  drawStroke(Selector::Selection)));
  connect(m_sphericalBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_drawToolSelector,             SLOT(         setChecked(bool)));
  connect(m_sphericalBrushSelector.get(), SIGNAL(radiusChanged(int)),
          this,                           SLOT(  radiusChanged(int)));
  connect(m_sphericalBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
          this,                           SLOT(  drawingModeChanged(bool)));

  m_drawTools[m_sphereTool] = m_sphericalBrushSelector;
  m_drawToolSelector->addAction(m_sphereTool);

  // TODO: contour filter, tool y selector.

  // draw with contour
  //    QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
  //                                       tr("Modify segmentation drawing contour"),
  //                                       m_drawToolSelector);
  //    FilledContourSPtr contour(new FilledContour(m_model,
  //                                                m_undoStack,
  //                                                m_viewManager));
  //
  //    connect(contour.get(), SIGNAL(changeMode(Brush::BrushMode)),
  //            this, SLOT(changeContourMode(Brush::BrushMode)));
  //    connect(contour.get(), SIGNAL(stopDrawing()),
  //            this, SLOT(cancelDrawOperation()));
  //    connect(contour.get(), SIGNAL(startDrawing()),
  //            this, SLOT(startContourOperation()));
  //
  //    m_drawTools[contourTool] = contour;
  //    m_drawTools[contourTool] = SelectorSPtr(this);
  //    m_drawToolSelector->addAction(contourTool);

  m_drawToolSelector->setDefaultAction(m_discTool);
  connect(m_drawToolSelector, SIGNAL(   triggered(QAction*)),
          this,               SLOT(changeSelector(QAction*)));
  connect(m_drawToolSelector, SIGNAL(actionCanceled()),
          this,               SLOT(   unsetSelector()));
  connect(m_categorySelector, SIGNAL(categoryChanged(CategoryAdapterSPtr)),
          this,               SLOT(  categoryChanged(CategoryAdapterSPtr)));

  connect(m_viewManager.get(), SIGNAL(ROIChanged()),
          this,                SLOT(ROIChanged()));
}

//-----------------------------------------------------------------------------
ManualROITool::~ManualROITool()
{
}

//-----------------------------------------------------------------------------
void ManualROITool::ROIChanged()
{
  bool hasROI = (m_toolGroup->currentROI() != nullptr);

  auto disc = dynamic_cast<CircularBrushROISelector *>(m_circularBrushSelector.get());
  disc->setHasROI(hasROI);
  auto sphere = dynamic_cast<SphericalBrushROISelector *>(m_sphericalBrushSelector.get());
  sphere->setHasROI(hasROI);
}

//-----------------------------------------------------------------------------
void ManualROITool::changeSelector(QAction* action)
{
  Q_ASSERT(m_drawTools.keys().contains(action));

  setControlVisibility(true);

  m_currentSelector = m_drawTools[action];
  m_currentSelector->setBrushColor(Qt::yellow);
  m_currentSelector->setRadius(m_radiusWidget->value());
  updateReferenceItem(m_viewManager->selection());

  m_viewManager->setEventHandler(m_currentSelector);
}

//-----------------------------------------------------------------------------
void ManualROITool::selectorInUse(bool value)
{
  if (!value)
  {
    emit stopDrawing(m_currentSelector->referenceItem(), m_hasEnteredEraserMode);

    m_currentSelector = nullptr;
  }

  m_drawToolSelector->setChecked(value);
  setControlVisibility(value);
}

//-----------------------------------------------------------------------------
void ManualROITool::drawingModeChanged(bool isDrawing)
{
  QAction *actualAction = m_drawToolSelector->getCurrentAction();
  QIcon icon;

  if (m_discTool == actualAction)
  {
    if (isDrawing)
      icon = QIcon(":/espina/voi_brush2D.svg");
    else
      icon = QIcon(":/espina/voi_brush2D-erase.svg");
  }
  else
  {
    if (m_sphereTool == actualAction)
    {
      if (isDrawing)
        icon = QIcon(":/espina/voi_brush3D.svg");
      else
        icon = QIcon(":/espina/voi_brush3D-erase.svg");
    }
  }

  m_drawToolSelector->setIcon(icon);
}

//------------------------------------------------------------------------
void ManualROITool::drawStroke(Selector::Selection selection)
{
  if(m_toolGroup->currentROI() == nullptr)
    m_undoStack->beginMacro("Create Region Of Interest");
  else
    m_undoStack->beginMacro("Modify Region Of Interest");

  m_undoStack->push(new ModifyROIUndoCommand{m_toolGroup, selection.first().first});
  m_undoStack->endMacro();

  updateReferenceItem(m_viewManager->selection());
}

//-----------------------------------------------------------------------------
void ManualROITool::cancelROI()
{
  m_currentSelector->abortOperation();
}

//-----------------------------------------------------------------------------
void ManualROITool::updateReferenceItem(SelectionSPtr selection)
{
  m_currentSelector->setReferenceItem(m_viewManager->activeChannel());

  if(m_toolGroup->currentROI() != nullptr)
  {
    auto disk = dynamic_cast<CircularBrushROISelector *>(m_circularBrushSelector.get());
    disk->setHasROI(true);
    auto sphere = dynamic_cast<SphericalBrushROISelector *>(m_sphericalBrushSelector.get());
    sphere->setHasROI(true);

    m_currentSelector->setBrushImage(QImage());
  }
  else
  {
    m_currentSelector->setBrushImage(QImage(":/espina/add.svg"));
  }
}

//-----------------------------------------------------------------------------
void ManualROITool::setControlVisibility(bool value)
{
  if (m_showRadiusControls)
    m_radiusWidget->setVisible(value);

  if (m_showOpacityControls)
    m_opacityWidget->setVisible(value);
}
