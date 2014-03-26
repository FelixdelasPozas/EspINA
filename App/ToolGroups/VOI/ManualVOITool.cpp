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

// EspINA

// Qt
#include <QDebug>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
ManualVOITool::ManualVOITool(ModelAdapterSPtr model,
                                   ViewManagerSPtr  viewManager)
: m_model      (model)
, m_viewManager(viewManager)
, m_actionSelector(new ActionSelector())
{
  //RectangularVOISPtr voi(new RectangularVOI(m_model, m_viewManager));
  //m_vois[action] = voi;
  //connect(voi.get(), SIGNAL(voiDeactivated()),
  //        this, SLOT(cancelVOI()));


//   connect(m_applyVOI, SIGNAL(triggered(bool)),
//           this,       SLOT(changeVOI(QAction*)));
//   connect(m_applyVOI, SIGNAL(nCanceled()),
//           this,       SLOT(cancelVOI()));
  // draw with a disc
  m_circularBrushAction = new QAction(QIcon(":/espina/pencil2D.png"),
                                      tr("Modify VOI drawing 2D discs"),
                                      m_actionSelector);

  m_circularBrushSelector = CircularBrushSelectorSPtr(new CircularBrushSelector(m_viewManager));
  m_circularBrushSelector->setBrushImage(QImage());
  m_circularBrushSelector->setBrushColor(Qt::yellow);

//   connect(m_circularBrushSelector.get(), SIGNAL(stroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)),
//           this,                          SLOT(drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane)));
  connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
          m_actionSelector,              SLOT(setChecked(bool)));
//   connect(m_circularBrushSelector.get(), SIGNAL(eventHandlerInUse(bool)),
//           this,                          SLOT(selectorInUse(bool)));
//   connect(m_circularBrushSelector.get(), SIGNAL(radiusChanged(int)),
//           this,                          SLOT(radiusChanged(int)));
//   connect(m_circularBrushSelector.get(), SIGNAL(drawingModeChanged(bool)),
//           this,                          SLOT(drawingModeChanged(bool)));

  m_selectors[m_circularBrushAction] = m_circularBrushSelector;
  m_actionSelector->addAction(m_circularBrushAction);

  m_actionSelector->setDefaultAction(m_circularBrushAction);

  connect(m_actionSelector, SIGNAL(triggered(QAction*)),
          this,             SLOT(changeSelector(QAction*)));
  connect(m_actionSelector, SIGNAL(actionCanceled()),
          this,             SLOT(unsetSelector()));
}

//-----------------------------------------------------------------------------
ManualVOITool::~ManualVOITool()
{
  delete m_circularBrushAction;
}

//-----------------------------------------------------------------------------
void ManualVOITool::setEnabled(bool value)
{

}

//-----------------------------------------------------------------------------
bool ManualVOITool::enabled() const
{
  return true;
}

//-----------------------------------------------------------------------------
QList<QAction *> ManualVOITool::actions() const
{
  QList<QAction *> actions;

  actions << m_actionSelector;

  return actions;
}

//-----------------------------------------------------------------------------
void ManualVOITool::changeSelector(QAction* selectorAction)
{
  Q_ASSERT(m_selectors.contains(selectorAction));

  m_currentSelector = m_selectors[selectorAction];

  m_viewManager->setEventHandler(m_currentSelector);
}

//-----------------------------------------------------------------------------
void ManualVOITool::selectorInUse(bool inUse)
{
//   if (!inUse)
//   {
//     m_currentSelector = nullptr;
//     //emit stopDrawing();
//   }
//   else
//   {
//     if (inUse && m_viewManager->activeCategory() && m_viewManager->activeChannel())
//       m_currentSelector->initBrush();
//   }
}

//-----------------------------------------------------------------------------
void ManualVOITool::unsetSelector()
{
  m_viewManager->unsetEventHandler(m_currentSelector);
  m_currentSelector.reset();
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