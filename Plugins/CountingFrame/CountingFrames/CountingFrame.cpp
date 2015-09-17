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

// Plugin
#include "CountingFrames/CountingFrame.h"
#include "vtkCountingFrameSliceWidget.h"
#include "Extensions/CountingFrameExtension.h"

// ESPINA
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/VolumeBounds.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkRenderWindow.h>

using namespace ESPINA;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
CountingFrame::CountingFrame(CountingFrameExtension *extension,
                             Nm                      inclusion[3],
                             Nm                      exclusion[3],
                             SchedulerSPtr           scheduler)
: INCLUSION_FACE   {255}
, EXCLUSION_FACE   {0}
, m_scheduler      {scheduler}
, m_inclusionVolume{0}
, m_totalVolume    {0}
, m_extension      {extension}
, m_command        {vtkSmartPointer<vtkCountingFrameCommand>::New()}
, m_visible        {true}
, m_enable         {true}
, m_highlight      {false}
{
  QWriteLocker lock(&m_marginsMutex);
  memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
  memcpy(m_exclusion, exclusion, 3*sizeof(Nm));

  m_command->setWidget(this);

  m_applyCountingFrame = std::make_shared<ApplyCountingFrame>(this, m_scheduler);

  connect(m_applyCountingFrame.get(), SIGNAL(finished()),
          this,                       SLOT(onCountingFrameApplied()));
}

//-----------------------------------------------------------------------------
CountingFrame::~CountingFrame()
{
  for(auto widget: m_widgets2D)
  {
    deleteSliceWidget(widget);
  }

  for(auto widget: m_widgets3D)
  {
    deleteWidget(widget);
  }
}

//-----------------------------------------------------------------------------
ChannelPtr CountingFrame::channel() const
{
  return m_extension->extendedItem();
}

//-----------------------------------------------------------------------------
void CountingFrame::deleteFromExtension()
{
  m_extension->deleteCountingFrame(this);
}

//-----------------------------------------------------------------------------
void CountingFrame::setMargins(Nm inclusion[3], Nm exclusion[3])
{
  {
    QWriteLocker lock(&m_marginsMutex);
    memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
    memcpy(m_exclusion, exclusion, 3*sizeof(Nm));
  }

  updateCountingFrame();
}

//-----------------------------------------------------------------------------
void CountingFrame::margins(Nm inclusion[3], Nm exclusion[3])
{
  QReadLocker lock(&m_marginsMutex);
  memcpy(inclusion, m_inclusion, 3*sizeof(Nm));
  memcpy(exclusion, m_exclusion, 3*sizeof(Nm));
}

//-----------------------------------------------------------------------------
QString CountingFrame::description() const
{
  auto channel  = m_extension->extendedItem();
  auto spacing  = channel->output()->spacing();
  Nm   voxelVol = spacing[0]*spacing[1]*spacing[2];

  int  totalVoxelVolume     = totalVolume()     / voxelVol;
  int  inclusionVoxelVolume = inclusionVolume() / voxelVol;
  int  exclusionVoxelVolume = exclusionVolume() / voxelVol;

  QString cube = QString::fromUtf8("\u00b3");
  QString br = "\n";
  QString desc;
  desc += tr("CF:   %1"            ).arg(m_id)                             + br;
  desc += tr("Type: %1"            ).arg(typeName())                       + br;
  desc += tr("Volume information:" )                                       + br;
  desc += tr("  Total Volume:"     )                                       + br;
  desc += tr("    %1 voxel"        ).arg(totalVoxelVolume)                 + br;
  desc += tr("    %1 nm"           ).arg(totalVolume(),0,'f',2)     + cube + br;
  desc += tr("  Inclusion Volume:" )                                       + br;
  desc += tr("    %1 voxel"        ).arg(inclusionVoxelVolume)             + br;
  desc += tr("    %1 nm"           ).arg(inclusionVolume(),0,'f',2) + cube + br;
  desc += tr("  Exclusion Volume:" )                                       + br;
  desc += tr("    %1 voxel"        ).arg(exclusionVoxelVolume)             + br;
  desc += tr("    %1 nm"           ).arg(exclusionVolume(),0,'f',2) + cube + br;

  return desc;
}

//-----------------------------------------------------------------------------
void CountingFrame::setVisible(bool visible)
{
  if(m_visible == visible) return;

  QWriteLocker lockState(&m_stateMutex);

  m_visible = visible;

  QMutexLocker lock(&m_widgetMutex);
  for (auto wa : m_widgets2D)
  {
    wa->SetEnabled(m_visible && m_enable);
    wa->setVisible(m_visible);
  }

  for(auto wa: m_widgets3D)
  {
    wa->SetEnabled(m_visible && m_enable);
    wa->setVisible(m_visible);
  }

  emit changedVisibility();
}

//-----------------------------------------------------------------------------
void CountingFrame::setEnabled(bool enable)
{
  if(m_enable == enable) return;

  QWriteLocker lockState(&m_stateMutex);
  m_enable = enable;

  QMutexLocker lock(&m_widgetMutex);
  for (auto wa : m_widgets2D)
  {
    wa->SetEnabled(m_visible && m_enable);
  }

  emit changedVisibility();
}

//-----------------------------------------------------------------------------
void CountingFrame::setHighlighted(bool highlight)
{
  QWriteLocker lockState(&m_stateMutex);
  m_highlight = highlight;

  QMutexLocker lock(&m_widgetMutex);
  for (auto wa : m_widgets2D)
  {
    wa->SetHighlighted(m_highlight);
  }
}

//-----------------------------------------------------------------------------
void CountingFrame::setCategoryConstraint(const QString& category)
{
  m_categoryConstraint = category;

  emit modified(this);
}

//-----------------------------------------------------------------------------
vtkCountingFrameSliceWidget *CountingFrame::createSliceWidget(RenderView *view)
{
  QMutexLocker lock(&m_widgetMutex);

  auto view2D = view2D_cast(view);
  Q_ASSERT(view2D);
  auto slice  = view2D->crosshair()[normalCoordinateIndex(view2D->plane())];

  auto widget = CountingFrame2DWidgetAdapter::New();

  widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
  widget->SetRepresentationDepth(view2D->widgetDepth());
  widget->SetPlane(view2D->plane());
  widget->SetSlicingStep(view2D->sceneResolution());
  widget->SetCountingFrame(channelEdgesPolyData(), m_inclusion, m_exclusion);
  widget->SetSlice(slice);
  widget->SetCurrentRenderer(view2D->mainRenderer());
  widget->SetInteractor(view2D->mainRenderer()->GetRenderWindow()->GetInteractor());
  widget->SetEnabled(true);

  m_widgets2D << widget;

  return widget;
}

//-----------------------------------------------------------------------------
vtkCountingFrame3DWidget *CountingFrame::createWidget(RenderView *view)
{
  QMutexLocker lock(&m_widgetMutex);

  QReadLocker lockMargins(&m_marginsMutex);

  Q_ASSERT(view3D_cast(view));

  auto widget = CountingFrame3DWidgetAdapter::New();

  widget->SetCountingFrame(countingFramePolyData(), m_inclusion, m_exclusion);
  widget->SetCurrentRenderer(view->mainRenderer());
  widget->SetInteractor(view->renderWindow()->GetInteractor());
  widget->SetEnabled(true);

  m_widgets3D << widget;

  return widget;
}

//-----------------------------------------------------------------------------
void CountingFrame::deleteSliceWidget(vtkCountingFrameSliceWidget *widget)
{
  widget->setVisible(false);
  widget->SetEnabled(false);
  widget->SetInteractor(nullptr);
  widget->SetCurrentRenderer(nullptr);
  widget->RemoveObserver(m_command);

  m_widgets2D.removeOne(widget);

  widget->Delete();
}


//-----------------------------------------------------------------------------
void CountingFrame::deleteWidget(vtkCountingFrame3DWidget *widget)
{
  widget->setVisible(false);
  widget->SetEnabled(false);
  widget->SetInteractor(nullptr);
  widget->SetCurrentRenderer(nullptr);

  m_widgets3D.removeOne(widget);

  widget->Delete();
}

//-----------------------------------------------------------------------------
void CountingFrame::updateCountingFrame()
{
  {
    QWriteLocker lock(&m_volumeMutex);
    updateCountingFrameImplementation();
  }

  { // We need to unlock m_widgetMutex before emitting the signal
    QMutexLocker lockWidgets(&m_widgetMutex);
    QReadLocker  lockMargins(&m_marginsMutex);

    for(auto widget : m_widgets2D)
    {
      widget->SetCountingFrame(channelEdgesPolyData(), m_inclusion, m_exclusion);
    }

    for(auto widget : m_widgets3D)
    {
      widget->SetCountingFrame(countingFramePolyData(), m_inclusion, m_exclusion);
    }
  }

  emit modified(this);
}

//-----------------------------------------------------------------------------
void CountingFrame::apply()
{
  Task::submit(m_applyCountingFrame);
}

//-----------------------------------------------------------------------------
void CountingFrame::onCountingFrameApplied()
{
  auto task = qobject_cast<ApplyCountingFrame *>(sender());

  if(!task->isAborted())
  {
    emit applied(this);
  }
}

//-----------------------------------------------------------------------------
Nm CountingFrame::equivalentVolume(const Bounds& bounds)
{
  auto channel = m_extension->extendedItem();
  auto volume  = readLockVolume(channel->output());
  auto origin  = volume->bounds().origin();
  auto spacing = volume->bounds().spacing();

  VolumeBounds volumeBounds(bounds, spacing, origin);

  return (volumeBounds[1]-volumeBounds[0])*(volumeBounds[3]-volumeBounds[2])* (volumeBounds[5]-volumeBounds[4]);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> CountingFrame::channelEdgesPolyData() const
{
  QReadLocker lock(&m_channelEdgesMutex);
  //qDebug() << "Locking for copying edges" << thread();

  auto edges = vtkSmartPointer<vtkPolyData>::New();
  edges->DeepCopy(m_channelEdges);
  //qDebug() << "Edges copied" << thread();

  return edges;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> CountingFrame::countingFramePolyData() const
{
  QReadLocker lock(&m_countingFrameMutex);
  //qDebug() << "Locking for copying CF" << thread();

  auto cf = vtkSmartPointer<vtkPolyData>::New();
  cf->DeepCopy(m_countingFrame);
  //qDebug() << "CF copied" << thread();

  return cf;
}

//-----------------------------------------------------------------------------
void vtkCountingFrameCommand::Execute(vtkObject* caller, long unsigned int eventId, void* callData)
{
  auto widget = static_cast<vtkCountingFrameSliceWidget *>(caller);

  if (widget)
  {
    Nm inOffset[3], exOffset[3];
    widget->GetInclusionOffset(inOffset);
    widget->GetExclusionOffset(exOffset);

    {
      QWriteLocker lock(&m_widget->m_marginsMutex);
      for (int i = 0; i < 3; i++)
      {
        m_widget->m_inclusion[i] = inOffset[i];
        if (m_widget->m_inclusion[i] < 0)
        {
          m_widget->m_inclusion[i] = 0;
        }

        m_widget->m_exclusion[i] = exOffset[i];
        if (m_widget->m_exclusion[i] < 0)
        {
          m_widget->m_exclusion[i] = 0;
        }
      }
    }

    m_widget->updateCountingFrame();
    m_widget->apply();
  }
}

