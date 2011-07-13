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

#include "espina_debug.h"

#include "espINAFactory.h"
#include "distance.h"
#include "selectionManager.h"
#include "data/taxonomy.h"
#include "espina.h"
#include "SegmentationExplorer.h"
#include "meshRenderer.h"
#include "volumetricRenderer.h"
#include "colorExtension.h"
#include "meshExtension.h"
#include "volumetricExtension.h"
#include "sampleDelegate.h"
#include "segmentationEditor.h"

//ParaQ includes
#include <pqParaViewBehaviors.h>
#include <pqParaViewMenuBuilders.h>
#include <pqLoadDataReaction.h>
#include <pqSaveDataReaction.h>
#include <vtkPVPlugin.h>
#include <pqServerManagerObserver.h>
#include <vtkSMOutputPort.h>
#include <vtkSMReaderFactory.h>
#include <vtkSMProxyManager.h>
#include <pqCoreUtilities.h>
#include <pqServer.h>
#include <pqSetName.h>

#include <pqApplicationCore.h>
#include <pqActiveObjects.h>
#include <pqObjectBuilder.h>
#include <pqFileDialog.h>

//VTK Includes
//QT includes

//Debug includes
#include <iostream>
#include <QPushButton>
#include <pqServerResources.h>
#include <QMessageBox>
#include <QColorDialog>
#include <QSignalMapper>

#include <taxonomyProxy.h>
#include <sampleProxy.h>
#include "ui/sliceView.h"
#include <QMouseEvent>
#include <QStringListModel>
#include <QWidgetAction>
#include "qTreeComboBox.h"
#include <cache/cachedObjectBuilder.h>
#include <pqServerManagerModel.h>
#include <pqServerDisconnectReaction.h>
#include <labelMapExtension.h>
#include <crosshairExtension.h>
#include <spatialExtension.h>
#include <crosshairRenderer.h>
#include <pqManagePluginsReaction.h>
#include <pqQtMessageHandlerBehavior.h>

const QString FILTERS("Trace Files (*.trace)");
const QString SEG_FILTERS("Seg Files (*.seg)");
QString DIRECTORY("");

#define XY_VIEW 1
#define YZ_VIEW 1
#define XZ_VIEW 1
#define VOL_VIEW 1

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
    , m_selectionManager(NULL) //TODO: Revise if deprecated
    , m_lastTaxonomyId(0)
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

#if DEBUG_GUI
  //// Populate application menus with actions.
  pqParaViewMenuBuilders::buildFileMenu(*this->Internals->menu_File);
  
  //// Populate filters menu.
  //pqParaViewMenuBuilders::buildFiltersMenu(*this->Internals->menuTools, this);

  //// Populate Tools menu.
  pqParaViewMenuBuilders::buildToolsMenu(*this->Internals->menuTools);
#else
  new pqManagePluginsReaction(this->Internals->menuTools->addAction("Manage Plugins") << pqSetName("actionManage_Plugins"));
#endif

  //// setup the context menu for the pipeline browser.
  //pqParaViewMenuBuilders::buildPipelineBrowserContextMenu(
  //  *this->Internals->pipelineBrowser);

  //// Setup the View menu. This must be setup after all toolbars and dockwidgets
  //// have been created.
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menu_View, *this);

  //// Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menu_Help);
  
  // ParaView Server
  pqServerManagerObserver *server = pqApplicationCore::instance()->getServerManagerObserver();

  
  MeshRenderer *mesh = new MeshRenderer();
  EspINAFactory::instance()->addViewWidget(mesh);
  VolumetricRenderer *volumetric = new VolumetricRenderer();
  EspINAFactory::instance()->addViewWidget(volumetric);
  CrosshairRenderer *crosshairs = new CrosshairRenderer();
  EspINAFactory::instance()->addViewWidget(crosshairs);

  SpatialExtension::SampleExtension sampleSpatialExt;
  EspINAFactory::instance()->addSampleExtension(&sampleSpatialExt);
//   ColorExtension::SegmentationExtension segColorExt;
//   EspINAFactory::instance()->addSegmentationExtension(&segColorExt);
//   ColorExtension::SampleExtension sampleColorExt;
//   EspINAFactory::instance()->addSampleExtension(&sampleColorExt);
  LabelMapExtension::SampleExtension sampleLabelMapExt(this->Internals->toggleVisibility);
  EspINAFactory::instance()->addSampleExtension(&sampleLabelMapExt);
  CrosshairExtension CrossExt;
  EspINAFactory::instance()->addSampleExtension(&CrossExt);
  MeshExtension meshExt;
  EspINAFactory::instance()->addSegmentationExtension(&meshExt);
  VolumetricExtension volExt;
  EspINAFactory::instance()->addSegmentationExtension(&volExt);
  
  
  //! BUILD ESPINA INTERNALS

  // Segementation Grouping Proxies
  TaxonomyProxy *taxProxy = new TaxonomyProxy();
  taxProxy->setSourceModel(m_espina);
  sampleProxy = new SampleProxy();
  sampleProxy->setSourceModel(m_espina);
  

  m_groupingName << "None" << "Taxonomy" << "Sample";
  m_groupingModel << m_espina << taxProxy << sampleProxy;
  m_groupingRoot << m_espina->segmentationRoot()
  << taxProxy->mapFromSource(m_espina->taxonomyRoot())
  << sampleProxy->mapFromSource(m_espina->sampleRoot());
  
#if DEBUG_GUI
  this->Internals->modelo->setModel(m_espina);
  this->Internals->taxonomias->setModel(taxProxy);
  this->Internals->samples->setModel(sampleProxy);
#endif

  // Group by List
  connect(this->Internals->groupList, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setGroupView(int)));
  QStringListModel *groupListModel = new QStringListModel(m_groupingName);
  this->Internals->groupList->setModel(groupListModel);
  this->Internals->groupList->setCurrentIndex(1);
  
  // Segmentation Manager Panel
  SegmentationEditor *segEditor = new SegmentationEditor();
  this->Internals->segmentationView->setItemDelegate(segEditor);
  this->Internals->segmentationView->installEventFilter(this);
  connect(this->Internals->deleteSegmentation, SIGNAL(clicked()),
          this, SLOT(deleteSegmentations()));
  
  // User selected Taxonomy Selection List
  m_taxonomySelector = new QComboBox(this);
  ///QTreeComboBox *treeCombo = new QTreeComboBox(this);
  m_taxonomyView = new QTreeView(this);
  m_taxonomyView->setHeaderHidden(true);
  m_taxonomySelector->setView(m_taxonomyView); //Brutal!
  m_taxonomySelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_taxonomySelector->setMinimumWidth(160);
  m_taxonomySelector->setModel(m_espina);
  ///treeCombo->setModel(m_espina);
  ///treeCombo->setRootModelIndex(m_espina->taxonomyRoot());
  ///treeCombo->setCurrentIndex(0);
  ///treeCombo->setMinimumWidth(200);
  m_taxonomySelector->setRootModelIndex(m_espina->taxonomyRoot());
  connect(m_taxonomySelector, SIGNAL(currentIndexChanged(QString)),
          m_espina, SLOT(setUserDefindedTaxonomy(const QString&)));
  m_taxonomySelector->setCurrentIndex(0);
  this->Internals->toolBar->addWidget(m_taxonomySelector);

  connect(m_espina, SIGNAL(resetTaxonomy()),
          this, SLOT(resetTaxonomy()));
  
#if DEBUG_GUI
  connect(pqApplicationCore::instance()->getObjectBuilder(),
            SIGNAL(proxyCreated (pqProxy *)),
            m_espina,
            SLOT(onProxyCreated(pqProxy*)));
  connect(pqApplicationCore::instance()->getObjectBuilder(),
	  SIGNAL(destroying(pqProxy*)),
	  m_espina,
	  SLOT(destroyingProxy(pqProxy*)));
#endif
  
  tabifyDockWidget(this->Internals->segmentationEditor,this->Internals->sampleEditor);
  tabifyDockWidget(this->Internals->segmentationEditor,this->Internals->taxonomyEditor);
  this->Internals->segmentationEditor->raise();
  
  // Taxonomy Editor
  this->Internals->taxonomyView->setModel(m_espina);
  this->Internals->taxonomyView->setRootIndex(m_espina->taxonomyRoot());
  connect(this->Internals->addTaxonomy,SIGNAL(clicked()),this,SLOT(addTaxonomyElement()));
  connect(this->Internals->addTaxonomyChild,SIGNAL(clicked()),this,SLOT(addTaxonomyChildElement()));
  connect(this->Internals->removeTaxonomy,SIGNAL(clicked()),this,SLOT(removeTaxonomyElement()));
  connect(this->Internals->taxonomyColorSelector,SIGNAL(clicked()),this,SLOT(changeTaxonomyColor()));

  // Sample Explorer
  SampleDelegate *sampleDelegate = new SampleDelegate();
  this->Internals->sampleView->setItemDelegate(sampleDelegate);
  this->Internals->sampleView->setModel(m_espina);
  this->Internals->sampleView->setRootIndex(m_espina->sampleRoot());
  
  //connect(this->Internals->makeActiveSample,SIGNAL(clicked()),this,SLOT(focusOnSample()));
  
  //Selection Manager
  m_selectionManager = SelectionManager::instance();
  
#if XY_VIEW
  //Create ESPINA VIEWS
  m_xy = new SliceView();
  m_xy->setPlane(VIEW_PLANE_XY);
  m_xy->setModel(sampleProxy);
  connect(server, SIGNAL(connectionCreated(vtkIdType)), m_xy, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), m_xy, SLOT(disconnectFromServer()));
  connect(this->Internals->toggleVisibility, SIGNAL(toggled(bool)),m_xy, SLOT(showSegmentations(bool)));
  this->setCentralWidget(m_xy);
#endif
  

#if YZ_VIEW
  m_yz = new SliceView();
  m_yz->setPlane(VIEW_PLANE_YZ);
  m_yz->setModel(sampleProxy);
  m_yz->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)), 
	  m_yz, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), 
	  m_yz, SLOT(disconnectFromServer()));
  connect(this->Internals->toggleVisibility, SIGNAL(toggled(bool)),m_yz, SLOT(showSegmentations(bool)));
  this->Internals->yzSliceDock->setWidget(m_yz);
#endif

#if XZ_VIEW
  m_xz = new SliceView();
  m_xz->setPlane(VIEW_PLANE_XZ);
  m_xz->setModel(sampleProxy);
  m_xz->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)), 
	  m_xz, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), 
	  m_xz, SLOT(disconnectFromServer()));
  connect(this->Internals->toggleVisibility, SIGNAL(toggled(bool)),m_xz, SLOT(showSegmentations(bool)));
  this->Internals->xzSliceDock->setWidget(m_xz);
#endif
  
#if VOL_VIEW
  m_3d = EspINAFactory::instance()->CreateVolumeView();
  m_3d->setModel(sampleProxy);
  m_3d->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleRoot()));
  connect(server, SIGNAL(connectionCreated(vtkIdType)),
	  m_3d, SLOT(connectToServer()));
  connect(server, SIGNAL(connectionClosed(vtkIdType)), 
	  m_3d, SLOT(disconnectFromServer()));
  //m_3d->addWidget(cross);
  this->Internals->volumeDock->setWidget(m_3d);
#endif //VOL_VIEW
  
  resetTaxonomy();

  // Setup default GUI layout.
  connect(this->Internals->toggleVisibility, SIGNAL(toggled(bool)), 
	  this, SLOT(toggleVisibility(bool)));
  //pqServerDisconnectReaction::disconnectFromServer();
  
  // m_3d->setSelectionModel(this->Internals->taxonomyView->selectionModel());

  // Final step, define application behaviors. Since we want all ParaView
  // behaviors, we use this convenience method.
  new pqParaViewBehaviors(this, this);

#if DEBUG_GUI
  QMetaObject::invokeMethod(this, "autoLoadStack", Qt::QueuedConnection);
#endif// DEBUG_GUI

#if DEBUG_GUI
  this->Internals->pipelineBrowserDock->setVisible(true);
  this->Internals->statisticsDock->setVisible(true);
  this->Internals->modelsDock->setVisible(true);
#else
  pqApplicationCore::instance()->disableOutputWindow();
  this->Internals->pipelineBrowserDock->setVisible(false);
  this->Internals->statisticsDock->setVisible(false);
  this->Internals->modelsDock->setVisible(false);
#endif
}


//-----------------------------------------------------------------------------
EspinaMainWindow::~EspinaMainWindow()
{
  delete sampleProxy;
  delete Internals;
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
void EspinaMainWindow::loadFile(QString method)
{
  // GUI
//   pqServer* server = pqApplicationCore::instance()->getActiveServer();
//   pqFileDialog fileDialog(server, this, tr("Import"), "", FILTERS);
//   fileDialog.setFileMode(pqFileDialog::ExistingFile);
//   if( fileDialog.exec() == QDialog::Accepted )
//   {
//     m_espina->loadFile( fileDialog.getSelectedFiles()[0], server );
//   }


  pqServer* server = pqActiveObjects::instance().activeServer();
  vtkSMReaderFactory* readerFactory =
    vtkSMProxyManager::GetProxyManager()->GetReaderFactory();
  QString filters = readerFactory->GetSupportedFileTypes(
    server->GetConnectionID());
  if (!filters.isEmpty())
    {
    filters += ";;";
    }
  filters += "All files (*)";
  pqFileDialog fileDialog(server,
    pqCoreUtilities::mainWidget(),
    tr("Open File:"), QString(), filters);
  fileDialog.setObjectName("FileOpenDialog");
  fileDialog.setFileMode(pqFileDialog::ExistingFiles);
  if (fileDialog.exec() == QDialog::Accepted)
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_espina->loadFile(fileDialog.getSelectedFiles()[0], method);
    QApplication::restoreOverrideCursor();
  }
#if XY_VIEW
  m_xy->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleIndex(m_espina->activeSample())));
#endif
#if YZ_VIEW
  m_yz->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleIndex(m_espina->activeSample())));
#endif
#if XZ_VIEW
  m_xz->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleIndex(m_espina->activeSample())));
#endif
#if VOL_VIEW
  m_3d->setRootIndex(sampleProxy->mapFromSource(m_espina->sampleIndex(m_espina->activeSample())));
#endif
  this->Internals->segmentationView->expandAll();
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::importFile()
{
  // GUI
  //TODO .pvd .mha ....
  pqFileDialog fileDialog(NULL, this, tr("Import"), "", SEG_FILTERS);
  fileDialog.setFileMode(pqFileDialog::ExistingFile);
  if( fileDialog.exec() == QDialog::Accepted )
  {
    //m_espina->loadFile( fileDialog.getSelectedFiles()[0] );
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::saveFile()
{
  // GUI
  pqServer* server = pqApplicationCore::instance()->getActiveServer();
  pqFileDialog fileDialog(server, this, tr("Save Trace"), "", SEG_FILTERS);
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
void EspinaMainWindow::addTaxonomyElement()
{
  try
  {
    IModelItem *taxItem = static_cast<IModelItem *>(this->Internals->taxonomyView->currentIndex().internalPointer());
    TaxonomyNode *taxNode = dynamic_cast<TaxonomyNode *>(taxItem);
    m_espina->addTaxonomy("Undefined",m_espina->taxonomyParent(taxNode)->getName());
  }catch (...)
  {
    QMessageBox box;
    box.setText("New taxonomy already undefined. Please, define it before creating a new one");
    box.exec();
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::addTaxonomyChildElement()
{ 
  try
  {
    IModelItem *parentItem = static_cast<IModelItem *>(this->Internals->taxonomyView->currentIndex().internalPointer());
    TaxonomyNode *parent = dynamic_cast<TaxonomyNode *>(parentItem);
    if( parent )
      m_espina->addTaxonomy("Undefined",parent->getName());
  }catch (...)
  {
    QMessageBox box;
    box.setText("New taxonomy already undefined. Please, define it before creating a new one");
    box.exec();
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::removeTaxonomyElement()
{
  
  IModelItem *currentItem = static_cast<IModelItem *>(this->Internals->taxonomyView->currentIndex().internalPointer());
  TaxonomyNode *currentNode = dynamic_cast<TaxonomyNode *>(currentItem);
  if( currentNode )
    m_espina->removeTaxonomy(currentNode->getName());
}


//-----------------------------------------------------------------------------
void EspinaMainWindow::changeTaxonomyColor()
{

  //m_espina->clear();
  //return;
  QColorDialog colorSelector;
  if( colorSelector.exec() == QDialog::Accepted)
    m_espina->setData(this->Internals->taxonomyView->currentIndex(),colorSelector.selectedColor(),Qt::DecorationRole);
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::resetTaxonomy()
{
  m_taxonomyView->expandAll();
  m_taxonomySelector->setCurrentIndex(0);
  this->Internals->taxonomyView->expandAll();
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::focusOnSample()
{
  
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::toggleVisibility(bool visible)
{
  this->Internals->toggleVisibility->setIcon(
    visible ? QIcon(":/espina/show_all.svg") : QIcon(":/espina/hide_all.svg")
  );
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
     this->Internals->segmentationView->expandAll();
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
    {
      m_espina->removeSegmentation(seg);
    }
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::autoLoadStack()
{
  QString filePath(getenv("ESPINA_FILE"));
  if( filePath.size() > 0 )
  {
    // Import
    m_espina->loadFile(filePath, "open");
  }
}

//-----------------------------------------------------------------------------
void EspinaMainWindow::buildFileMenu(QMenu &menu)
{
  pqServer* server = pqApplicationCore::instance()->getActiveServer();
  QIcon iconOpen = qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
  QSignalMapper* signalMapper = new QSignalMapper(this);
 
  QAction *action = new QAction(iconOpen,tr("Open - ParaView mode"),this);
  action->setShortcut(tr("Ctrl+O"));
//   pqLoadDataReaction * loadReaction = new pqLoadDataReaction(action);
//   QObject::connect(loadReaction, SIGNAL(loadedData(pqPipelineSource *)),
// 		    m_espina, SLOT( loadSource(pqPipelineSource *)));
  signalMapper->setMapping(action, QString("open"));
  connect(action, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
  menu.addAction(action);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  action = new QAction(iconSave, tr("Save - ParaView mode"), this);
  action->setShortcut(tr("Ctrl+S"));
//  pqSaveDataReaction* saveReaction = new pqSaveDataReaction(action);
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT(saveFile()));
  menu.addAction(action);
  
  //signalMapper->setMapping(accountFileButton, QString("open"));

  action = new QAction(QIcon(":espina/add.svg"),tr("Add"),this);
  signalMapper->setMapping(action, QString("add"));
  connect(action, SIGNAL(triggered(bool)), signalMapper, SLOT(map()));
  menu.addAction(action);

  connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(loadFile(QString)));
  /*
  // Import Trace from localhost
  action = new QAction(tr("Import"),this);
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( importFile()));
  action->setShortcut(tr("Ctrl+I"));
  //QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( fileDialog(NULL, "Import")));
  menu.addAction(action);

  // Export Trace to localhost
  action = new QAction(tr("Export"),this);
  action->setShortcut(tr("Ctrl+E"));
  QObject::connect(action, SIGNAL(triggered(bool)), this, SLOT( exportFile()) );
  menu.addAction(action);
  */
  
}
