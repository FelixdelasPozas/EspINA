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

#include <QDebug>
#include <data/taxonomy.h>
#include <traceNodes.h>
#include <vtkSMRenderViewProxy.h>
#include <pqPipelineSource.h>

//-----------------------------------------------------------------------------
VolumeView::VolumeView(QWidget* parent)
: QAbstractItemView(parent)
, m_showSegmentations(false)
{
  m_controlLayout = new QHBoxLayout();
  
  m_toggleActors = new QToolButton(this);
  m_toggleActors->setIcon(QIcon(":/espina/hide3D"));
  m_toggleActors->setCheckable(true);
  
  m_controlLayout->addStretch();
  m_controlLayout->addWidget(m_toggleActors);
  
  QMenu *renders = new QMenu();
  QAction *volumeRenderer = new QAction(QIcon(":/espina/hide3D"),tr("Volume"),renders);
  QAction *meshRenderer = new QAction(QIcon(":/espina/hidePlanes"),tr("Mesh"),renders);
  renders->addAction(volumeRenderer);
  renders->addAction(meshRenderer);
  m_toggleActors->setMenu(renders);
  connect(m_toggleActors,SIGNAL(toggled(bool)),this,SLOT(showSegmentations(bool)));
  connect(volumeRenderer,SIGNAL(triggered()),this,SLOT(setVolumeRenderer()));
  connect(meshRenderer,SIGNAL(triggered()),this,SLOT(setMeshRenderer()));
  
  m_mainLayout = new QVBoxLayout();
  m_mainLayout->addLayout(m_controlLayout);
  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");
  
  m_renderer = VolumeRenderer::renderer();
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
void VolumeView::showSegmentations(bool value)
{
  if (m_showSegmentations == value)
    return;
  
  m_showSegmentations = value;
  switch (m_renderer->type())
  {
    case MESH_RENDERER:
      m_toggleActors->setIcon(m_showSegmentations?QIcon(":/espina/showPlanes"):QIcon(":/espina/hidePlanes"));
      break;
    case VOLUME_RENDERER:
      m_toggleActors->setIcon(m_showSegmentations?QIcon(":/espina/show3D"):QIcon(":/espina/hide3D"));
      break;
    default:
      assert(false);
  }
  
  updateScene();
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
  Product *actor = dynamic_cast<Product *>(item);
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
    else if (!sample && m_showSegmentations)
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      assert(seg); // If not sample, it has to be a segmentation
      m_renderer->render(seg,m_view);
    }
  }
  m_view->render();
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
    else if (!sample && m_showSegmentations)
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
  else if (!sample && m_showSegmentations)
  {
    Segmentation *seg = dynamic_cast<Segmentation *>(item);
    assert(seg); // If not sample, it has to be a segmentation
    dp->setRepresentationVisibility(seg->outputPort(), m_view, seg->visible());
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
void VolumeView::setMeshRenderer()
{
  m_renderer = MeshRenderer::renderer();
  // Which one is easier to read/understand? This one or the VolumeRenderer one?
  m_toggleActors->setIcon(
    m_showSegmentations?
      QIcon(":/espina/showPlanes")
    :
      QIcon(":/espina/hidePlanes")
	);
  
  updateScene();
}

//-----------------------------------------------------------------------------
void VolumeView::setVolumeRenderer()
{
  m_renderer = VolumeRenderer::renderer();
  m_toggleActors->setIcon(m_showSegmentations?QIcon(":/espina/show3D"):QIcon(":/espina/hide3D"));
  
  updateScene();
}

//-----------------------------------------------------------------------------
void VolumeView::updateScene()
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqRepresentation *rep;
  foreach(rep,m_view->getRepresentations())
  {
    rep->setVisible(false);
  }
  
  render(rootIndex());
  
  //TODO: Center on selection bounding box or active stack if no selection
  foreach (IViewWidget *widget, m_widgets)
    if (widget->isChecked())
      widget->renderInView(m_view);

  //m_view->resetCamera();
  
  m_view->render();
  //if (m_showPlanes)
  //  dp->setRepresentationVisibility(m_planes[SLICE_AXIS_X]->getBgOutput(),m_view,true);
  
  // Render renderable products
  /*
  foreach(actor,m_actors)
  {
    if (m_showActors)
      m_renderer->render(actor,m_view);
  }
  */
    /* XXX: Is really necessary? They should be already hidden by 
     * initial foreach loop
    else
      m_renderer->hide(actor,m_view);
  }
    */
  /*
  // Render planes
  for (SlicePlane plane = SLICE_PLANE_FIRST
    ; plane <= SLICE_PLANE_LAST
    ; plane = SlicePlane(plane+1))
    dp->setRepresentationVisibility(m_planes[plane]->getOutput(),m_view,m_showPlanes);
  */
}


void VolumeView::render(const QModelIndex& index)
{
  if (!isIndexHidden(index))
  {
    IModelItem *item = static_cast<IModelItem *>(index.internalPointer());
    // Check for sample
    Sample *sample = dynamic_cast<Sample *>(item);
    if (sample)
    {
      pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
      dp->setRepresentationVisibility(sample->sourceData()->getOutputPort(0),m_view,true);
    } 
    else if (!sample && m_showSegmentations)
    {
      Segmentation *seg = dynamic_cast<Segmentation *>(item);
      assert(seg); // If not sample, it has to be a segmentation
      m_renderer->render(seg,m_view);
    }
  }
  for (int row = 0; row < model()->rowCount(index); row++)
    render(model()->index(row,0,index));
}

