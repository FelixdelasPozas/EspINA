/*=========================================================================

   Program: Espina
   Module:    EspinaMainWindow.cpp

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "espinaMainWindow.h"
#include "ui_espinaMainWindow.h"
#include "sliceWidget.h"
#include "volumeWidget.h"

#include "pqHelpReaction.h"
#include "pqObjectInspectorWidget.h"
#include "pqParaViewBehaviors.h"
#include "pqParaViewMenuBuilders.h"
#include "pqLoadDataReaction.h"
#include "pqPipelineSource.h"
#include "vtkPVPlugin.h"

#include "pqTwoDRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqObjectBuilder.h"
#include "pqObjectInspectorWidget.h"
#include "pqDisplayPolicy.h"

#include <QDebug>

class EspinaMainWindow::pqInternals : public Ui::pqClientMainWindow
{
};

//-----------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow()
	: m_stack(NULL)
	, m_xy(NULL)
	, m_yz(NULL)
	, m_xz(NULL)
{
  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  // Setup default GUI layout.

  // Set up the dock window corners to give the vertical docks more room.
  this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  //Create File Menu
  buildFileMenu(*this->Internals->menu_File);
  
  //// Populate application menus with actions.
  //pqParaViewMenuBuilders::buildFileMenu(*this->Internals->menu_File);
  //pqParaViewMenuBuilders::buildEditMenu(*this->Internals->menu_Edit);

  //// Populate sources menu.
  //pqParaViewMenuBuilders::buildSourcesMenu(*this->Internals->menuSources, this);

  //// Populate filters menu.
  //pqParaViewMenuBuilders::buildFiltersMenu(*this->Internals->menuFilters, this);

  //// Populate Tools menu.
  //pqParaViewMenuBuilders::buildToolsMenu(*this->Internals->menuTools);

  //// setup the context menu for the pipeline browser.
  //pqParaViewMenuBuilders::buildPipelineBrowserContextMenu(
  //  *this->Internals->pipelineBrowser);

  //pqParaViewMenuBuilders::buildToolbars(*this);

  //// Setup the View menu. This must be setup after all toolbars and dockwidgets
  //// have been created.
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menu_View, *this);

  //// Setup the menu to show macros.
  //pqParaViewMenuBuilders::buildMacrosMenu(*this->Internals->menu_Macros);

  //// Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menu_Help);

  // Final step, define application behaviors. Since we want all ParaView
  // behaviors, we use this convenience method.
  new pqParaViewBehaviors(this, this);

  //Create ESPINA views
  m_xy = new SliceWidget();
  this->setCentralWidget(m_xy);
  m_yz = new SliceWidget();
  this->Internals->yzSliceDock->setWidget(m_yz);
  m_xz = new SliceWidget();
  this->Internals->xzSliceDock->setWidget(m_xz);
  m_3d = new VolumeWidget();
  this->Internals->volumeDock->setWidget(m_3d);
}


//-----------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
  delete this->Internals;
  delete m_xy;
  delete m_yz;
  delete m_xz;
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::setWorkingStack(pqPipelineSource *source)
{
	pqActiveObjects& activeObjects = pqActiveObjects::instance();
	if (m_stack) pqApplicationCore::instance()->getObjectBuilder()->destroy(m_stack);
	m_stack = source;
	activeObjects.setActiveSource(source);
	pqApplicationCore::instance()->getDisplayPolicy()->setRepresentationVisibility(source->getOutputPort(0),m_xy->getView(),true);
	pqApplicationCore::instance()->getDisplayPolicy()->setRepresentationVisibility(source->getOutputPort(0),m_yz->getView(),true);
	pqApplicationCore::instance()->getDisplayPolicy()->setRepresentationVisibility(source->getOutputPort(0),m_xz->getView(),true);
	pqApplicationCore::instance()->getDisplayPolicy()->setRepresentationVisibility(source->getOutputPort(0),m_3d->getView(),true);
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::buildFileMenu(QMenu &menu)
{
	QIcon icon = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
	QAction *openAction = new QAction(icon,tr("Open"),this);
	pqLoadDataReaction * loadReaction = new pqLoadDataReaction(openAction);
	QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
		this, SLOT(setWorkingStack(pqPipelineSource *)));
	menu.addAction(openAction);
}
