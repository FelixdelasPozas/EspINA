/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012 Félix de las Pozas Álvarez <@>

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
, m_resetZoom(new ResetZoom())
, m_zoomArea(new ZoomArea(viewManager))
{
//   setObjectName("ViewToolBar");
// 
//   setWindowTitle(tr("Zoom Tool Bar"));
// 
//   m_resetViews = addAction(QIcon(":/espina/zoom_reset.png"),
//                            tr("Reset Camera"));
//   m_resetViews->setStatusTip(tr("Reset Camera"));
//   m_resetViews->setCheckable(false);
//   connect(m_resetViews, SIGNAL(triggered()),
//           this,         SLOT(resetViews()));
// 
//   m_zoomToolAction = addAction(QIcon(":/espina/zoom_selection.png"),
//                                tr("Zoom selection tool"));
//   m_zoomToolAction->setStatusTip(tr("Zoom selection tool"));
//   m_zoomToolAction->setCheckable(true);
//   connect(m_zoomToolAction, SIGNAL(triggered(bool)),
//           this,             SLOT(initViewTool(bool)));
}

//----------------------------------------------------------------------------
ViewTools::~ViewTools()
{
}

//----------------------------------------------------------------------------
void ViewTools::setEnabled(bool value)
{

}

//----------------------------------------------------------------------------
bool ViewTools::enabled() const
{

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
void ViewTools::initViewTool(bool value)
{
//   if(value)
//   {
//     m_viewManager->setActiveTool(m_zoomTool);
//     m_viewManager->setSelectionEnabled(false);
//   }
//   else
//   {
//     m_viewManager->unsetActiveTool(m_zoomTool);
//     m_viewManager->setSelectionEnabled(true);
//   }
}

//----------------------------------------------------------------------------
void ViewTools::resetViews()
{
  m_viewManager->resetViewCameras();
  m_viewManager->updateViews();
}


// //----------------------------------------------------------------------------
// void ViewTools::resetToolbar()
// {
//   if (m_zoomToolAction->isChecked())
//   {
//     m_zoomToolAction->setChecked(false);
//     initViewTool(false);
//   }
// }
