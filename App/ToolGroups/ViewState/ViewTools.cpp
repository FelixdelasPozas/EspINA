/*
    
    Copyright (C) 2014 Felix de las Pozas Alvarez <@>

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

// EspINA
#include "ViewTools.h"

// #include <Tools/Zoom/ViewTool.h>

// Qt
#include <QIcon>
#include <QAction>
#include <QObject>
#include <QEvent>

using namespace EspINA;

//----------------------------------------------------------------------------
ViewTools::ViewTools(ViewManagerSPtr viewManager, QWidget* parent)
: ToolGroup(viewManager, QIcon(":/espina/show_all.svg"), tr("View Tools"), parent)
, m_toggleSegmentations(new ToggleSegmentationsVisibility(viewManager))
, m_toggleCrosshair(new ToggleCrosshairVisibility(viewManager))
, m_resetZoom(new ResetZoom(viewManager))
, m_zoomArea(new ZoomArea(viewManager))
, m_segmentationsShortcut(new QShortcut(parent))
, m_crosshairShortcut(new QShortcut(parent))
{
  m_segmentationsShortcut->setKey(Qt::Key_Space);
  m_segmentationsShortcut->setContext(Qt::ApplicationShortcut);
  connect(m_segmentationsShortcut,SIGNAL(activated()),m_toggleSegmentations.get(),SLOT(shortcut()));

  m_crosshairShortcut->setKey(Qt::Key_C);
  m_crosshairShortcut->setContext(Qt::ApplicationShortcut);
  connect(m_crosshairShortcut,SIGNAL(activated()),m_toggleCrosshair.get(),SLOT(shortcut()));

  connect(parent, SIGNAL(abortOperation()), this, SLOT(abortOperation()), Qt::QueuedConnection);
  connect(parent, SIGNAL(analysisClosed()), this, SLOT(abortOperation()), Qt::QueuedConnection);
}

//----------------------------------------------------------------------------
ViewTools::~ViewTools()
{
  delete m_segmentationsShortcut;
  delete m_crosshairShortcut;
}

//----------------------------------------------------------------------------
void ViewTools::setEnabled(bool value)
{
  m_enabled = value;

  m_toggleCrosshair->setEnabled(value);
  m_toggleSegmentations->setEnabled(value);
  m_zoomArea->setEnabled(value);
  m_resetZoom->setEnabled(value);

  m_segmentationsShortcut->setEnabled(value);
  m_crosshairShortcut->setEnabled(value);
}

//----------------------------------------------------------------------------
bool ViewTools::enabled() const
{
  return m_enabled;
}

//----------------------------------------------------------------------------
ToolSList ViewTools::tools()
{
  ToolSList zoomTools;

  zoomTools << m_toggleSegmentations;
  zoomTools << m_toggleCrosshair;
  zoomTools << m_zoomArea;
  zoomTools << m_resetZoom;

  return zoomTools;
}

//----------------------------------------------------------------------------
void ViewTools::abortOperation()
{
  m_zoomArea->abortOperation();
}

