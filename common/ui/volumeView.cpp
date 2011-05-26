/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "volumeView.h"

#include "interfaces.h"
#include "renderer.h"
#include "products.h"

// GUI
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QAction>

// ParaView
#include "pqRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineRepresentation.h"
#include "vtkSMUniformGridVolumeRepresentationProxy.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMStringVectorProperty.h"
#include <pqBoxWidget.h>

#include <QDebug>
#include <data/taxonomy.h>
#include <vtkSMRenderViewProxy.h>
#include <pqPipelineSource.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkPVGenericRenderWindowInteractor.h>

//-----------------------------------------------------------------------------
VolumeView::VolumeView(QWidget* parent)
: QAbstractItemView(parent)
, m_VOIWidget(NULL)
{
  m_controlLayout = new QHBoxLayout();
  
  m_controlLayout->addStretch();
  
  connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));
  
  m_mainLayout = new QVBoxLayout();
  m_mainLayout->addLayout(m_controlLayout);
  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
}

//-----------------------------------------------------------------------------
void VolumeView::connectToServer()
{
  //TODO: Review
  //qDebug() << "Creating View";
  pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
  pqServer * server= pqActiveObjects::instance().activeServer();
  m_view = qobject_cast<pqRenderView*>(builder->createView(
    pqRenderView::renderViewType(), server));
  m_viewWidget = m_view->getWidget();
  m_mainLayout->insertWidget(0,m_viewWidget);//To preserver view order
  m_view->setCenterAxesVisibility(true);
  double black[3] = {0,0,0};
  m_view->getRenderViewProxy()->SetBackgroundColorCM(black);
}

//-----------------------------------------------------------------------------
void VolumeView::disconnectFromServer()
{
  // TODO: Review
  if (m_view)
  {
    //qDebug() << "Deleting Widget";
    m_mainLayout->removeWidget(m_viewWidget);
    //qDebug() << "Deleting View";
    //TODO: BugFix -> destroy previous instance of m_view
    //pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
  }
}

//-----------------------------------------------------------------------------
void VolumeView::setVOI(IVOI* voi)
{
  return;
  if (m_VOIWidget)
  {
    m_VOIWidget->deselect();
    m_VOIWidget->setVisible(false);
    delete m_VOIWidget;
    m_VOIWidget = NULL;
  }
  
  qDebug()<< "VolumeView: elemets" << model()->rowCount();
  if (model()->rowCount() == 0)
    return;
     
  if (!voi)
    return;
    
  m_VOIWidget = voi->newWidget();
  m_VOIWidget->setView(m_view);
  m_VOIWidget->setWidgetVisible(true);
  m_VOIWidget->select();
}


//-----------------------------------------------------------------------------
QRegion VolumeView::visualRegionForSelection(const QItemSelection& selection) const
{
  return QRect();
}

//-----------------------------------------------------------------------------
void VolumeView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags command)
{
  qDebug() << "Selection";
}

//-----------------------------------------------------------------------------
bool VolumeView::isIndexHidden(const QModelIndex& index) const
{
  if (!index.isValid())
    return true;
  
  if (index.internalId() < 3)
    return true;
  
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  EspinaProduct *actor = dynamic_cast<EspinaProduct *>(item);
  return !actor;
}

//-----------------------------------------------------------------------------
int VolumeView::verticalOffset() const
{
  return 0;
}

//-----------------------------------------------------------------------------
int VolumeView::horizontalOffset() const
{
  return 0;

}

//-----------------------------------------------------------------------------
QModelIndex VolumeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
void VolumeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  for (int r = start; r <= end; r++)
  {
    QModelIndex index = parent.child(r,0);
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    // Check for sample
    Sample *sample = dynamic_cast<Sample *>(item);
    if (sample)
    {
      //TODO: Render sample
      qDebug() << "Render sample?";
    } 
    else if (!sample)
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      assert(seg); // If not sample, it has to be a segmentation
      seg->representation("Mesh")->render(m_view);
    }
  }
  updateScene();
}

//-----------------------------------------------------------------------------
void VolumeView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  for (int r = start; r <= end; r++)
  {
    QModelIndex index = parent.child(r,0);
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    // Check for sample
    Sample *sample = dynamic_cast<Sample *>(item);
    if (sample)
    {
      //TODO: Render sample
      qDebug() << "Render sample?";
    } 
    else if (!sample)
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      assert(seg); // If not sample, it has to be a segmentation
     dp->setRepresentationVisibility(seg->outputPort(), m_view, false);
    }
  }
  m_view->render();
}

//-----------------------------------------------------------------------------
void VolumeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  if (!topLeft.isValid() || !bottomRight.isValid())
    return;
        
  qDebug()<< "Updating " << topLeft;
  
  
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  //TODO: Update to deal with hierarchy
  assert(topLeft == bottomRight);
  QModelIndex index = topLeft;
  IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
  // Check for sample
  Sample *sample = dynamic_cast<Sample *>(item);
  if (sample)
  {
      //TODO: Render sample
      qDebug() << "Render sample?";
  } 
  else if (!sample)
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    assert(seg); // If not sample, it has to be a segmentation
    foreach (IViewWidget *widget, m_widgets)
    {
      if (widget->isChecked())
	widget->renderInView(index,m_view);
    }
  }
  
  m_view->render();
}


//-----------------------------------------------------------------------------
QModelIndex VolumeView::indexAt(const QPoint& point) const
{
  return QModelIndex();
}

//-----------------------------------------------------------------------------
void VolumeView::scrollTo(const QModelIndex& index, QAbstractItemView::ScrollHint hint)
{
  //updateScene();
}

//-----------------------------------------------------------------------------
QRect VolumeView::visualRect(const QModelIndex& index) const
{
  return QRect();
}

//-----------------------------------------------------------------------------
void VolumeView::addWidget(IViewWidget* widget)
{
  m_controlLayout->addWidget(widget);
  connect(widget,SIGNAL(updateRequired()),this,SLOT(updateScene()));
  connect(widget,SIGNAL(toggled(bool)),this,SLOT(updateScene()));
  m_widgets.append(widget);
}

//-----------------------------------------------------------------------------
void VolumeView::updateScene()
{
  vtkSMRenderViewProxy* view = vtkSMRenderViewProxy::SafeDownCast(
    m_view->getProxy());
  assert(view);
  
  //double cor[3];
  //view->GetInteractor()->GetCenterOfRotation(cor);
  
  //vtkCamera *cam = view->GetActiveCamera();
  //double pos[3], focus[3];
  //cam->GetPosition(pos);
  //cam->GetFocalPoint(focus);
  
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqRepresentation *rep;
  foreach(rep,m_view->getRepresentations())
  {
    rep->setVisible(false);
  }
  
  //TODO: Center on selection bounding box or active stack if no selection
  foreach (IViewWidget *widget, m_widgets)
  {
    if (widget->isChecked())
      widget->renderInView(rootIndex(),m_view);
  }

  //cam->SetPosition(pos);
  //cam->SetFocalPoint(m_focus);
  //view->GetInteractor()->SetCenterOfRotation(m_focus);

  
  m_view->render();
}


// DEPRECATED
void VolumeView::render(const QModelIndex& index)
{
  if (!isIndexHidden(index))
  {
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    // Check for sample
    Sample *sample = dynamic_cast<Sample *>(item);
    if (sample)
    {
      double bounds[6];
      sample->bounds(bounds);
      m_focus[0] = (bounds[1]-bounds[0])/2.0;
      m_focus[1] = (bounds[3]-bounds[2])/2.0;
      m_focus[2] = (bounds[5]-bounds[4])/2.0;
      pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
      dp->setRepresentationVisibility(sample->outputPort(),m_view,true);
    } 
    else if (!sample)
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      assert(seg); // If not sample, it has to be a segmentation
    }
  }
  for (int row = 0; row < model()->rowCount(index); row++)
    render(model()->index(row,0,index));

}
