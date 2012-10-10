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
#include <model/Channel.h>
#include <model/Representation.h>
#include <model/EspinaFactory.h>
#include <pluginInterfaces/Renderer.h>
#include <settings/EspinaSettings.h>
#include <renderers/CrosshairRenderer.h>

// GUI
#include <QEvent>
#include <QSettings>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMessageBox>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPropPicker.h>
#include <vtkPolyDataMapper.h>
#include <QVTKWidget.h>
#include <vtkCamera.h>
#include <vtkPOVExporter.h>
#include <vtkVRMLExporter.h>
#include <vtkX3DExporter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkRenderLargeImage.h>
#include <QApplication>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>

//-----------------------------------------------------------------------------
VolumeView::VolumeView(ViewManager* vm, QWidget* parent)
: EspinaRenderView(parent)
, m_viewManager(vm)
, m_mainLayout      (new QVBoxLayout())
, m_controlLayout   (new QHBoxLayout())
, m_settings        (new Settings(QString(), this))
{
  setupUI();
  buildControls();

  setLayout(m_mainLayout);
  // connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  //   this->setStyleSheet("background-color: grey;");

  memset(m_center,0,3*sizeof(double));
  m_numEnabledRenders = 0;
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection)),
          this, SLOT(updateSelection(ViewManager::Selection)));
}

//-----------------------------------------------------------------------------
void VolumeView::addRendererControls(Renderer* renderer)
{
  QPushButton *button;

  button = new QPushButton(renderer->icon(), "");
  button->setFlat(true);
  button->setCheckable(true);
  button->setChecked(false);
  button->setIconSize(QSize(22, 22));
  button->setMaximumSize(QSize(32, 32));
  button->setToolTip(renderer->tooltip());
  button->setObjectName(renderer->name());
  connect(button, SIGNAL(clicked(bool)), renderer, SLOT(setEnable(bool)));
  connect(button, SIGNAL(clicked(bool)), this, SLOT(countEnabledRenderers(bool)));
  connect(button, SIGNAL(destroyed(QObject*)), renderer, SLOT(deleteLater()));
  connect(renderer, SIGNAL(renderRequested()), this, SLOT(updateView()));
  m_controlLayout->addWidget(button);
  renderer->setVtkRenderer(this->m_renderer);

  // add all model items to the renderer
  foreach(ModelItem *item, m_addedItems)
    renderer->addItem(item);

  m_itemRenderers << renderer;
}

//-----------------------------------------------------------------------------
void VolumeView::removeRendererControls(const QString name)
{
  for (int i = 0; i < m_controlLayout->count(); i++)
  {
    if (m_controlLayout->itemAt(i)->isEmpty())
      continue;

    if (m_controlLayout->itemAt(i)->widget()->objectName() == name)
    {
      QWidget *button = m_controlLayout->itemAt(i)->widget();
      m_controlLayout->removeWidget(button);
      delete button;
    }
  }

  Renderer *removedRenderer = NULL;
  foreach (Renderer *renderer, m_itemRenderers)
  {
    if (renderer->name() == name)
    {
      removedRenderer = renderer;
      break;
    }
  }

  m_itemRenderers.removeAll(removedRenderer);
  delete removedRenderer;
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

  foreach(Renderer* renderer, m_settings->renderers())
    this->addRendererControls(renderer->clone());

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
void VolumeView::centerViewOn(Nm center[3])
{
  if (!isVisible() ||
      (m_center[0] == center[0] &&
       m_center[1] == center[1] &&
       m_center[2] == center[2]))
    return;

  memcpy(m_center, center, 3*sizeof(double));

  foreach(Renderer* ren, m_itemRenderers)
  {
    if (QString("Crosshairs") == ren->name())
    {
      CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren);
      crossren->setCrosshair(center);
    }
  }

  updateView();
}

//-----------------------------------------------------------------------------
void VolumeView::setCameraFocus(const Nm center[3])
{
  m_renderer->GetActiveCamera()->SetFocalPoint(center[0],center[1],center[2]);
}

//-----------------------------------------------------------------------------
void VolumeView::resetCamera()
{
  this->m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void VolumeView::addChannel(Channel* channel)
{
  m_addedItems << channel;
  foreach(Renderer* renderer, m_itemRenderers)
    renderer->addItem(channel);
}

//-----------------------------------------------------------------------------
bool VolumeView::updateChannel(Channel* channel)
{
  if (!isVisible())
    return false;

  bool updated = false;
  foreach(Renderer* renderer, m_itemRenderers)
    updated = renderer->updateItem(channel) | updated;

  return updated;
}

//-----------------------------------------------------------------------------
void VolumeView::removeChannel(Channel* channel)
{
  m_addedItems.removeAll(channel);

  foreach(Renderer* renderer, m_itemRenderers)
    renderer->removeItem(channel);
}


//-----------------------------------------------------------------------------
void VolumeView::addSegmentation(Segmentation *seg)
{
  Q_ASSERT(!m_segmentations.contains(seg));

  m_addedItems << seg;
  foreach(Renderer* renderer, m_itemRenderers)
    renderer->addItem(seg);

  m_segmentations << seg;
}

//-----------------------------------------------------------------------------
bool VolumeView::updateSegmentation(Segmentation* seg)
{
  if (!isVisible())
    return false;

  bool updated = false;
  foreach(Renderer* renderer, m_itemRenderers)
    updated = renderer->updateItem(seg) | updated;

  return updated;
}

//-----------------------------------------------------------------------------
void VolumeView::removeSegmentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));

  m_addedItems.removeAll(seg);
  foreach(Renderer* renderer, m_itemRenderers)
    renderer->removeItem(seg);

  m_segmentations.removeOne(seg);
}

//-----------------------------------------------------------------------------
void VolumeView::addWidget(EspinaWidget* widget)
{
  /* TODO 2012-10-09
  if (!widget)
    return;

  Q_ASSERT(!m_widgets.contains(widget));

  widget->SetInteractor(m_viewWidget->GetRenderWindow()->GetInteractor());
  widget->GetRepresentation()->SetVisibility(true);
  widget->On();
  m_renderer->ResetCameraClippingRange();
  m_widgets << widget;
  */
}

//-----------------------------------------------------------------------------
void VolumeView::removeWidget(EspinaWidget* widget)
{
  /* TODO 2012-10-09
  if (!widget)
    return;

  Q_ASSERT(m_widgets.contains(widget));

  m_widgets.removeOne(widget);
  */
}

//-----------------------------------------------------------------------------
void VolumeView::setCursor(const QCursor& cursor)
{
  m_view->setCursor(cursor);
}

//-----------------------------------------------------------------------------
void VolumeView::eventPosition(int& x, int& y)
{
//   vtkRenderWindowInteractor *rwi = m_renderWindow->GetInteractor();
//   Q_ASSERT(rwi);
//   rwi->GetEventPosition(x, y);
}

//-----------------------------------------------------------------------------
IPicker::PickList VolumeView::pick(IPicker::PickableItems filter, IPicker::DisplayRegionList regions)
{
  return IPicker::PickList();
}

//-----------------------------------------------------------------------------
vtkRenderWindow* VolumeView::renderWindow()
{
  return m_view->GetRenderWindow();
}


//-----------------------------------------------------------------------------
void VolumeView::setupUI()
{
  m_view = new QVTKWidget();
  m_mainLayout->insertWidget(0,m_view); //To preserver view order
  m_view->show();
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->LightFollowCameraOn();
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorstyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactorstyle->AutoAdjustCameraClippingRangeOn();
  interactorstyle->KeyPressActivationOff();
  m_view->GetRenderWindow()->AddRenderer(m_renderer);
  m_view->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorstyle);
  m_view->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void VolumeView::updateView()
{
  if(isVisible())
  {
//     qDebug() << "Render Volume View";
    this->m_view->GetRenderWindow()->Render();
    this->m_view->update();
  }
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
  // only mesh actors are exported in a 3D scene, not volumes
  unsigned int numActors = 0;
  foreach(Renderer* renderer, m_itemRenderers)
    numActors += renderer->getNumberOfvtkActors();

  if (0 == numActors)
  {
    QMessageBox msgBox;
    QString message(tr("The scene can not be exported because there are no mesh objects in it."));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(message);
    msgBox.exec();
    return;
  }

  QFileDialog fileDialog(this, tr("Save Scene"), QString(), tr("All supported formats (*.x3d *.pov *.vrml);; POV-Ray files (*.pov);; VRML files (*.vrml);; X3D format (*.x3d)"));
  fileDialog.setObjectName("SaveSceneFileDialog");
  fileDialog.setWindowTitle("Save View as a 3D Scene");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix(QString(tr("vrml")));
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile("");

  if (fileDialog.exec() == QDialog::Accepted)
  {
    const QString selectedFile = fileDialog.selectedFiles().first();

    QStringList splittedName = selectedFile.split(".");
    QString extension = splittedName[((splittedName.size())-1)].toUpper();

    QStringList validFileExtensions;
    validFileExtensions << "X3D" << "POV" << "VRML";

    if (validFileExtensions.contains(extension))
    {
      if (QString("POV") == extension)
      {
        vtkPOVExporter *exporter = vtkPOVExporter::New();
        exporter->DebugOn();
        exporter->SetGlobalWarningDisplay(true);
        exporter->SetFileName(selectedFile.toStdString().c_str());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        exporter->Write();
        QApplication::restoreOverrideCursor();
        exporter->Delete();
      }

      if (QString("VRML") == extension)
      {
        vtkVRMLExporter *exporter = vtkVRMLExporter::New();
        exporter->DebugOn();
        exporter->SetFileName(selectedFile.toStdString().c_str());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        exporter->Write();
        QApplication::restoreOverrideCursor();
        exporter->Delete();
      }

      if (QString("X3D") == extension)
      {
        vtkX3DExporter *exporter = vtkX3DExporter::New();
        exporter->DebugOn();
        exporter->SetFileName(selectedFile.toStdString().c_str());
        exporter->SetRenderWindow(m_renderer->GetRenderWindow());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        exporter->Write();
        QApplication::restoreOverrideCursor();
        exporter->Delete();
      }
    }
    else
    {
      QMessageBox msgBox;
      QString message(tr("Scene not exported. Unrecognized extension "));
      message.append(extension).append(".");
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(message);
      msgBox.exec();
    }
  }
}

void VolumeView::takeSnapshot()
{
  QFileDialog fileDialog(this, tr("Save Scene As Image"), QString(), tr("All supported formats (*.jpg *.png);; JPEG images (*.jpg);; PNG images (*.png)"));
  fileDialog.setObjectName("SaveSnapshotFileDialog");
  fileDialog.setWindowTitle("Save Scene As Image");
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setDefaultSuffix(QString(tr("png")));
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile("");

  if (fileDialog.exec() == QDialog::Accepted)
  {
    const QString selectedFile = fileDialog.selectedFiles().first();

    QStringList splittedName = selectedFile.split(".");
    QString extension = splittedName[((splittedName.size()) - 1)].toUpper();

    QStringList validFileExtensions;
    validFileExtensions << "JPG" << "PNG";

    if (validFileExtensions.contains(extension))
    {
      int witdh = m_renderer->GetRenderWindow()->GetSize()[0];
      vtkRenderLargeImage *image = vtkRenderLargeImage::New();
      image->SetInput(m_renderer);
      image->SetMagnification(4096.0/witdh+0.5);
      image->Update();

      if (QString("PNG") == extension)
      {
        vtkPNGWriter *writer = vtkPNGWriter::New();
        writer->SetFileDimensionality(2);
        writer->SetFileName(selectedFile.toStdString().c_str());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }

      if (QString("JPG") == extension)
      {
        vtkJPEGWriter *writer = vtkJPEGWriter::New();
        writer->SetQuality(100);
        writer->ProgressiveOff();
        writer->WriteToMemoryOff();
        writer->SetFileDimensionality(2);
        writer->SetFileName(selectedFile.toStdString().c_str());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }
    }
    else
    {
      QMessageBox msgBox;
      QString message(tr("Snapshot not exported. Unrecognized extension "));
      message.append(extension).append(".");
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.setText(message);
      msgBox.exec();
    }
  }
}

//-----------------------------------------------------------------------------
VolumeView::Settings::Settings(const QString prefix, VolumeView* parent)
: RENDERERS(prefix + "VolumeView::renderers")
{
  this->parent = parent;
  QSettings settings(CESVIMA, ESPINA);

  if (!settings.contains(RENDERERS))
    settings.setValue(RENDERERS, QStringList() << "Crosshairs" << "Volumetric" << "Mesh");

  /*BUG TODO 2012-10-05
  QMap<QString, Renderer *> renderers = EspinaFactory::instance()->renderers();
  foreach(QString name, settings.value(RENDERERS).toStringList())
  {
    Renderer *renderer = renderers.value(name, NULL);
    if (renderer)
      m_renderers << renderer;
  }
  */
}

//-----------------------------------------------------------------------------
void VolumeView::Settings::setRenderers(QList<Renderer *> values)
{
  QSettings settings(CESVIMA, ESPINA);
  QStringList activeRenderersNames;
  QList<Renderer *> activeRenderers;

  // remove controls for unused renderers
  foreach(Renderer *oldRenderer, m_renderers)
  {
    bool selected = false;
    int i = 0;
    while (!selected && i < values.size())
      selected = values[i++]->name() == oldRenderer->name();

    if (!selected)
    {
      parent->removeRendererControls(oldRenderer->name());
      if (!oldRenderer->isHidden())
      {
        oldRenderer->hide();
        parent->countEnabledRenderers(false);
      }
      oldRenderer->setVtkRenderer(NULL);
    }
    else
      activeRenderers << oldRenderer;
  }

  // add controls for added renderers
  foreach(Renderer *renderer, values)
  {
    activeRenderersNames << renderer->name();
    if (!activeRenderers.contains(renderer))
    {
      activeRenderers << renderer;
      parent->addRendererControls(renderer->clone());
    }
  }

  settings.setValue(RENDERERS, activeRenderersNames);
  settings.sync();
  m_renderers = activeRenderers;
}

//-----------------------------------------------------------------------------
QList<Renderer*> VolumeView::Settings::renderers() const
{
  return m_renderers;
}

//-----------------------------------------------------------------------------
void VolumeView::changePlanePosition(PlaneType plane, Nm dist)
{
  foreach(Renderer* ren, m_itemRenderers)
    if (QString("Crosshairs") == ren->name())
    {
      CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren);
      crossren->setPlanePosition(plane, dist);
    }
}

//-----------------------------------------------------------------------------
void VolumeView::countEnabledRenderers(bool value)
{
  if ((true == value) && (0 == this->m_numEnabledRenders))
  {
    m_renderer->ResetCamera();
    updateView();
  }

  if (true == value)
    m_numEnabledRenders++;
  else
    m_numEnabledRenders--;

}

//-----------------------------------------------------------------------------
void VolumeView::updateSelection(ViewManager::Selection selection)
{
  updateSegmentationRepresentations();
}

//-----------------------------------------------------------------------------
void VolumeView::updateSegmentationRepresentations()
{
  if (isVisible())
    foreach(Segmentation *seg, m_segmentations)
      updateSegmentation(seg);
}
