/*
 * ZoomToolbar.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/zoom/ZoomToolBar.h"
#include "toolbar/zoom/ZoomTool.h"
#include "common/gui/ViewManager.h"

// Qt
#include <QIcon>
#include <QAction>
#include <QObject>
#include <QEvent>

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

  m_resetViews = new QAction(QIcon(":zoom_reset.png"), tr("Reset view's cameras"),this);
  m_resetViews->setStatusTip(tr("Reset views' cameras"));
  connect(m_resetViews, SIGNAL(triggered()), this, SLOT(ResetViews()));
  addAction(m_resetViews);

  m_zoomToolAction = new QAction(QIcon(":zoom_selection.png"), tr("Zoom selection tool"), this);
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
    if(m_zoomTool)
    {
      m_viewManager->unsetActiveTool(m_zoomTool);
      m_viewManager->setSelectionEnabled(true);
      delete m_zoomTool;
      m_zoomTool = NULL;
    }
}
