/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "RenderView.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/VolumeBounds.h>
#include <GUI/ColorEngines/NumberColorEngine.h>
#include <GUI/Extension/Visualization/VisualizationState.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/Representations/Frame.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// VTK
#include <vtkMath.h>
#include <QVTKWidget.h>
#include <vtkCoordinate.h>
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkRenderer.h>

// Qt
#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::View;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;

//-----------------------------------------------------------------------------
RenderView::RenderView(ViewState &state, ViewType type)
: SelectableView(state)
, m_view {new QVTKWidget()}
, m_state(state)
, m_selection{state.selection()}
, m_type {type}
, m_latestFrame{Frame::InvalidFrame()}
{
  connectSignals();
}

//-----------------------------------------------------------------------------
RenderView::~RenderView()
{
  delete m_view;
}

//-----------------------------------------------------------------------------
TimeStamp RenderView::timeStamp() const
{
  return m_state.timer().timeStamp();
}

//-----------------------------------------------------------------------------
void RenderView::addRepresentationManager(RepresentationManagerSPtr manager)
{
  connect(&m_state,      SIGNAL(frameChanged(GUI::Representations::FrameCSPtr)),
          manager.get(), SLOT(onFrameChanged(GUI::Representations::FrameCSPtr)));

  connect(manager.get(), SIGNAL(renderRequested()),
          this,          SLOT(onRenderRequest()), Qt::QueuedConnection);

  configureManager(manager);

  manager->setView(this, m_latestFrame);

  m_managers << manager;
}

//-----------------------------------------------------------------------------
void RenderView::removeRepresentationManager(RepresentationManagerSPtr manager)
{
  if (m_managers.removeOne(manager))
  {
    disconnect(&m_state,      SIGNAL(frameChanged(GUI::Representations::FrameCSPtr)),
               manager.get(), SLOT(onFrameChanged(GUI::Representations::FrameCSPtr)));

    disconnect(manager.get(), SIGNAL(renderRequested()),
               this,          SLOT(onRenderRequest()));
  }
}

//-----------------------------------------------------------------------------
NmVector3 RenderView::toWorldCoordinates(vtkRenderer *renderer, int x, int y, int z) const
{
  Q_ASSERT(renderer);

  renderer->SetDisplayPoint(x, y, z);
  renderer->DisplayToWorld();

  double worldPoint[4];
  renderer->GetWorldPoint(worldPoint);

  return NmVector3(worldPoint);
}

//-----------------------------------------------------------------------------
void RenderView::onSelectionSet(SelectionSPtr selection)
{
  connect(selection.get(), SIGNAL(selectionStateChanged(SegmentationAdapterList)),
          this,            SLOT(updateSelection(SegmentationAdapterList)));
}

//-----------------------------------------------------------------------------
void RenderView::selectPickedItems(int x, int y, bool append)
{
  ViewItemAdapterList selection;

  if (append)
  {
    selection = currentSelection()->items();
  }

  auto flags       = Selector::SelectionFlags(Selector::CHANNEL|Selector::SEGMENTATION);
  auto pickedItems = pick(flags, x, y);

  ViewItemAdapterList channels;
  ViewItemAdapterList segmentations;

  for (auto selectionItem : pickedItems)
  {
    auto pickedItem = dynamic_cast<ViewItemAdapterPtr>(selectionItem.second);

    if (pickedItem->isVisible())
    {
      if (isSegmentation(pickedItem))
      {
        segmentations << pickedItem;
      }
      else if (isChannel(pickedItem))
      {
        channels << pickedItem;
      }
    }
  }

  for (auto item : segmentations + channels)
  {
    if (selection.contains(item))
    {
      selection.removeAll(item);
    }
    else
    {
      selection << item;
    }

    if (!append) break;
  }

  currentSelection()->set(selection);
}

//-----------------------------------------------------------------------------
void RenderView::takeSnapshot()
{
  auto title      = tr("Save scene as image");
  auto suggestion = tr("snapshot.png");
  auto formats    = SupportedFormats(tr("PNG Image"),  "png")
                        .addFormat(tr("JPEG Image"), "jpg");
  auto fileName   = DefaultDialogs::SaveFile(title, formats, "", ".png", suggestion);

  if (!fileName.isEmpty())
  {
    QStringList splittedName = fileName.split(".");
    QString extension = splittedName[((splittedName.size()) - 1)].toUpper();

    QStringList validFileExtensions;
    validFileExtensions << "JPG" << "PNG";

    if (validFileExtensions.contains(extension))
    {
      // avoid artifacts when acquiring the image
      int offScreenRender = renderWindow()->GetOffScreenRendering();
      renderWindow()->SetOffScreenRendering(true);

      vtkSmartPointer<vtkWindowToImageFilter> image = vtkSmartPointer<vtkWindowToImageFilter>::New();
      image->SetInput(renderWindow());
      image->SetMagnification(4096.0/renderWindow()->GetSize()[0]+0.5);
      image->Update();

      renderWindow()->SetOffScreenRendering(offScreenRender);

      if (QString("PNG") == extension)
      {
        vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetFileDimensionality(2);
        writer->SetFileName(fileName.toUtf8());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }

      if (QString("JPG") == extension)
      {
        vtkSmartPointer<vtkJPEGWriter> writer = vtkSmartPointer<vtkJPEGWriter>::New();
        writer->SetQuality(100);
        writer->ProgressiveOff();
        writer->WriteToMemoryOff();
        writer->SetFileDimensionality(2);
        writer->SetFileName(fileName.toUtf8());
        writer->SetInputConnection(image->GetOutputPort());
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        writer->Write();
        QApplication::restoreOverrideCursor();
      }
    }
    else
    {
      auto message = tr("Snapshot not exported. Unrecognized extension ");

      DefaultDialogs::InformationMessage(message, title);
    }
  }
}

//-----------------------------------------------------------------------------
bool RenderView::hasVisibleRepresentations() const
{
  for (auto manager : m_managers)
  {
    if (manager->hasActors()) return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
GUI::View::ViewState &RenderView::state() const
{
  return m_state;
}

//-----------------------------------------------------------------------------
vtkRenderWindow* RenderView::renderWindow() const
{
  return m_view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
const NmVector3 RenderView::crosshair() const
{
  return m_state.crosshair();
}

//-----------------------------------------------------------------------------
void RenderView::eventPosition(int& x, int& y)
{
  x = y = -1;

  if (mainRenderer())
  {
    auto rwi = renderWindow()->GetInteractor();
    Q_ASSERT(rwi);
    rwi->GetEventPosition(x, y);
  }
}

//----------------------------------------------------------------------------
NmVector3 RenderView::worldEventPosition()
{
  int x,y;
  eventPosition(x,y);

  auto coords = vtkSmartPointer<vtkCoordinate>::New();

  coords->SetCoordinateSystemToDisplay();
  coords->SetValue(x, y, 0);

  double *displayCoords = coords->GetComputedWorldValue(mainRenderer());

  NmVector3 position{displayCoords[0], displayCoords[1], displayCoords[2]};

  normalizeWorldPosition(position);

  return position;
}

//-----------------------------------------------------------------------------
NmVector3 RenderView::worldEventPosition(const QPoint &pos)
{
  renderWindow()->GetInteractor()->SetEventPositionFlipY(pos.x(), pos.y());

  return worldEventPosition();
}

//-----------------------------------------------------------------------------
void RenderView::reset()
{
  resetImplementation();

  onRenderRequest();
}

//-----------------------------------------------------------------------------
void RenderView::resetCamera()
{
  auto frame = m_state.createFrame();

  frame->reset = true;

  onCameraReset(frame);
  refresh();
}

//-----------------------------------------------------------------------------
Selector::Selection RenderView::pick(const Selector::SelectionFlags flags, const NmVector3 &point, bool multiselection) const
{
  auto coords = vtkSmartPointer<vtkCoordinate>::New();

  coords->SetCoordinateSystemToWorld();
  coords->SetValue(point[0], point[1], point[2]);

  int *displayCoords = coords->GetComputedDisplayValue(mainRenderer());

  return pick(flags, displayCoords[0], displayCoords[1], multiselection);
}

//-----------------------------------------------------------------------------
Selector::Selection RenderView::pick(const Selector::SelectionFlags flags, const int x, const int y, bool multiselection) const
{
  return pickImplementation(flags, x, y, multiselection);
}

//-----------------------------------------------------------------------------
void RenderView::onCameraReset(FrameCSPtr frame)
{
  //TODO
}

//-----------------------------------------------------------------------------
void RenderView::refresh()
{
  onRenderRequest();
}

//-----------------------------------------------------------------------------
void RenderView::connectSignals()
{
  connect(this,     SIGNAL(crosshairChanged(NmVector3)),
          &m_state, SLOT(setCrosshair(NmVector3)));

  connect(this,     SIGNAL(crosshairPlaneChanged(Plane,Nm)),
          &m_state, SLOT(setCrosshairPlane(Plane,Nm)));

  connect(this,     SIGNAL(viewFocusedOn(NmVector3)),
          &m_state, SLOT(focusViewOn(NmVector3)));

  connect(&m_state, SIGNAL(resetCamera(GUI::Representations::FrameCSPtr)),
          this,     SLOT(onCameraReset(GUI::Representations::FrameCSPtr)));

//   connect(&m_state, SIGNAL(refreshRequested()),
//           this,     SLOT(refresh()));

  connect(&m_state, SIGNAL(frameChanged(GUI::Representations::FrameCSPtr)),
          this,     SLOT(onCrosshairChanged(GUI::Representations::FrameCSPtr)));

  connect(&m_state, SIGNAL(widgetsAdded(GUI::Representations::Managers::TemporalPrototypesSPtr,GUI::Representations::FrameCSPtr)),
          this,     SLOT(onWidgetsAdded(GUI::Representations::Managers::TemporalPrototypesSPtr,GUI::Representations::FrameCSPtr)));

  connect(&m_state, SIGNAL(widgetsRemoved(GUI::Representations::Managers::TemporalPrototypesSPtr,GUI::Representations::FrameCSPtr)),
          this,     SLOT(onWidgetsRemoved(GUI::Representations::Managers::TemporalPrototypesSPtr,GUI::Representations::FrameCSPtr)));

  connect(&m_state, SIGNAL(sliceSelectorAdded(SliceSelectorSPtr,SliceSelectionType)),
          this,     SLOT(addSliceSelectors(SliceSelectorSPtr,SliceSelectionType)));

  connect(&m_state, SIGNAL(sliceSelectorRemoved(SliceSelectorSPtr)),
          this,     SLOT(removeSliceSelectors(SliceSelectorSPtr)));

//   connect(&m_state, SIGNAL(viewFocusChanged(NmVector3)),
//           this,     SLOT(onFocusChanged(NmVector3)));

  connect(m_state.coordinateSystem().get(), SIGNAL(resolutionChanged(NmVector3)),
          this,                             SLOT(onSceneResolutionChanged(NmVector3)));

  connect (m_state.coordinateSystem().get(), SIGNAL(boundsChanged(Bounds)),
           this,                             SLOT(onSceneBoundsChanged(Bounds)));
}

// //-----------------------------------------------------------------------------
// void RenderView::onFocusChanged(NmVector3 point)
// {
//   m_focusPoint = point;
//   m_requiresFocusChange = true;
//
//   onRenderRequest();
// }

//-----------------------------------------------------------------------------
void RenderView::onWidgetsAdded(TemporalPrototypesSPtr prototypes, const GUI::Representations::FrameCSPtr frame)
{
  if (prototypes->supportedViews().testFlag(m_type))
  {
    if(!m_temporalManagers.contains(prototypes))
    {
      auto manager = std::make_shared<TemporalManager>(prototypes);

      addRepresentationManager(manager);

      manager->show(frame);

      m_temporalManagers[prototypes] = manager;
    }
    else
    {
      qWarning() << "Tried to add already present prototypes.";
    }
  }
}

//-----------------------------------------------------------------------------
void RenderView::onWidgetsRemoved(TemporalPrototypesSPtr prototypes, const GUI::Representations::FrameCSPtr frame)
{
  if (prototypes->supportedViews().testFlag(m_type))
  {
    if(m_temporalManagers.contains(prototypes))
    {
      auto manager = m_temporalManagers[prototypes];

      manager->hide(frame);

    //NOTE: managers should be removed from m_temporalManagers after processing render
    //      request of t so they can hide its representations
    }
    else
    {
      qWarning() << "Trying to remove a non existent manager";
    }
  }
}

//-----------------------------------------------------------------------------
void RenderView::onRenderRequest()
{
  if (!isVisible()) return;

  auto readyManagers = pendingManagers();

  auto frame = latestReadyFrame(readyManagers);

  if (m_latestFrame->time < frame->time)
  {
    //     qDebug() << viewName() << "Rendering period" << m_timer.elapsed();
    //     m_timer.restart();
    display(readyManagers, frame);

    //qDebug() << viewName() << ": Update actors:" << renderTime;

    m_state.timer().activate();

    deleteInactiveWidgetManagers();

    if (hasVisibleRepresentations() && frame->reset)
    {
      resetCameraImplementation();
      //qDebug() << viewName() << ": Reset camera:" << renderTime;
    }

    if (frame->focus)
    {
      moveCamera(frame->crosshair);
    }

    updateViewActions(managerFlags());

    refreshViewImplementation();

    //     qDebug() << viewName() << "Rendering frame" << renderTime;
    mainRenderer()->ResetCameraClippingRange();
    m_view->update();

    m_latestFrame = frame;
  }
}

//-----------------------------------------------------------------------------
const NmVector3 RenderView::sceneResolution() const
{
  return m_state.coordinateSystem()->resolution();
}

//-----------------------------------------------------------------------------
const Bounds RenderView::sceneBounds() const
{
  return m_state.coordinateSystem()->bounds();
}

//-----------------------------------------------------------------------------
QPushButton* RenderView::createButton(const QString& icon, const QString& tooltip)
{
  const int BUTTON_SIZE = 22;
  const int ICON_SIZE   = 20;

  auto button = new QPushButton();

  button->setIcon(QIcon(icon));
  button->setToolTip(tooltip);
  button->setFlat(true);
  button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
  button->setMinimumSize(QSize(BUTTON_SIZE, BUTTON_SIZE));
  button->setMaximumSize(QSize(BUTTON_SIZE, BUTTON_SIZE));
  button->setEnabled(false);
  button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  return button;
}

//-----------------------------------------------------------------------------
RepresentationManagerSList RenderView::pendingManagers() const
{
  RepresentationManagerSList result;

  result << pendingManagers(m_managers);

  return result;
}

//-----------------------------------------------------------------------------
RepresentationManagerSList RenderView::pendingManagers(RepresentationManagerSList managers) const
{
  RepresentationManagerSList result;

  for (auto manager : managers)
  {
    if (!manager->isIdle())
    {
      result << manager;
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
FrameCSPtr RenderView::latestReadyFrame(RepresentationManagerSList managers) const
{
  TimeStamp latest = Timer::INVALID_TIME_STAMP;

  RepresentationManagerSPtr activeManager;

  if (!managers.isEmpty())
  {
    QMap<TimeStamp, unsigned> count;

    unsigned activeManagers = 0;

    for (auto manager : managers)
    {
      Q_ASSERT(!manager->isIdle());

      if (manager->isActive())
      {
        activeManager = manager;
        activeManagers++;

        for(auto time: manager->readyRange())
        {
          count[time] = count.value(time, 0) + 1;
        }
      }
    }


    if (activeManagers > 0)
    {
      //qDebug() << viewName() << "Frames Availabe: " << count;

      for(auto time : count.keys())
      {
        // We allow display of managers which are pending a hide request
        // even it is not synched with the current frame
        if(count[time] == activeManagers)
        {
          if(time > latest)
          {
            latest = time;
          }
        }
      }
    }
//     else
//     {
//       // We don't care about the actual frame as all managers are pending to
//       // hide their actors. We only need to be greater than the last render
//       // time stamp so it actually performs a display on every manager
//       latest = m_lastRender + 1;
//     }

    //qDebug() << viewName() << "Latest common frame "<< latest;
  }

  return (activeManager && latest != Timer::INVALID_TIME_STAMP)?activeManager->frame(latest):Frame::InvalidFrame();
}

//-----------------------------------------------------------------------------
void RenderView::display(RepresentationManagerSList managers, const GUI::Representations::FrameCSPtr frame)
{
  for (auto manager : managers)
  {
    manager->display(frame);
  }
}

//-----------------------------------------------------------------------------
RepresentationManager::ManagerFlags RenderView::managerFlags() const
{
  RepresentationManager::ManagerFlags flags;

  for (auto manager : m_managers)
  {
    if(manager->isActive())
    {
      flags |= manager->flags();
    }
  }

  return flags;
}

//-----------------------------------------------------------------------------
void RenderView::deleteInactiveWidgetManagers()
{
  auto factories = m_temporalManagers.keys();

  for (auto factory : factories)
  {
    if (!m_temporalManagers[factory]->isActive())
    {
      auto manager = m_temporalManagers[factory];

      removeRepresentationManager(manager);

      m_temporalManagers.remove(factory);
    }
  }
}

//-----------------------------------------------------------------------------
void RenderView::showSegmentationTooltip(const int x, const int y)
{
  auto segmentations = pick(Selector::SEGMENTATION, x, y);

  QString toopTip;

  for (auto segmentation : segmentations)
  {
    toopTip = toopTip.append(segmentation.second->data(Qt::ToolTipRole).toString());
  }

  m_view->setToolTip(toopTip);
}

//-----------------------------------------------------------------------------
EventHandlerSPtr RenderView::eventHandler() const
{
  return state().eventHandler();
}

//-----------------------------------------------------------------------------
bool RenderView::eventHandlerFilterEvent(QEvent *event)
{
  return eventHandler() && eventHandler()->filterEvent(event, this);
}
