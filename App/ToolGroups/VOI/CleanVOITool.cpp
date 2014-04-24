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

#include "CleanVOITool.h"

// EspINA

// Qt
#include <QDebug>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
CleanVOITool::CleanVOITool(ModelAdapterSPtr model,
                           ViewManagerSPtr  viewManager)
: m_model      {model}
, m_viewManager{viewManager}
, m_cleanVOI   {new QAction(QIcon(":/espina/voi_clean.svg"), tr("Clean Volume Of Interest"), this)}
, m_enabled    {true}
{
  connect(m_viewManager.get(), SIGNAL(ROIChanged()),
          this,                SLOT(ROIChanged()));

  connect(m_cleanVOI, SIGNAL(triggered(bool)),
          this,       SLOT(cancelVOI()));

  ROIChanged();
}

//-----------------------------------------------------------------------------
CleanVOITool::~CleanVOITool()
{
  delete m_cleanVOI;
}

//-----------------------------------------------------------------------------
void CleanVOITool::setEnabled(bool value)
{
  if (m_enabled == value)
    return;

  m_cleanVOI->setEnabled(value);
  m_enabled = value;
}

//-----------------------------------------------------------------------------
bool CleanVOITool::enabled() const
{
  return m_enabled;
}

//-----------------------------------------------------------------------------
QList<QAction *> CleanVOITool::actions() const
{
  QList<QAction *> actions;

  actions << m_cleanVOI;

  return actions;
}

//-----------------------------------------------------------------------------
void CleanVOITool::cancelVOI()
{
  m_viewManager->setCurrentROI(nullptr);
}

//-----------------------------------------------------------------------------
void CleanVOITool::ROIChanged()
{
  setEnabled(m_viewManager->currentROI() != nullptr);
}
