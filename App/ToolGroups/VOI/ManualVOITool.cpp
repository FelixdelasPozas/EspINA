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

// EspINA

// Qt
#include <QDebug>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
ManualVOITool::ManualVOITool(ModelAdapterSPtr model,
                             ViewManagerSPtr  viewManager)
: ManualEditionTool(model, viewManager)
{
  showCategoryControls(false);
}

//-----------------------------------------------------------------------------
ManualVOITool::~ManualVOITool()
{
}

//-----------------------------------------------------------------------------
void ManualVOITool::changeSelector(QAction* action)
{
  Q_ASSERT(m_drawTools.keys().contains(action));

  m_currentSelector = m_drawTools[action];
  m_currentSelector->setBrushColor(Qt::yellow);
  m_currentSelector->initBrush();
  m_currentSelector->setRadius(m_radiusWidget->value());

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
  else
    m_currentSelector->initBrush();
}

//------------------------------------------------------------------------
void ManualVOITool::drawStroke(ViewItemAdapterPtr item, Selector::WorldRegion region, Nm radius, Plane plane)
{
  auto mask = m_currentSelector->voxelSelectionMask();
  emit stroke(mask);

  m_currentSelector->initBrush();
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
