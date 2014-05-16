/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "ManualVOITool.h"

#include <GUI/Widgets/SliderAction.h>
#include <Undo/ROIUndoCommand.h>

// EspINA

// Qt
#include <QDebug>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
ManualVOITool::ManualVOITool(ModelAdapterSPtr model,
                             ViewManagerSPtr  viewManager,
                             QUndoStack      *undoStack)
: ManualEditionTool(model, viewManager)
, m_undoStack(undoStack)
{
  showCategoryControls(false);
  setPencil2DIcon(QIcon(":/espina/voi_brush2D.svg"));
  setPencil3DIcon(QIcon(":/espina/voi_brush3D.svg"));
  setPencil2DText(QString("Modify VOI drawing 2D discs"));
  setPencil3DText(QString("Modify VOI drawing 3D spheres"));
}

//-----------------------------------------------------------------------------
ManualVOITool::~ManualVOITool()
{
}

//-----------------------------------------------------------------------------
void ManualVOITool::changeSelector(QAction* action)
{
  Q_ASSERT(m_drawTools.keys().contains(action));
  if (m_showCategoryControls)
    m_categorySelector->setVisible(true);

  if (m_showRadiusControls)
    m_radiusWidget->setVisible(true);

  if (m_showOpacityControls)
    m_opacityWidget->setVisible(true);

  m_currentSelector = m_drawTools[action];
  m_currentSelector->setBrushColor(Qt::yellow);
  m_currentSelector->setRadius(m_radiusWidget->value());
  m_currentSelector->setReferenceItem(m_viewManager->activeChannel());

  m_viewManager->setEventHandler(m_currentSelector);
}

//-----------------------------------------------------------------------------
void ManualVOITool::selectorInUse(bool value)
{
  if (!value)
  {
    m_currentSelector = nullptr;
    emit stopDrawing();
  }
}

//------------------------------------------------------------------------
void ManualVOITool::drawStroke(Selector::Selection selection)
{
  if(m_viewManager->currentROI() == nullptr)
    m_undoStack->beginMacro("Create Region Of Interest");
  else
    m_undoStack->beginMacro("Modify Region Of Interest");

  m_undoStack->push(new ModifyROIUndoCommand{m_viewManager, selection.first().first});
  m_undoStack->endMacro();

  qDebug() << "draw stroke emit";
}

//-----------------------------------------------------------------------------
void ManualVOITool::changeVOI(QAction* action)
{
//   Q_ASSERT(m_vois.contains(action));
//   m_viewManager->setVOI(m_vois[action]);
}

//-----------------------------------------------------------------------------
void ManualVOITool::cancelVOI()
{
//   m_voiSelector->cancel();
//   m_viewManager->unsetActiveVOI();
}
