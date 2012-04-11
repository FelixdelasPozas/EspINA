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

#include "VolumeView.h"

// Debug
#include <QDebug>

// EspINA
#include <common/model/Segmentation.h>

// GUI
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// ParaView
#include <pqActiveObjects.h>
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include "pqRenderView.h"
#include <pqServer.h>
#include <pqServerManagerObserver.h>
#include <pqViewExporterManager.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSMRenderViewProxy.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
#include <pqScalarsToColors.h>
#include <pq3DWidget.h>
#include <pqOutputPort.h>
#include "ColorEngine.h"
#include </home/jpena/src/ParaView-3.12.0/Qt/Core/pqPipelineRepresentation.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>

//-----------------------------------------------------------------------------
VolumeView::VolumeView(QWidget* parent)
: QAbstractItemView (parent)
, m_mainLayout      (new QVBoxLayout())
, m_controlLayout   (new QHBoxLayout())
// , m_VOIWidget(NULL)
// , m_lastSample(NULL)
{
  buildControls();

  setLayout(m_mainLayout);
  //   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  //   this->setStyleSheet("background-color: grey;");

  pqServerManagerObserver *SMObserver = pqApplicationCore::instance()->getServerManagerObserver();
  connect(SMObserver, SIGNAL(connectionCreated(vtkIdType)),
	  this, SLOT(onConnect()));
  connect(SMObserver, SIGNAL(connectionClosed(vtkIdType)),
	  this, SLOT(onDisconnect()));

  if (pqApplicationCore::instance()->getActiveServer())
    onConnect();
}

//-----------------------------------------------------------------------------
void VolumeView::buildControls()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_snapshot.setIcon(QIcon(":/espina/snapshot_scene.svg"));
  m_snapshot.setToolTip(tr("Save Scene as Image"));
  m_snapshot.setFlat(true);
  m_snapshot.setIconSize(QSize(22,22));
  m_snapshot.setMaximumSize(QSize(32,32));
  connect(&m_snapshot,SIGNAL(clicked(bool)),this,SLOT(takeSnapshot()));

  m_export.setIcon(QIcon(":/espina/export_scene.svg"));
  m_export.setToolTip(tr("Export 3D Scene"));
  m_export.setFlat(true);
  m_export.setIconSize(QSize(22,22));
  m_export.setMaximumSize(QSize(32,32));
  connect(&m_export,SIGNAL(clicked(bool)),this,SLOT(exportScene()));
  
  QSpacerItem * horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(&m_snapshot);
  m_controlLayout->addWidget(&m_export);
  m_controlLayout->addItem(horizontalSpacer);

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
void VolumeView::addSegmentationRepresentation(Segmentation *seg)
{
  pqOutputPort *oport = seg->outputPort();
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqDataRepresentation *dr = dp->setRepresentationVisibility(oport, m_view, true);
  if (!dr)
    return;

  pqPipelineRepresentation *repProxy = qobject_cast<pqPipelineRepresentation *>(dr);
  Q_ASSERT(repProxy);
  repProxy->setRepresentation("Volume");

  m_segmentations[seg].outport = oport;
  m_segmentations[seg].repProxy = repProxy;

  updateSegmentationRepresentation(seg);
}

//-----------------------------------------------------------------------------
void VolumeView::removeSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));
  SegRep rep = m_segmentations[seg];
//   qDebug() << "Remove Seg:" << seg->number() << "Rep:" << rep;
  rep.repProxy->setVisible(false);
//   rep->deleteLater();
  m_segmentations.remove(seg);
}

//-----------------------------------------------------------------------------
bool VolumeView::updateSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));
  SegRep &rep = m_segmentations[seg];
  if (seg->outputPort() != rep.outport)
  {
    removeSegmentationRepresentation(seg);
    addSegmentationRepresentation(seg);
    return true;
  }
  if (seg->selected() != rep.selected
    || seg->visible() != rep.visible
    || m_colorEngine->color(seg) != rep.color)
  {
    rep.selected = seg->selected();
    rep.visible  = seg->visible();
    rep.color = m_colorEngine->color(seg);
    //   repProxy->PrintSelf(std::cout,vtkIndent(0));
    vtkSMProperty *p = rep.repProxy->getProxy()->GetProperty("LookupTable");
    vtkSMProxyProperty *lut = vtkSMProxyProperty::SafeDownCast(p);
    if (lut)
      lut->SetProxy(0, m_colorEngine->lut(seg));
    rep.repProxy->setVisible(rep.visible);
    rep.repProxy->getProxy()->UpdateVTKObjects();
    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------
void VolumeView::addRepresentation(pqOutputPort* oport)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqDataRepresentation *dr = dp->setRepresentationVisibility(oport, m_view, true);
  if (!dr)
    return;
  pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
  Q_ASSERT(rep);
  rep->setRepresentation("Volume");
}

//-----------------------------------------------------------------------------
void VolumeView::addWidget(pq3DWidget* widget)
{
  widget->setView(m_view);
  widget->setWidgetVisible(true);
  widget->select();
  widget->setEnabled(false);
}


//-----------------------------------------------------------------------------
void VolumeView::onConnect()
{
    qDebug() << this << ": Connecting to a new server";

  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  pqServer    *server = pqActiveObjects::instance().activeServer();

  m_view = qobject_cast<pqRenderView*>(ob->createView(
             pqRenderView::renderViewType(), server));

  m_viewWidget = m_view->getWidget();
  m_mainLayout->insertWidget(0,m_viewWidget);//To preserver view order

  m_view->setCenterAxesVisibility(false);

  vtkSMRenderViewProxy *viewProxy = vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
//   assert(viewProxy);
//   vtkCamera *cam = viewProxy->GetActiveCamera();
//   cam->Azimuth(180);
//   cam->Roll(180);

  // Change Render Window Interactor
  vtkRenderWindowInteractor *rwi = vtkRenderWindowInteractor::SafeDownCast(
    viewProxy->GetRenderWindow()->GetInteractor());
  Q_ASSERT(rwi);

  vtkInteractorStyleTrackballCamera *style = vtkInteractorStyleTrackballCamera::New();
  rwi->SetInteractorStyle(style);
  style->Delete();

  double black[3] = {0,0,0};
  viewProxy->GetRenderer()->SetBackground(black);

//   // Disable menu
//   //TODO : OLDVERSION m_view->getWidget()->removeAction(m_view->getWidget()->actions().first());
// 
// //   QObject::connect(m_viewWidget, SIGNAL(mouseEvent(QMouseEvent *)),
// //                    this, SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
}

//-----------------------------------------------------------------------------
void VolumeView::onDisconnect()
{
//   pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
//    qDebug() << this << ": Disconnecting from server";
//   if (m_view)
//   {
//     m_mainLayout->removeWidget(m_viewWidget);
//     ob->destroy(m_view);
//     m_view = NULL;
//   }
}

//-----------------------------------------------------------------------------
void VolumeView::forceRender()
{
  if(isVisible())
    m_view->forceRender();
}


// //-----------------------------------------------------------------------------
// void VolumeView::setVOI(IVOI* voi)
// {
//   if (m_VOIWidget)
//   {
//     m_VOIWidget->deselect();
//     m_VOIWidget->setVisible(false);
//     delete m_VOIWidget;
//     m_VOIWidget = NULL;
//   }
//   
// //   qDebug()<< "VolumeView: elemets" << model()->rowCount();
//   if (model()->rowCount() == 0)
//     return;
//      
//   if (!voi)
//     return;
//     
//   m_VOIWidget = voi->newWidget(VIEW_3D);
//   m_VOIWidget->setView(m_view);
//   m_VOIWidget->setWidgetVisible(true);
//   m_VOIWidget->select();
//   m_VOIWidget->accept();
//   voi->resizeToDefaultSize();
// }
// 
// 
// //-----------------------------------------------------------------------------
// QRegion VolumeView::visualRegionForSelection(const QItemSelection& selection) const
// {
//   return QRect();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
// {
//   qDebug() << "Selection";
// }
// 
// //-----------------------------------------------------------------------------
// bool VolumeView::isIndexHidden(const QModelIndex& index) const
// {
//   if (!index.isValid())
//     return true;
//   
//   if (index.internalId() < 3)
//     return true;
//   
//   IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//   EspinaProduct *actor = dynamic_cast<EspinaProduct *>(item);
//   return !actor;
// }
// 
// //-----------------------------------------------------------------------------
// int VolumeView::verticalOffset() const
// {
//   return 0;
// }
// 
// //-----------------------------------------------------------------------------
// int VolumeView::horizontalOffset() const
// {
//   return 0;
// 
// }
// 
// //-----------------------------------------------------------------------------
// QModelIndex VolumeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
// {
//   return QModelIndex();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::rowsInserted(const QModelIndex& parent, int start, int end)
// {
//   for (int r = start; r <= end; r++)
//   {
//     QModelIndex index = model()->index(r,0,parent);
//     IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//     // Check for sample
//     Sample *sample = dynamic_cast<Sample *>(item);
//     if (sample)
//     {
//       m_lastSample = sample;
//       double bounds[6];
//       sample->bounds(bounds);
//       m_focus[0] = (bounds[1]-bounds[0])/2.0;
//       m_focus[1] = (bounds[3]-bounds[2])/2.0;
//       m_focus[2] = (bounds[5]-bounds[4])/2.0;
//       connect(sample->representation(LabelMapExtension::SampleRepresentation::ID),SIGNAL(representationUpdated()),this,SLOT(updateScene()));
//       connect(sample->representation(CrosshairExtension::SampleRepresentation::ID),SIGNAL(representationUpdated()),this,SLOT(updateScene()));
//       connect(sample,SIGNAL(updated(Sample*)),this,SLOT(updateScene()));
//       sample->representation(CrosshairExtension::SampleRepresentation::ID)->render(m_view);
//     } 
//     else 
//     {
// //       Segmentation *seg = dynamic_cast<Segmentation *>(item);
// //       assert(seg); // If not sample, it has to be a segmentation
// //       seg->representation("Mesh")->render(m_view);
//     }
//   }
//   updateScene();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
// {
//   pqApplicationCore *core = pqApplicationCore::instance();
//   pqObjectBuilder *ob = core->getObjectBuilder();
//   pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//   for (int r = start; r <= end; r++)
//   {
//     QModelIndex index = model()->index(r,0,parent);
//     IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//     // Check for sample
//     Sample *sample = dynamic_cast<Sample *>(item);
//     if (sample)
//     {
//       //TODO: Render sample
//       qDebug() << "Render sample?";
//     } 
//     else
//     {
//       Segmentation *seg = dynamic_cast<Segmentation *>(item);
//       assert(seg); // If not sample, it has to be a segmentation
//       pqRepresentation *rep = dp->setRepresentationVisibility(seg->outputPort(), m_view, false);
//       if (rep)
//       {
// 	qDebug() << seg->label() << " destroyed";
// 	ob->destroy(rep);
//       }
//     }
//   }
//   m_view->render();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
// {
//   if (!topLeft.isValid() || !bottomRight.isValid())
//     return;
//         
// //   qDebug()<< "Update Request " << topLeft;
//   
//   pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//   //TODO: Update to deal with hierarchy
//   assert(topLeft == bottomRight);
//   QModelIndex index = topLeft;
//   IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
//   // Check for sample
//   Sample *sample = dynamic_cast<Sample *>(item);
//   if (sample)
//   {
//     sample->representation(CrosshairExtension::SampleRepresentation::ID)->render(m_view);
//     m_lastSample = sample;
//   } 
//   else
//   {
//     Segmentation *seg = dynamic_cast<Segmentation *>(item);
//     assert(seg); // If not sample, it has to be a segmentation
//     foreach (IViewWidget *widget, m_widgets)
//     {
//       if (widget->isChecked())
// 	widget->renderInView(index,m_view);
//     }
//   }
//   
//   m_view->render();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::setRootIndex(const QModelIndex& index)
// {
//   QAbstractItemView::setRootIndex(index);
//   if (index.isValid())
//     updateScene();
// }
// 
// 
// 
// 
// //-----------------------------------------------------------------------------
// QModelIndex VolumeView::indexAt(const QPoint& point) const
// {
//   return QModelIndex();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
// {
//   //updateScene();
// }
// 
// //-----------------------------------------------------------------------------
// QRect VolumeView::visualRect(const QModelIndex& index) const
// {
//   return QRect();
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::addWidget(IViewWidget* widget)
// {
//   m_controlLayout->addWidget(widget);
//   connect(widget,SIGNAL(updateRequired()),this,SLOT(updateScene()));
//   connect(widget,SIGNAL(toggled(bool)),this,SLOT(updateScene()));
//   m_widgets.append(widget);
// }
// 
// 
// //-----------------------------------------------------------------------------
// void VolumeView::selectSegmentations(int x, int y, int z)
// {
//   QItemSelection selection;
//   
//   QModelIndex selIndex;
//   for (int i=0; i < model()->rowCount(rootIndex()); i++)
//   {
//     QModelIndex segIndex = rootIndex().child(i,0);
//     IModelItem *segItem = static_cast<IModelItem *>(segIndex.internalPointer());
//     Segmentation *seg = dynamic_cast<Segmentation *>(segItem);
//     assert(seg);
//     
//     
//     seg->creator()->pipelineSource()->updatePipeline();;
//     seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
//     vtkPVDataInformation *info = seg->outputPort()->getDataInformation();
//     int extent[6];
//     info->GetExtent(extent);
//     if ((extent[0] <= x && x <= extent[1]) &&
//       (extent[2] <= y && y <= extent[3]) &&
//       (extent[4] <= z && z <= extent[5]))
//     {
//       SegmentationSelectionExtension *selector = dynamic_cast<SegmentationSelectionExtension *>(
// 	seg->extension(SegmentationSelectionExtension::ID));
//       
//       if (selector->isSegmentationPixel(x,y,z))
// 	selIndex = segIndex;
//       // 	seg->outputPort()->getDataInformation()->GetPointDataInformation();
//       //selection.indexes().append(segIndex);
// //       double pixelValue[4];
// //       pixelValue[0] = x;
// //       pixelValue[1] = y;
// //       pixelValue[2] = z;
// //       pixelValue[3] = 4;
// //       vtkSMPropertyHelper(seg->creator()->pipelineSource()->getProxy(),"CheckPixel").Set(pixelValue,4);
// //       seg->creator()->pipelineSource()->getProxy()->UpdateVTKObjects();
// //       int value;
// //       seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
// //       vtkSMPropertyHelper(seg->creator()->pipelineSource()->getProxy(),"PixelValue").Get(&value,1);
// //       	qDebug() << "Pixel Value" << value;
// //       if (value == 255)
// // 	selIndex = segIndex;
//     }
//     if (selIndex.isValid())
//       selectionModel()->select(selIndex,QItemSelectionModel::ClearAndSelect);
//     else
//       selectionModel()->clearSelection();
//   }
// }
// 
// //-----------------------------------------------------------------------------
// void VolumeView::vtkWidgetMouseEvent(QMouseEvent* event)
// {
//   if (!m_lastSample)
//     return;
//   
//   //Use Render Window Interactor's to obtain event's position
//   vtkSMRenderViewProxy* view = 
//     vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
//   //vtkSMRenderViewProxy* renModule = view->GetRenderView();
//   vtkRenderWindowInteractor *rwi =
//     vtkRenderWindowInteractor::SafeDownCast(
//       view->GetRenderWindow()->GetInteractor());
//       //renModule->GetInteractor());
//   assert(rwi);
//   
//   vtkSMRenderViewProxy *viewProxy = vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
//   
//   int xPos, yPos;
//   rwi->GetEventPosition(xPos, yPos);
//   
//   if (event->type() == QMouseEvent::MouseButtonPress &&
//     event->buttons() == Qt::LeftButton)
//   {
//     double spacing[3];//Image Spacing
//     m_lastSample->spacing(spacing);
//     
//     double pickPos[3];//World coordinates
//     vtkPropPicker *wpicker = vtkPropPicker::New();
//     //TODO: Check this--> wpicker->AddPickList();
//     wpicker->Pick(xPos, yPos, 0.1, viewProxy->GetRenderer());
//     wpicker->GetPickPosition(pickPos);
//     
//     int selectedPixel[3];
//     for(int dim = 0; dim < 3; dim++)
//       selectedPixel[dim] = round(pickPos[dim]/spacing[dim]);
//     selectSegmentations(selectedPixel[0], selectedPixel[1], selectedPixel[2]);
//   }
// }
// 
// 
// //-----------------------------------------------------------------------------
// void VolumeView::updateScene()
// {
//   vtkSMRenderViewProxy* view = vtkSMRenderViewProxy::SafeDownCast(
//     m_view->getProxy());
//   assert(view);
//   
// //   double cor[3];
// //   view->GetInteractor()->GetCenterOfRotation(cor);
// //   
//   vtkCamera *cam = view->GetActiveCamera();
//   double pos[3], focus[3];
//   cam->GetPosition(pos);
// //   cam->GetFocalPoint(focus);
//   
//   pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//   pqRepresentation *rep;
//   foreach(rep,m_view->getRepresentations())
//   {
//     rep->setVisible(false);
//   }
//   
// //   if (selectionModel()->selection().size() == 0)
// //   {
// //     focus[0] = m_focus[0];
// //     focus[1] = m_focus[1];
// //     focus[1] = m_focus[2];
// //   }
//   
//   foreach (IViewWidget *widget, m_widgets)
//   {
// //     if (widget->isChecked())
//       widget->renderInView(rootIndex(),m_view);
//   }
// 
//   //cam->SetFocalPoint(focus);
//   //view->GetInteractor()->SetCenterOfRotation(focus);
//   m_view->resetCamera();
//   cam->SetPosition(pos);
// 
//   
//   m_view->render();
// }
// 
void VolumeView::exportScene()
{
  pqViewExporterManager *exporter = new pqViewExporterManager();
  exporter->setView(m_view);
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Save Scene"), "", tr("3D Scene (*.x3d *.pov *.vrml)"));
  exporter->write(fileName);
  delete exporter;
  m_view->saveImage(1024,768,"/home/jorge/scene.jpg");
}

void VolumeView::takeSnapshot()
{
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Save Scene"), "", tr("Image Files (*.jpg *.png)"));
  m_view->saveImage(1024,768,fileName);
}