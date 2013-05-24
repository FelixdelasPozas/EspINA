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

// EspINA
#include "GUI/Renderers/CrosshairRenderer.h"
#include <Core/EspinaSettings.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaFactory.h>

// QT
#include <QEvent>
#include <QSettings>
#include <QFileDialog>
#include <QPushButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDebug>

// VTK
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <QVTKWidget.h>
#include <vtkCamera.h>
#include <vtkPOVExporter.h>
#include <vtkVRMLExporter.h>
#include <vtkX3DExporter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkRenderLargeImage.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>
#include <vtkMath.h>

// Qt
#include <QApplication>
#include "GUI/Representations/CrosshairRepresentation.h"

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeView::VolumeView(const EspinaFactory *factory,
                       ViewManager *viewManager,
                       bool additionalScrollBars,
                       QWidget* parent)
: EspinaRenderView      (viewManager, parent)
, m_mainLayout          (new QVBoxLayout())
, m_controlLayout       (new QHBoxLayout())
, m_additionalScrollBars(additionalScrollBars)
, m_settings            (new Settings(factory, QString(), this))
{
  setupUI();

  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);

  memset(m_center,0,3*sizeof(double));
  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
          this, SLOT(updateSelection(ViewManager::Selection,bool)));

  viewManager->registerView(this);
}

//-----------------------------------------------------------------------------
VolumeView::~VolumeView()
{
//   qDebug() << "********************************************************";
//   qDebug() << "              Destroying Volume View";
//   qDebug() << "********************************************************";

  // Representation destructors may need to access slice view in their destructors
  m_channelStates.clear();
  m_segmentationStates.clear();

  m_viewManager->unregisterView(this);
}

//-----------------------------------------------------------------------------
void VolumeView::reset()
{
  foreach(EspinaWidget *widget, m_widgets.keys())
  {
    removeWidget(widget);
  }

  foreach(SegmentationPtr segmentation, m_segmentationStates.keys())
  {
    removeSegmentation(segmentation);
  }

  // After removing segmentations there should be only channels
  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    removeChannel(channel);
  }

  Q_ASSERT(m_channelStates.isEmpty());
  Q_ASSERT(m_segmentationStates.isEmpty());
  Q_ASSERT(m_widgets.isEmpty());

  QMap<QPushButton *, IRendererSPtr>::iterator it = m_renderers.begin();
  while (it != m_renderers.end())
  {
    it.key()->setChecked(false);
    ++it;
  }

  m_numEnabledChannelRenders = 0;
  m_numEnabledSegmentationRenders = 0;
}

//-----------------------------------------------------------------------------
void VolumeView::addRendererControls(IRendererSPtr renderer)
{
  QPushButton *button;

  button = new QPushButton(renderer->icon(), "");
  button->setFlat(true);
  button->setCheckable(true);
  button->setChecked(false);
  button->setIconSize(QSize(22, 22));
  button->setMaximumSize(QSize(32, 32));
  button->setToolTip(renderer.get()->tooltip());
  button->setObjectName(renderer.get()->name());
  connect(button, SIGNAL(toggled(bool)), renderer.get(), SLOT(setEnable(bool)));
  connect(button, SIGNAL(toggled(bool)), this, SLOT(updateEnabledRenderersCount(bool)));
  connect(button, SIGNAL(destroyed(QObject*)), renderer.get(), SLOT(deleteLater()));
  connect(renderer.get(), SIGNAL(renderRequested()), this, SLOT(updateView()));
  m_controlLayout->addWidget(button);
  renderer->setViewData(this, this->m_renderer);

  // add representations to renderer
  foreach(SegmentationPtr segmentation, m_segmentationStates.keys())
  {
    if (renderer->itemCanBeRendered(segmentation))
      foreach(GraphicalRepresentationSPtr rep, m_segmentationStates[segmentation].representations)
         if (renderer->managesRepresentation(rep)) renderer->addRepresentation(segmentation, rep);
  }

  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    if (renderer->itemCanBeRendered(channel))
      foreach(GraphicalRepresentationSPtr rep, m_channelStates[channel].representations)
        if (renderer->managesRepresentation(rep)) renderer->addRepresentation(channel, rep);
  }

  m_itemRenderers << renderer;
  m_renderers[button] = renderer;

  if (!renderer->isHidden())
  {
    if (renderer->getRenderableItemsType().testFlag(EspINA::SEGMENTATION))
      this->m_numEnabledSegmentationRenders++;

    if (renderer->getRenderableItemsType().testFlag(EspINA::CHANNEL))
      this->m_numEnabledChannelRenders++;
  }

  if (0 != m_numEnabledSegmentationRenders)
  {
    QMap<EspinaWidget *, vtkAbstractWidget *>::const_iterator it = m_widgets.begin();
    for( ; it != m_widgets.end(); ++it)
    {
      if (it.key()->manipulatesSegmentations())
      {
        it.value()->SetEnabled(true);
        it.value()->GetRepresentation()->SetVisibility(true);
      }
    }
  }

  updateRenderersControls();
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

  IRendererSPtr removedRenderer;
  foreach (IRendererSPtr renderer, m_itemRenderers)
  {
    if (renderer->name() == name)
    {
      removedRenderer = renderer;
      break;
    }
  }

  if (removedRenderer)
  {
    if (!removedRenderer->isHidden())
      removedRenderer->hide();

    if (!removedRenderer->isHidden() && (removedRenderer->getRenderableItemsType().testFlag(EspINA::SEGMENTATION)))
      this->m_numEnabledSegmentationRenders--;

    if (!removedRenderer->isHidden() && (removedRenderer->getRenderableItemsType().testFlag(EspINA::CHANNEL)))
      this->m_numEnabledChannelRenders--;

    if (0 == m_numEnabledSegmentationRenders)
    {
      QMap<EspinaWidget *, vtkAbstractWidget *>::const_iterator it = m_widgets.begin();
      for( ; it != m_widgets.end(); ++it)
      {
        if (it.key()->manipulatesSegmentations())
        {
          it.value()->SetEnabled(false);
          it.value()->GetRepresentation()->SetVisibility(false);
        }
      }
    }

    QMap<QPushButton*, IRendererSPtr>::iterator it = m_renderers.begin();
    bool erased = false;
    while(!erased && it != m_renderers.end())
    {
      if (it.value() == removedRenderer)
      {
        m_renderers.erase(it);
        erased = true;
      }
      else
        ++it;
    }

    m_itemRenderers.removeAll(removedRenderer);
  }

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void VolumeView::buildControls()
{
  m_controlLayout = new QHBoxLayout();
  m_controlLayout->addStretch();

  m_zoom.setIcon(QIcon(":/espina/zoom_reset.png"));
  m_zoom.setToolTip(tr("Reset view's camera"));
  m_zoom.setFlat(true);
  m_zoom.setIconSize(QSize(22,22));
  m_zoom.setMaximumSize(QSize(32,32));
  m_zoom.setCheckable(false);
  connect(&m_zoom, SIGNAL(clicked()), this, SLOT(resetView()));

  m_snapshot.setIcon(QIcon(":/espina/snapshot_scene.svg"));
  m_snapshot.setToolTip(tr("Save Scene as Image"));
  m_snapshot.setFlat(true);
  m_snapshot.setIconSize(QSize(22,22));
  m_snapshot.setMaximumSize(QSize(32,32));
  m_snapshot.setEnabled(false);
  connect(&m_snapshot,SIGNAL(clicked(bool)),this,SLOT(takeSnapshot()));

  m_export.setIcon(QIcon(":/espina/export_scene.svg"));
  m_export.setToolTip(tr("Export 3D Scene"));
  m_export.setFlat(true);
  m_export.setIconSize(QSize(22,22));
  m_export.setMaximumSize(QSize(32,32));
  m_export.setEnabled(false);
  connect(&m_export,SIGNAL(clicked(bool)),this,SLOT(exportScene()));

  QSpacerItem * horizontalSpacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  m_controlLayout->addWidget(&m_zoom);
  m_controlLayout->addWidget(&m_snapshot);
  m_controlLayout->addWidget(&m_export);
  m_controlLayout->addItem(horizontalSpacer);

  foreach(IRenderer *renderer, m_settings->renderers())
    if (renderer->getRenderType().testFlag(IRenderer::RENDERER_VOLUMEVIEW))
      this->addRendererControls(renderer->clone());

  m_mainLayout->addLayout(m_controlLayout);
}


//-----------------------------------------------------------------------------
void VolumeView::centerViewOn(Nm *center, bool notUsed)
{
  if (!isVisible() ||
      (m_center[0] == center[0] &&
       m_center[1] == center[1] &&
       m_center[2] == center[2]))
    return;

  memcpy(m_center, center, 3*sizeof(double));

  foreach(IRendererSPtr ren, m_itemRenderers)
  {
    if (QString("Crosshairs") == ren->name())
    {
      CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren.get());
      crossren->setCrosshair(center);
    }
  }

  if (m_additionalScrollBars && !m_channelStates.empty())
  {
    Nm minSpacing[3];
    m_channelStates.begin().key()->volume()->spacing(minSpacing);

    foreach(ChannelPtr channel, m_channelStates.keys())
    {
      double spacing[3];
      channel->volume()->spacing(spacing);
      if (spacing[0] < minSpacing[0])
        minSpacing[0] = spacing[0];

      if (spacing[1] < minSpacing[1])
        minSpacing[1] = spacing[1];

      if (spacing[2] < minSpacing[2])
        minSpacing[2] = spacing[2];
    }

    int iCenter[3] = { vtkMath::Round(center[0]/minSpacing[0]), vtkMath::Round(center[1]/minSpacing[1]), vtkMath::Round(center[2]/minSpacing[2]) };
    m_axialScrollBar->blockSignals(true);
    m_coronalScrollBar->blockSignals(true);
    m_sagittalScrollBar->blockSignals(true);
    m_axialScrollBar->setValue(iCenter[0]);
    m_coronalScrollBar->setValue(iCenter[1]);
    m_sagittalScrollBar->setValue(iCenter[2]);
    m_axialScrollBar->blockSignals(false);
    m_coronalScrollBar->blockSignals(false);
    m_sagittalScrollBar->blockSignals(false);
  }

  setCameraFocus(center);
  updateView();
}

//-----------------------------------------------------------------------------
void VolumeView::setCameraFocus(const Nm center[3])
{
  m_renderer->GetActiveCamera()->SetFocalPoint(center[0],center[1],center[2]);
  m_renderer->ResetCameraClippingRange();
}

//-----------------------------------------------------------------------------
void VolumeView::resetCamera()
{
  this->m_renderer->GetActiveCamera()->SetViewUp(0,1,0);
  this->m_renderer->GetActiveCamera()->SetPosition(0,0,-1);
  this->m_renderer->GetActiveCamera()->SetFocalPoint(0,0,0);
  this->m_renderer->GetActiveCamera()->SetRoll(180);
  this->m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void VolumeView::addChannel(ChannelPtr channel)
{
  EspinaRenderView::addChannel(channel);

  updateRenderersControls();
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
void VolumeView::removeChannel(ChannelPtr channel)
{
  EspinaRenderView::removeChannel(channel);

  updateRenderersControls();
  updateScrollBarsLimits();
}

//-----------------------------------------------------------------------------
bool VolumeView::updateChannelRepresentation(ChannelPtr channel, bool render)
{
  bool returnVal = EspinaRenderView::updateChannelRepresentation(channel, render);

  updateRenderersControls();
  return returnVal;
}

//-----------------------------------------------------------------------------
bool VolumeView::updateSegmentationRepresentation(SegmentationPtr seg, bool render)
{
  bool returnVal = EspinaRenderView::updateSegmentationRepresentation(seg, render);

  updateRenderersControls();
  return returnVal;
}


//-----------------------------------------------------------------------------
void VolumeView::addWidget(EspinaWidget* eWidget)
{
  if(m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = eWidget->create3DWidget(this);
  if (!widget)
    return;

  widget->SetCurrentRenderer(this->m_renderer);
  widget->SetInteractor(m_view->GetInteractor());

  bool activate = (m_numEnabledSegmentationRenders != 0);
  if (eWidget->manipulatesSegmentations())
  {
    widget->SetEnabled(activate);
    widget->GetRepresentation()->SetVisibility(activate);
  }

  m_renderer->ResetCameraClippingRange();
  m_widgets[eWidget] = widget;

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void VolumeView::removeWidget(EspinaWidget* eWidget)
{
  if (!m_widgets.contains(eWidget))
    return;

  vtkAbstractWidget *widget = m_widgets[eWidget];
  widget->Off();
  widget->SetInteractor(NULL);
  widget->RemoveAllObservers();
  m_widgets.remove(eWidget);

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
ISelector::PickList VolumeView::pick(ISelector::PickableItems filter, ISelector::DisplayRegionList regions)
{
  return ISelector::PickList();
}

//-----------------------------------------------------------------------------
void VolumeView::setupUI()
{
  if (m_additionalScrollBars)
  {
    m_additionalGUI = new QHBoxLayout();
    m_axialScrollBar = new QScrollBar(Qt::Horizontal);
    m_axialScrollBar->setEnabled(false);
    m_axialScrollBar->setFixedHeight(15);
    m_axialScrollBar->setToolTip("Axial scroll bar");
    connect(m_axialScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));

    m_coronalScrollBar = new QScrollBar(Qt::Vertical);
    m_coronalScrollBar->setEnabled(false);
    m_coronalScrollBar->setFixedWidth(15);
    m_coronalScrollBar->setToolTip("Coronal scroll bar");
    connect(m_coronalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));

    m_sagittalScrollBar = new QScrollBar(Qt::Vertical);
    m_sagittalScrollBar->setEnabled(false);
    m_sagittalScrollBar->setFixedWidth(15);
    m_sagittalScrollBar->setToolTip("Sagittal scroll bar");
    connect(m_sagittalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarMoved(int)));

    updateScrollBarsLimits();

    m_additionalGUI->insertWidget(0, m_coronalScrollBar,0);
    m_additionalGUI->insertWidget(1, m_view,1);
    m_additionalGUI->insertWidget(2, m_sagittalScrollBar,0);

    m_mainLayout->insertLayout(0, m_additionalGUI, 1);
    m_mainLayout->insertWidget(1, m_axialScrollBar, 0);
  }
  else
  {
    m_mainLayout->insertWidget(0,m_view);
  }

  m_view->show();
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->LightFollowCameraOn();
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorstyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  interactorstyle->AutoAdjustCameraClippingRangeOn();
  interactorstyle->KeyPressActivationOff();
  m_view->GetRenderWindow()->AddRenderer(m_renderer);
  m_view->GetRenderWindow()->GetInteractor()->SetInteractorStyle(interactorstyle);
  m_view->GetRenderWindow()->Render();
  m_view->installEventFilter(this);

  buildControls();
}

//-----------------------------------------------------------------------------
void VolumeView::updateView()
{
  if(isVisible())
  {
    this->m_view->GetRenderWindow()->Render();
    this->m_view->update();
  }
}

//-----------------------------------------------------------------------------
void VolumeView::selectPickedItems(int vx, int vy, bool append)
{

  ViewManager::Selection selection, pickedItems;
  if (append)
    selection = m_viewManager->selection();

  // If no append, segmentations have priority over channels
  foreach(IRendererSPtr renderer, m_itemRenderers)
  {
    if (!renderer->isHidden() && (renderer->getRenderableItemsType().testFlag(EspINA::SEGMENTATION)))
    {
      pickedItems = renderer->pick(vx, vy, NULL, IRenderer::RenderabledItems(EspINA::SEGMENTATION), append);
      if (!pickedItems.empty())
      {
        foreach(PickableItemPtr item, pickedItems)
          if (!selection.contains(item))
            selection << item;
          else
            selection.removeAll(item);
      }
    }
  }

  pickedItems.clear();

  foreach(IRendererSPtr renderer, m_itemRenderers)
  {
    if (!renderer->isHidden() && (renderer->getRenderableItemsType().testFlag(EspINA::CHANNEL)))
    {
      pickedItems = renderer->pick(vx, vy, NULL, IRenderer::RenderabledItems(EspINA::CHANNEL), append);
      if (!pickedItems.empty())
      {
        foreach(PickableItemPtr item, pickedItems)
          if (!selection.contains(item))
            selection << item;
          else
            selection.removeAll(item);
      }
    }
  }

  if (!append && !selection.empty())
  {
    PickableItemPtr returnItem = selection.first();
    selection.clear();
    selection << returnItem;
  }
  m_viewManager->setSelection(selection);
}

//-----------------------------------------------------------------------------
bool VolumeView::eventFilter(QObject* caller, QEvent* e)
{
  // there is not a "singleclick" event so we need to remember the position of the
  // press event and compare it with the position of the release event (for picking).
  static int x = -1;
  static int y = -1;

  int newX, newY;
  eventPosition(newX, newY);

  if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      if (me->modifiers() == Qt::CTRL)
      {
        foreach(IRendererSPtr renderer, m_itemRenderers)
        {
          if (!renderer->isHidden())
          {
            ViewManager::Selection selection = renderer->pick(newX, newY, NULL, IRenderer::RenderabledItems(EspINA::SEGMENTATION|EspINA::CHANNEL), false);
            if (!selection.empty())
            {
              double point[3];
              renderer->getPickCoordinates(point);

              emit centerChanged(point[0], point[1], point[2]);
              break;
            }
          }
        }
      }
      else
      {
        x = newX;
        y = newY;
      }
    }
  }

  if (e->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent *me = static_cast<QMouseEvent*>(e);
    if ((me->button() == Qt::LeftButton) && !(me->modifiers() == Qt::CTRL))
      if ((newX == x) && (newY == y))
        selectPickedItems(newX, newY, me->modifiers() == Qt::SHIFT);
  }

  return QObject::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void VolumeView::exportScene()
{
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
        exporter->SetFileName(selectedFile.toUtf8());
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
        exporter->SetFileName(selectedFile.toUtf8());
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
        exporter->SetFileName(selectedFile.toUtf8());
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

//-----------------------------------------------------------------------------
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
        writer->SetFileName(selectedFile.toUtf8());
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
        writer->SetFileName(selectedFile.toUtf8());
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
void VolumeView::changePlanePosition(PlaneType plane, Nm dist)
{
  bool needUpdate = false;
  foreach(IRendererSPtr ren, m_itemRenderers)
  {
    if (QString("Crosshairs") == ren->name())
    {
      CrosshairRenderer *crossren = reinterpret_cast<CrosshairRenderer *>(ren.get());
      crossren->setPlanePosition(plane, dist);
      needUpdate = true;
    }
  }

  if (needUpdate)
    updateView();
}

//-----------------------------------------------------------------------------
void VolumeView::updateEnabledRenderersCount(bool value)
{
  int updateValue = (value ? +1 : -1);

  // reset camera if is the first renderer the user activates
  if ((true == value) && (0 == (m_numEnabledChannelRenders + m_numEnabledSegmentationRenders)))
  {
    m_renderer->ResetCamera();
    updateView();
  }

  // get enabled/disabled renderer that triggered the signal
  IRendererSPtr renderer;
  QPushButton *button = dynamic_cast<QPushButton*>(sender());
  if (button)
    renderer = m_renderers[button];

  if (renderer && (renderer->getRenderableItemsType().testFlag(EspINA::CHANNEL)))
  {
    m_numEnabledChannelRenders += updateValue;
    if (m_additionalScrollBars)
    {
      m_axialScrollBar->setEnabled(value);
      m_coronalScrollBar->setEnabled(value);
      m_sagittalScrollBar->setEnabled(value);
    }
  }

  if (renderer && (renderer->getRenderableItemsType().testFlag(EspINA::SEGMENTATION)))
  {
    m_numEnabledSegmentationRenders += updateValue;
    if (0 != m_numEnabledSegmentationRenders)
    {
      QMap<EspinaWidget *, vtkAbstractWidget *>::const_iterator it = m_widgets.begin();
      for( ; it != m_widgets.end(); ++it)
      {
        if (it.key()->manipulatesSegmentations())
        {
          it.value()->SetEnabled(value);
          it.value()->GetRepresentation()->SetVisibility(value);
        }
      }
    }
  }

  updateRenderersControls();
}

//-----------------------------------------------------------------------------
void VolumeView::updateRenderersControls()
{
  bool canTakeSnapshot = false;
  bool canBeExported = false;
  QMap<QPushButton *, IRendererSPtr>::iterator it;
  for(it = m_renderers.begin(); it != m_renderers.end(); ++it)
  {
    bool canRenderItems = it.value()->itemsBeenRendered() != 0;

    canTakeSnapshot |= (!it.value()->isHidden()) && canRenderItems;
    canBeExported |= canTakeSnapshot && (it.value()->getNumberOfvtkActors() != 0);

    it.key()->setEnabled(canRenderItems);
    if (!canRenderItems)
      it.key()->setChecked(false);
  }

  m_snapshot.setEnabled(canTakeSnapshot);
  m_export.setEnabled(canBeExported);
}

//-----------------------------------------------------------------------------
void VolumeView::scrollBarMoved(int value)
{
  Nm point[3];
  Nm minSpacing[3];

  Q_ASSERT(!m_channelStates.isEmpty());
  m_channelStates.keys().first()->volume()->spacing(minSpacing);

  foreach(ChannelPtr channel, m_channelStates.keys())
  {
    double spacing[3];
    channel->volume()->spacing(spacing);
    if (spacing[0] < minSpacing[0])
      minSpacing[0] = spacing[0];

    if (spacing[1] < minSpacing[1])
      minSpacing[1] = spacing[1];

    if (spacing[2] < minSpacing[2])
      minSpacing[2] = spacing[2];
  }

  point[0] = m_axialScrollBar->value() * minSpacing[0];
  point[1] = m_coronalScrollBar->value() * minSpacing[1];
  point[2] = m_sagittalScrollBar->value() * minSpacing[2];

  foreach(IRendererSPtr renderer, m_itemRenderers)
    if (renderer->getRenderableItemsType().testFlag(EspINA::CHANNEL))
    {
      CrosshairRenderer *crossRender = dynamic_cast<CrosshairRenderer *>(renderer.get());
      if (crossRender != NULL)
        crossRender->setCrosshair(point);
    }

  m_view->update();
}

//-----------------------------------------------------------------------------
void VolumeView::updateScrollBarsLimits()
{
  if (!m_additionalScrollBars)
    return;

  if(m_channelStates.isEmpty())
  {
    m_axialScrollBar->setMinimum(0);
    m_axialScrollBar->setMaximum(0);
    m_coronalScrollBar->setMinimum(0);
    m_coronalScrollBar->setMaximum(0);
    m_sagittalScrollBar->setMinimum(0);
    m_sagittalScrollBar->setMaximum(0);
    return;
  }

  int maxExtent[6];
  m_channelStates.keys().first()->volume()->extent(maxExtent);

  foreach (ChannelPtr channel, m_channelStates.keys())
  {
    int extent[6];
    channel->volume()->extent(extent);

    if (maxExtent[0] > extent[0])
      maxExtent[0] = extent[0];

    if (maxExtent[1] < extent[1])
      maxExtent[1] = extent[1];

    if (maxExtent[2] > extent[2])
      maxExtent[2] = extent[2];

    if (maxExtent[3] < extent[3])
      maxExtent[3] = extent[3];

    if (maxExtent[4] > extent[4])
      maxExtent[4] = extent[4];

    if (maxExtent[5] < extent[5])
      maxExtent[5] = extent[5];
  }

  m_axialScrollBar->setMinimum(maxExtent[0]);
  m_axialScrollBar->setMaximum(maxExtent[1]);
  m_coronalScrollBar->setMinimum(maxExtent[2]);
  m_coronalScrollBar->setMaximum(maxExtent[3]);
  m_sagittalScrollBar->setMinimum(maxExtent[4]);
  m_sagittalScrollBar->setMaximum(maxExtent[5]);
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr VolumeView::cloneRepresentation(GraphicalRepresentationSPtr prototype)
{
  GraphicalRepresentationSPtr rep = GraphicalRepresentationSPtr();

  if (prototype->canRenderOnView().testFlag(GraphicalRepresentation::RENDERABLEVIEW_VOLUME))
    rep = prototype->clone(this);

  return rep;
}

//-----------------------------------------------------------------------------
// VOLUMEVIEW::SETTINGS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
VolumeView::Settings::Settings(const EspinaFactory *factory,
                               const QString        prefix,
                               VolumeView          *parent)
: RENDERERS(prefix + "VolumeView::renderers")
{
  this->parent = parent;
  QSettings settings(CESVIMA, ESPINA);

  if (!settings.contains(RENDERERS))
    settings.setValue(RENDERERS, QStringList() << "Crosshairs" << "Volumetric" << "Mesh" << "Smoothed Mesh");

  QMap<QString, IRenderer *> renderers = factory->renderers();
  foreach(QString name, settings.value(RENDERERS).toStringList())
  {
    IRenderer *renderer = renderers.value(name, NULL);
    if (renderer)
      m_renderers << renderer;
  }
}

//-----------------------------------------------------------------------------
void VolumeView::Settings::setRenderers(IRendererList values)
{
  QSettings settings(CESVIMA, ESPINA);
  QStringList activeRenderersNames;
  IRendererList activeRenderers;

  // remove controls for unused renderers
  foreach(IRenderer *oldRenderer, m_renderers)
  {
    bool selected = false;
    int i = 0;
    while (!selected && i < values.size())
      selected = values[i++]->name() == oldRenderer->name();

    if (!selected)
      parent->removeRendererControls(oldRenderer->name());
    else
      activeRenderers << oldRenderer;
  }

  // add controls for added renderers
  foreach(IRenderer *renderer, values)
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
IRendererList VolumeView::Settings::renderers() const
{
  return m_renderers;
}
