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
#include "ColorEngine.h"
#include <model/Channel.h>
#include <model/Representation.h>
#include <model/EspinaFactory.h>
#include <pluginInterfaces/Renderer.h>

// GUI
#include <QEvent>
#include <QSettings>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPropPicker.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <QVTKWidget.h>

//-----------------------------------------------------------------------------
VolumeView::VolumeView(QWidget* parent)
: QWidget(parent)
, m_mainLayout      (new QVBoxLayout())
, m_controlLayout   (new QHBoxLayout())
, m_settings        (new Settings())
{
  init();
  buildControls();

  setLayout(m_mainLayout);
  //   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  //   this->setStyleSheet("background-color: grey;");
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

  QPushButton *button;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    button = new QPushButton(renderer->icon(), "");
    button->setFlat(true);
    button->setCheckable(true);
    button->setChecked(true);
    button->setIconSize(QSize(22,22));
    button->setMaximumSize(QSize(32,32));
    button->setToolTip(renderer->tooltip());
    connect(button, SIGNAL(clicked(bool)), renderer.data(), SLOT(setEnable(bool)));
    connect(renderer.data(), SIGNAL(renderRequested()), this, SLOT(forceRender()));
    m_controlLayout->addWidget(button);

    renderer->setVtkRenderer(this->m_renderer);
  }

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
void VolumeView::centerViewOn(double center[3])
{
  if (!isVisible() ||
      (m_center[0] == center[0] &&
       m_center[1] == center[1] &&
       m_center[2] == center[2]))
    return;

  memcpy(m_center, center, 3*sizeof(double));

  //TODO:m_view->SetCrosshair(m_center[0], m_center[1], m_center[2]);
}

//-----------------------------------------------------------------------------
void VolumeView::setCameraFocus(double center[3])
{
  Q_ASSERT(false);
  //TODO:m_view->SetCameraFocus(m_center[0], m_center[1], m_center[2]);
}

//-----------------------------------------------------------------------------
void VolumeView::resetCamera()
{
  this->m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void VolumeView::addChannelRepresentation(Channel* channel)
{
  bool modified = false;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    modified |= renderer->addItem(channel);
  }
}

//-----------------------------------------------------------------------------
bool VolumeView::updateChannelRepresentation(Channel* channel)
{
  bool updated = false;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    updated = renderer->updateItem(channel) | updated;
  }
  return updated;
}

//-----------------------------------------------------------------------------
void VolumeView::removeChannelRepresentation(Channel* channel)
{
  bool modified = false;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    modified |= renderer->removeItem(channel);
  }
  Q_ASSERT(false);
  //TODO:m_view->resetCamera();
}


//-----------------------------------------------------------------------------
void VolumeView::addSegmentationRepresentation(Segmentation *seg)
{
  bool modified = false;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    modified |= renderer->addItem(seg);
  }
}

//-----------------------------------------------------------------------------
bool VolumeView::updateSegmentationRepresentation(Segmentation* seg)
{
  bool updated = false;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    updated = renderer->updateItem(seg) | updated;
  }
  return updated;
}

//-----------------------------------------------------------------------------
void VolumeView::removeSegmentationRepresentation(Segmentation* seg)
{
  bool modified = false;
  foreach(Settings::RendererPtr renderer, m_settings->renderers())
  {
    modified |= renderer->removeItem(seg);
  }
}

void VolumeView::init()
{
  m_viewWidget = new QVTKWidget();
  m_mainLayout->insertWidget(0,m_viewWidget); //To preserver view order
  m_viewWidget->show();
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->SetOcclusionRatio(0.5);
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorstyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactorstyle->AutoAdjustCameraClippingRangeOn();
  interactorstyle->KeyPressActivationOff();
  m_viewWidget->GetRenderWindow()->AddRenderer(m_renderer);
  m_viewWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorstyle);
  m_viewWidget->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void VolumeView::forceRender()
{
  if(isVisible())
  {
      this->m_viewWidget->GetRenderWindow()->Render();
  }
}

//-----------------------------------------------------------------------------
double VolumeView::suggestedChannelOpacity()
{
//   double numVisibleRep = 0;
// 
//   foreach(Channel *channel, m_channels.keys())
//     if (channel->isVisible())
//       numVisibleRep++;
// 
//   if (numVisibleRep == 0)
//     return 0.0;
// 
//   return 1.0 /  numVisibleRep;
return 1;
}

//-----------------------------------------------------------------------------
void VolumeView::selectPickedItems(bool append)
{
//   vtkSMRenderViewProxy *view =
//     vtkSMRenderViewProxy::SafeDownCast(m_view->getProxy());
//   Q_ASSERT(view);
//   vtkRenderer *renderer = view->GetRenderer();
//   Q_ASSERT(renderer);
//   vtkRenderWindowInteractor *rwi =
//     vtkRenderWindowInteractor::SafeDownCast(
//       view->GetRenderWindow()->GetInteractor());
//   Q_ASSERT(rwi);
// 
//   int x, y;
//   rwi->GetEventPosition(x, y);
// 
//   vtkPropPicker *propPicker = vtkPropPicker::New();
//   if (propPicker->Pick(x, y, 0.1, renderer))
//   {
//     vtkProp3D *pickedProp = propPicker->GetProp3D();
//     vtkObjectBase *object;
// 
//     foreach(Channel *channel, m_channels.keys())
//     {
//       vtkCrosshairRepresentation *rep;
//       object = m_channels[channel].proxy->GetClientSideObject();
//       rep = dynamic_cast<vtkCrosshairRepresentation *>(object);
//       Q_ASSERT(rep);
//       if (rep->GetCrosshairProp() == pickedProp)
//       {
// // 	qDebug() << "Channel" << channel->data(Qt::DisplayRole).toString() << "Selected";
// 	emit channelSelected(channel);
// 	return;
//       }
//     }
// 
//     foreach(Segmentation *seg, m_segmentations.keys())
//     {
//       vtkVolumetricRepresentation *rep;
//       object = m_segmentations[seg].proxy->GetClientSideObject();
//       rep = dynamic_cast<vtkVolumetricRepresentation *>(object);
//       Q_ASSERT(rep);
//       if (rep->GetVolumetricProp() == pickedProp)
//       {
// // 	qDebug() << "Segmentation" << seg->data(Qt::DisplayRole).toString() << "Selected";
// 	emit segmentationSelected(seg, append);
// 	return;
//       }
//     }
//   }
}
//-----------------------------------------------------------------------------
bool VolumeView::eventFilter(QObject* caller, QEvent* e)
{
  return QObject::eventFilter(caller, e);

  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      selectPickedItems(me->modifiers() == Qt::SHIFT);
    }
  }

  return QObject::eventFilter(caller, e);
}


//-----------------------------------------------------------------------------
void VolumeView::exportScene()
{
  Q_ASSERT(false);
//   pqViewExporterManager *exporter = new pqViewExporterManager();
//   exporter->setView(m_view);
//   QString fileName = QFileDialog::getSaveFileName(this,
//      tr("Save Scene"), "", tr("3D Scene (*.x3d *.pov *.vrml)"));
//   exporter->write(fileName);
//   delete exporter;
}

void VolumeView::takeSnapshot()
{
  Q_ASSERT(false);
//   QString fileName = QFileDialog::getSaveFileName(this,
//      tr("Save Scene"), "", tr("Image Files (*.jpg *.png)"));
//   m_view->saveImage(1024,768,fileName);
}

//-----------------------------------------------------------------------------
VolumeView::Settings::Settings(const QString prefix)
: RENDERERS(prefix + "VolumeView::renderers")
{
  QSettings settings("CeSViMa", "EspINA");

  if (!settings.contains(RENDERERS))
    settings.setValue(RENDERERS, QStringList() << "Crosshairs" << "Volumetric");

  QMap<QString, Renderer *> renderers = EspinaFactory::instance()->renderers();
  foreach(QString name, settings.value(RENDERERS).toStringList())
  {
    Renderer *renderer = renderers.value(name, NULL);
    if (renderer)
      m_renderers << RendererPtr(renderer->clone());
  }

}

//-----------------------------------------------------------------------------
void VolumeView::Settings::setRenderers(QList<Renderer *> values)
{
  QSettings settings("CeSViMa", "EspINA");
  QStringList activeRenderers;

  m_renderers.clear();
  foreach(Renderer *renderer, values)
  {
    m_renderers << RendererPtr(renderer->clone());
    activeRenderers << renderer->name();
  }
  settings.setValue(RENDERERS, activeRenderers);
}

//-----------------------------------------------------------------------------
QList< VolumeView::Settings::RendererPtr> VolumeView::Settings::renderers() const
{
  return m_renderers;
}
