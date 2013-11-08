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
#include "ZoomTools.h"

// #include <Tools/Zoom/ZoomTool.h>

// Qt
#include <QIcon>
#include <QAction>
#include <QObject>
#include <QEvent>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomTools::ZoomTools(ViewManagerSPtr viewManager, QWidget* parent)
: ToolGroup(viewManager, QIcon(":/espina/zoom_reset.png"), tr("Zoom Tools"), parent)
//, m_zoomTool(new ZoomTool(m_viewManager))
{
//   setObjectName("ZoomToolBar");
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
//           this,             SLOT(initZoomTool(bool)));
}

//----------------------------------------------------------------------------
ZoomTools::~ZoomTools()
{
  // NOTE: Lo destruye Qt?
  delete m_resetViews;
  delete m_zoomToolAction;
}

//----------------------------------------------------------------------------
void ZoomTools::resetViews()
{
  m_viewManager->resetViewCameras();
  m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
bool ZoomTools::enabled() const
{

}

//----------------------------------------------------------------------------
SelectorSPtr ZoomTools::selector() const
{

}

//----------------------------------------------------------------------------
void ZoomTools::setActiveTool(ToolSPtr tool)
{

}

//----------------------------------------------------------------------------
void ZoomTools::setEnabled(bool value)
{

}

//----------------------------------------------------------------------------
void ZoomTools::setInUse(bool value)
{
  if (value)
  {
    m_viewManager->setToolGroup(this);
  } else
  {
    m_viewManager->unsetActiveToolGroup(this);
  }
}

//----------------------------------------------------------------------------
ToolSList ZoomTools::tools()
{

}

//----------------------------------------------------------------------------
void ZoomTools::initZoomTool(bool value)
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

// //----------------------------------------------------------------------------
// void ZoomTools::resetToolbar()
// {
//   if (m_zoomToolAction->isChecked())
//   {
//     m_zoomToolAction->setChecked(false);
//     initZoomTool(false);
//   }
// }
