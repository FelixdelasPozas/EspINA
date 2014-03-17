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

#include "OrtogonalVOITool.h"

// EspINA

// Qt
#include <QDebug>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
OrtogonalVOITool::OrtogonalVOITool(ModelAdapterSPtr model,
                                   ViewManagerSPtr  viewManager)
: m_model      (model)
, m_viewManager(viewManager)
, m_applyVOI   (new QAction(QIcon(":/espina/voi_ortogonal.svg"), tr("Ortogonal Volume Of Interest"), this))
{
  //RectangularVOISPtr voi(new RectangularVOI(m_model, m_viewManager));
  //m_vois[action] = voi;
  //connect(voi.get(), SIGNAL(voiDeactivated()),
  //        this, SLOT(cancelVOI()));


//   connect(m_applyVOI, SIGNAL(triggered(bool)),
//           this,       SLOT(changeVOI(QAction*)));
//   connect(m_applyVOI, SIGNAL(nCanceled()),
//           this,       SLOT(cancelVOI()));
}

//-----------------------------------------------------------------------------
OrtogonalVOITool::~OrtogonalVOITool()
{
  delete m_applyVOI;
}

//-----------------------------------------------------------------------------
void OrtogonalVOITool::setEnabled(bool value)
{

}

//-----------------------------------------------------------------------------
bool OrtogonalVOITool::enabled() const
{
  return true;
}

//-----------------------------------------------------------------------------
QList<QAction *> OrtogonalVOITool::actions() const
{
  QList<QAction *> actions;

  actions << m_applyVOI;

  return actions;
}

//-----------------------------------------------------------------------------
void OrtogonalVOITool::changeVOI(QAction* action)
{
//   Q_ASSERT(m_vois.contains(action));
//   m_viewManager->setVOI(m_vois[action]);
}

//-----------------------------------------------------------------------------
void OrtogonalVOITool::cancelVOI()
{
//   m_voiSelector->cancel();
//   m_viewManager->unsetActiveVOI();
}