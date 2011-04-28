/*=========================================================================
 *
 *   Program: Espina
 *   Module:    EspinaMainWindow.cpp
 *
 *   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
 *   All rights reserved.
 *
 *   ParaView is a free software; you can redistribute it and/or modify it
 *   under the terms of the ParaView license version 1.2.
 *
 *   See License_v1.2.txt for the full ParaView license.
 *   A copy of this license can be obtained by contacting
 *   Kitware Inc.
 *   28 Corporate Drive
 *   Clifton Park, NY 12065
 *   USA
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ========================================================================*/
// ESPINA includes
#include "espinaMainWindow.h"
#include "ui_espinaMainWindow.h"
#include "espINAFactory.h"
#include "distance.h"
#include "unitExplorer.h"
#include "selectionManager.h"
#include "data/taxonomy.h"
#include "espina.h"

//ParaQ includes
#include "pqHelpReaction.h"
#include "pqObjectInspectorWidget.h"
#include "pqParaViewBehaviors.h"
#include "pqParaViewMenuBuilders.h"
#include "pqLoadDataReaction.h"
#include "pqSaveDataReaction.h"
#include "pqPipelineSource.h"
#include "vtkPVPlugin.h"
#include "pqOutputPort.h"
#include "pqServerManagerObserver.h"
#include "vtkSMOutputPort.h"
#include "vtkSMProperty.h"

#include "pqRenderView.h"
#include "pqTwoDRenderView.h"
#include "pqRepresentation.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqObjectBuilder.h"
#include "pqObjectInspectorWidget.h"
#include "pqDisplayPolicy.h"
#include <pqFileDialog.h>

//VTK Includes
//QT includes
#include <QFileDialog>

//Debug includes
#include <QDebug>
#include <iostream>
#include <QPushButton>
#include <pqServerResources.h>

#include <taxonomyProxy.h>
#include <sampleProxy.h>
#include "sliceView.h"
#include <QMouseEvent>
#include <QStringListModel>
#include <QWidgetAction>
#include "qTreeComboBox.h"
#include <cache/cachedObjectBuilder.h>
#include "Crosshairs.h"
#include <pqServerManagerModel.h>
#include <pqServerDisconnectReaction.h>
#include "SegmentationExplorer.h"

QString FILTERS("Trace Files (*.trace)");
QString SEG_FILTERS("Seg Files (*.seg)");
QString DIRECTORY("");

class EspinaMainWindow::pqInternals : public Ui::pqClientMainWindow
{
};

//-----------------------------------------------------------------------------
EspinaMainWindow::EspinaMainWindow()
    : m_xy(NULL)
    , m_yz(NULL)
    , m_xz(NULL)
    , m_3d(NULL)
    , m_unit(NM)
    , m_selectionManager(NULL)
{
  m_espina = EspINA::instance();
  
  this->Internals = new pqInternals();
  this->Internals->setupUi(this);

  // Set up the dock window corners to give the vertical docks more room.
  this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // BUILDE ESPINA MENUS
  //Create File Menu
  buildFileMenu(*this->Internals->menu_File);

  //// Populate application menus with actions.
  pqParaViewMenuBuilders::buildFileMenu(*this->Internals->menu_File);

  //// Populate filters menu.
  //pqParaViewMenuBuilders::buildFiltersMenu(*this->Internals->menuTools, this);

  //// Populate Tools menu.
  pqParaViewMenuBuilders::buildToolsMenu(*this->Internals->menuTools);

  //// setup the context menu for the pipeline browser.
  //pqParaViewMenuBuilders::buildPipelineBrowserContextMenu(
  //  *this->Internals->pipelineBrowser);

  //// Setup the View menu. This must be setup after all toolbars and dockwidgets
  //// have been created.
  m_unitExplorer = new UnitExplorer();
  connect(this->Internals->actionUnits, SIGNAL(triggered()), m_unitExplorer, SLOT(show()));
  SegmentationExplorer *segExpl = new SegmentationExplorer();
  connect(this->Internals->actionSegmentationExplorer, SIGNAL(triggered()), segExpl, SLOT(show()));
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menu_View, *this);

  //// Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menu_Help);

  // ParaView Server
  pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();

  
  // BUILD ESPINA INTERNALS
  

  // Segementation Grouping Proxies
  TaxonomyProxy *taxProxy = new TaxonomyProxy();
  taxProxy->setSourceModel(m_espina);
  SampleProxy *sampleProxy = new SampleProxy();
  sampleProxy->setSourceModel(m_espina);

  m_groupingName << "None" << "Taxonomy" << "Sample";
  m_groupingModel << m_espina << taxProxy << sampleProxy;
  m_groupingRoot << m_espina->segmentationRoot()
  << taxProxy->mapFromSource(m_espina->taxonomyRoot())
  << sampleProxy->mapFromSource(m_espina->sampleRoot());

  // Group by List
  connect(this->Internals->groupList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setGroupView(int)));
  QStringListModel *groupListModel = new QStringListModel(m_groupingName);
  this->Internals->groupList->setModel(groupListModel);
  this->Internals->groupList->setCurrentIndex(1);
  
  // Segmentation Manager Panel
  this->Internals->segmentationView->installEventFilter(this);
  connect(this->Internals->deleteSegmentation, SIGNAL(clicked()),
          this, SLOT(deleteSegmentations()));
  
  // Taxonomy Selection List
  QComboBox *taxonomySelector = new QComboBox(this);
  ///QTreeComboBox *treeCombo = new QTreeComboBox(this);
  QTreeView *taxonomyView = new QTreeView(this);
  taxonomyView->setHeaderHidden(true);
  taxonomySelector->setView(taxonomyView); //Brutal!
  taxonomySelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  taxonomySelector->setMinimumWidth(160);
  taxonomySelector->setModel(m_espina);
  ///treeCombo->setModel(m_espina);
  ///treeCombo->setRootModelIndex(m_espina->taxonomyRoot());
  ///treeCombo->setCurrentIndex(0);
  ///treeCombo->setMinimumWidth(200);
  taxonomySelector->setRootModelIndex(m_espina->taxonomyRoot());
  taxonomyView->expandAll();;
  connect(taxonomySelector, SIGNAL(currentIndexChanged(QString)),
          m_espina, SLOT(setUserDefindedTaxonomy(const QString&)));

  connect(pqApplicationCore::instance()->getObjectBuilder(),
            SIGNAL(proxyCreated (pqProxy *)),
            m_espina,
            SLOT(onProxyCreated(pqProxy*)));
  taxonomySelector->setCurrentIndex(0);
  this->Internals->toolBar->addWidget(taxonomySelector);
  
  // Final step, define application behaviors. Since we want all ParaView
  // behaviors, we use this convenience method.
  new pqParaViewBehaviors(this, this);
  
  std::cout << "Creating Views" << std::endl;

  // Label Editor
  this->Internals->taxonomyView->setModel(m_espina);
  this->Internals->taxonomyView->setRootIndex(m_espina->taxonomyRoot());
  
  //Selection Manager
  m_selectionManager = SelectionManager::singleton();

  //Create ESPINA VIEWS
  m_xy = new SliceView();
  m_xy->setPlane(SliceView::SLICE_PLANE_XY);
  m_xy->setModel(sampleProxy);
  m_xy->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)), m_xy, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), m_xy, SLOT(disconnectFromServer()));
  connect(m_xy, SIGNAL(pointSelected(const Point)), m_selectionManager, SLOT(pointSelected(const Point)));
  this->setCentralWidget(m_xy);
  
#if 1
  
  m_yz = new SliceView();
  m_yz->setPlane(SliceView::SLICE_PLANE_YZ);
  m_yz->setModel(sampleProxy);
  m_yz->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)), 
	  m_yz, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), 
	  m_yz, SLOT(disconnectFromServer()));
  connect(m_yz, SIGNAL(pointSelected(const Point)), 
	  m_selectionManager, SLOT(pointSelected(const Point)));
  this->Internals->yzSliceDock->setWidget(m_yz);

 
  m_xz = new SliceView();
  m_xz->setPlane(SliceView::SLICE_PLANE_XZ);
  m_xz->setModel(sampleProxy);
  m_xz->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)), 
	  m_xz, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), 
	  m_xz, SLOT(disconnectFromServer()));
  this->Internals->xzSliceDock->setWidget(m_xz);
  
#endif
  
  Crosshairs *cross = new Crosshairs();
  cross->addPlane(0,m_xy->output());
  cross->addPlane(1,m_yz->output());
  cross->addPlane(2,m_xz->output());
  connect(m_xy,SIGNAL(sliceChanged()),cross,SLOT(update()));
  connect(m_yz,SIGNAL(sliceChanged()),cross,SLOT(update()));
  connect(m_xz,SIGNAL(sliceChanged()),cross,SLOT(update()));

  m_3d = EspINAFactory::instance()->CreateVolumeView();
  m_3d->setModel(sampleProxy);
  m_3d->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)),
	  m_3d, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), 
	  m_3d, SLOT(disconnectFromServer()));
  m_3d->addWidget(cross);
  this->Internals->volumeDock->setWidget(m_3d);

  // Setup default GUI layout.
  connect(this->Internals->toggleVisibility, SIGNAL(toggled(bool)), 
	  this, SLOT(toggleVisibility(bool)));
  pqServerDisconnectReaction::disconnectFromServer();
  // m_3d->setSelectionModel(this->Internals->taxonomyView->selectionModel());

  // Final step, define application behaviors. Since we want all ParaView
  // behaviors, we use this convenience method.
  new pqParaViewBehaviors(this, this);

  // Debug load stack
  QMetaObject::invokeMethod(this, "autoLoadStack", Qt::QueuedConnection);

}


//-----------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
  //delete this->Internals;
  //delete m_xy;
  //delete m_yz;
  //delete m_xz;
  // delete m_3d;
}

// //-----------------------------------------------------------------------------
// void EspinaMainWindow::loadData(pqPipelineSource *source)//TODO Delete
// {
//   
// //   pqApplicationCore* core = pqApplicationCore::instance();
// //   QString filePath = core->serverResources().list().first().path();
// // 
// //   m_espina->loadFile(filePath, core->getActiveServer());
//   m_espina->loadSource(source);
// }

//-----------------------------------------------------------------------------
// void EspinaMainWindow::loadFile()
// {
//   // GUI
//   pqServer* server = pqApplicationCore::instance()->getActiveServer();
//   pqFileDialog fileDialog(server, this, tr("Import"), "", FILTERS);
//   fileDialog.setFileMode(pqFileDialog::ExistingFile);
//   if( fileDialog.exec() == QDialog::Accepted )
//   {
//     m_espina->loadFile( fileDialog.getSelectedFiles()[0], server );
//   }
// }

//-----------------------------------------------------------------------------
void EspinaMainWindow::importFile()
{
  // GUI
  //TODO .pvd .mha ....
  pqFileDialog fileDialog(NULL, this, tr("Import"), "", SEG_FILTERS);
  fileDialog.setFileMode(pqFileDialog::ExistingFile);
  if( fileDialog.exec() == QDialog::Accepted )
  {
    m_espina->loadFile( fileDialog.getSelectedFiles()[0] );
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::saveFile()
{
  // GUI
  pqServer* server = pqApplicationCore::instance()->getActiveServer();
  pqFileDialog fileDialog(server, this, tr("Save Trace"), "", FILTERS);
  fileDialog.setFileMode(pqFileDialog::AnyFile);
  if( fileDialog.exec() == QDialog::Accepted )
  {
    //qDebug() << "Destination file " << fileDialog.getSelectedFiles()[0];
    m_espina->saveFile(fileDialog.getSelectedFiles()[0], server);
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::exportFile()
{
  // GUI
  pqFileDialog fileDialog(NULL, this, tr("Export"), "", SEG_FILTERS);
  fileDialog.setFileMode(pqFileDialog::AnyFile);
  if( fileDialog.exec() == QDialog::Accepted )
  {
    m_espina->saveFile( fileDialog.getSelectedFiles()[0] );
  }
}

// //-----------------------------------------------------------------------------
// void EspinaMainWindow::fileDialog(bool server, QString& title)
// {
//   pqFileDialog fileDialog(pqActiveObjects::instance().activeServer(), this,
//                           tr("Save Trace"), "", tr("Trace Files (*.trace)"));
// }

//-----------------------------------------------------------------------------
void EspinaMainWindow::toggleVisibility(bool visible)
{
  this->Internals->toggleVisibility->setIcon(
    visible ? QIcon(":/espina/show_all.svg") : QIcon(":/espina/hide_all.svg")
  );
  m_xy->showSegmentations(visible);
  m_yz->showSegmentations(visible);
  m_xz->showSegmentations(visible);
}

//-----------------------------------------------------------------------------
bool EspinaMainWindow::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == this->Internals->segmentationView)
  {
    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Delete
          || keyEvent->key() == Qt::Key_Backspace)
      {
        deleteSegmentations();
      }
    }
  }
  // Pass the event on to the parent class
  return QMainWindow::eventFilter(obj, event);
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::setGroupView(int idx)
{
  if (idx < m_groupingModel.size())
  {
    this->Internals->segmentationView->setModel(m_groupingModel[idx]);
    this->Internals->segmentationView->setRootIndex(m_groupingRoot[idx]);
  }
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::deleteSegmentations()
{
  QItemSelectionModel *selection = this->Internals->segmentationView->selectionModel();
  foreach(QModelIndex index, selection->selectedIndexes())
  {
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    //TODO: Handle segmentation and taxonomy deletions differently
    if (seg)
      m_espina->removeSegmentation(seg);
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::autoLoadStack()
{
  QString filePath(getenv("ESPINA_FILE"));
  if( filePath.size() > 0 )
  {
    // Import
    m_espina->loadFile(filePath);
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::buildFileMenu(QMenu &menu)
{
  pqServer* server = pqApplicationCore::instance()->getActiveServer();
  QIcon iconOpen = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
 
  QAction *action = new QAction(iconOpen,tr("Open - ParaView mode"),this);
  pqLoadDataReaction * loadReaction = new pqLoadDataReaction(action);
  QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
		    m_espina, SLOT( loadSource(pqPipelineSource *)));
//   QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( loadFile()));
  menu.addAction(action);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  action = new QAction(iconSave, tr("Save - ParaView mode"), this);
//  pqSaveDataReaction* saveReaction = new pqSaveDataReaction(action);
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(saveFile()));
  menu.addAction(action);

  /* Import Trace from localhost  */
  action = new QAction(tr("Import"),this);
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( importFile()));
  //QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( fileDialog(NULL, "Import")));
  menu.addAction(action);

  /* Export Trace to localhost */
  action = new QAction(tr("Export"),this);
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( exportFile()) );
  menu.addAction(action);
}
