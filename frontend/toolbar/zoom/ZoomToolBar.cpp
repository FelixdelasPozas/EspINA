/*
 * ZoomToolbar.cpp
 *
 *  Created on: Nov 14, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "toolbar/zoom/ZoomToolBar.h"
#include "common/gui/ViewManager.h"

// Qt
#include <QIcon>
#include <QAction>

//----------------------------------------------------------------------------
ZoomToolBar::ZoomToolBar(ViewManager *vm, QWidget* parent)
: QToolBar(parent)
, m_viewManager(vm)
, m_resetViews(NULL)
, m_zoomTool(NULL)
{
  setObjectName("ZoomToolBar");
  setWindowTitle("Zoom Tool Bar");

  m_resetViews = new QAction(QIcon(":zoom_reset.png"), tr("Reset view's cameras"),this);
  m_resetViews->setStatusTip(tr("Reset view's cameras"));
  connect(m_resetViews, SIGNAL(triggered()), this, SLOT(ResetViews()));
  addAction(m_resetViews);

  m_zoomTool = new QAction(QIcon(":zoom_selection.png"), tr("Zoom selection tool"), this);
  m_zoomTool->setStatusTip(tr("Zoom selection tool"));
  connect(m_zoomTool, SIGNAL(triggered()), this, SLOT(InitZoomTool()));
  addAction(m_zoomTool);
}

//----------------------------------------------------------------------------
ZoomToolBar::~ZoomToolBar()
{
  delete m_resetViews;
  delete m_zoomTool;
}

//----------------------------------------------------------------------------
void ZoomToolBar::ResetViews()
{
  m_viewManager->resetViewCameras();
  m_viewManager->updateViews();
}

//----------------------------------------------------------------------------
void ZoomToolBar::InitZoomTool()
{
  // TODO
}
