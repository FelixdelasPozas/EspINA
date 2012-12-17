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
#include "ZoomToolBar.h"
#include <Tools/Zoom/ZoomTool.h>
#include <GUI/ViewManager.h>

// Qt
#include <QIcon>
#include <QAction>
#include <QObject>
#include <QEvent>

using namespace EspINA;

//----------------------------------------------------------------------------
ZoomToolBar::ZoomToolBar(ViewManager *vm, QWidget* parent)
: QToolBar(parent)
, m_viewManager(vm)
, m_resetViews(NULL)
, m_zoomToolAction(NULL)
, m_zoomTool(NULL)
{
  setObjectName("ZoomToolBar");
  setWindowTitle("Zoom Tool Bar");

  m_resetViews = new QAction(QIcon(":/espina/zoom_reset.png"), tr("Reset view's cameras"),this);
  m_resetViews->setStatusTip(tr("Reset views' cameras"));
  m_resetViews->setCheckable(false);
  connect(m_resetViews, SIGNAL(triggered()), this, SLOT(ResetViews()));
  addAction(m_resetViews);

  m_zoomToolAction = new QAction(QIcon(":/espina/zoom_selection.png"), tr("Zoom selection tool"), this);
  m_zoomToolAction->setStatusTip(tr("Zoom selection tool"));
  m_zoomToolAction->setCheckable(true);
  connect(m_zoomToolAction, SIGNAL(triggered(bool)), this, SLOT(InitZoomTool(bool)));
  addAction(m_zoomToolAction);
}

//----------------------------------------------------------------------------
ZoomToolBar::~ZoomToolBar()
{
  delete m_resetViews;
  delete m_zoomToolAction;
}

//----------------------------------------------------------------------------
void ZoomToolBar::ResetViews()
{
  m_viewManager->resetViewCameras();
  m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
void ZoomToolBar::InitZoomTool(bool value)
{
  if(value)
  {
    if (m_zoomTool)
      return;

    m_zoomTool = new ZoomTool(m_viewManager);
    m_viewManager->setActiveTool(m_zoomTool);
    m_viewManager->setSelectionEnabled(false);
  }
  else
  {
    if(m_zoomTool)
    {
      m_viewManager->unsetActiveTool(m_zoomTool);
      m_viewManager->setSelectionEnabled(true);
      delete m_zoomTool;
      m_zoomTool = NULL;
    }
  }
}

//----------------------------------------------------------------------------
void ZoomToolBar::resetState()
{
  if (m_zoomToolAction->isChecked())
  {
    m_zoomToolAction->setChecked(false);
    InitZoomTool(false);
  }
}
