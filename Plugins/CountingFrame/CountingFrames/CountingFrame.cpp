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
#include "Extensions/CountingFrameExtension.h"
#include "vtkCountingFrameSliceWidget.h"

// ESPINA
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Utils/VolumeBounds.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkRenderWindow.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::CF;

//-----------------------------------------------------------------------------
CountingFrame::CountingFrame(CountingFrameExtension *extension,
                             Nm                      inclusion[3],
                             Nm                      exclusion[3],
                             SchedulerSPtr           scheduler,
                             CoreFactory            *factory)
: INCLUSION_FACE   {255}
, EXCLUSION_FACE   {0}
, m_scheduler      {scheduler}
, m_factory        {factory}
, m_countingFrame  {nullptr}
, m_innerFrame     {nullptr}
, m_inclusionVolume{0}
, m_totalVolume    {0}
, m_extension      {extension}
, m_id             {"Global"}
, m_command        {vtkSmartPointer<vtkCountingFrameCommand>::New()}
, m_visible        {true}
, m_enable         {true}
, m_highlight      {false}
, m_applyTask      {nullptr}
{
  QWriteLocker lock(&m_marginsMutex);
  memcpy(m_inclusion, inclusion, 3*sizeof(Nm));
  memcpy(m_exclusion, exclusion, 3*sizeof(Nm));

  m_command->setCountingFrame(this);

  auto itemExtensions = extension->extendedItem()->readOnlyExtensions();
  Q_ASSERT(itemExtensions->hasExtension(ChannelEdges::TYPE));

  // TODO: the channel edges extension is accessed every time, even if the Counting Frame
  //       is orthogonal, triggering computation that may be unneeded.
  auto edgesExtension = retrieveExtension<ChannelEdges>(itemExtensions);
  m_channelEdges = edgesExtension->channelEdges();
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

  unsigned long long totalVoxelVolume     = totalVolume()     / voxelVol;
  unsigned long long inclusionVoxelVolume = inclusionVolume() / voxelVol;
  unsigned long long exclusionVoxelVolume = exclusionVolume() / voxelVol;
  long int  frontSl                       = int(front()/spacing[2]);
  long int  backSl                        = int(back()/spacing[2]);
  auto constraint                         = categoryConstraint().isEmpty() ? "None (Global)" : categoryConstraint();

  QString cube = QString::fromUtf8("\u00b3");
  QString br = "\n";
  QString desc;
  desc += tr("CF id : %1"               ).arg(id())                             + br;
  desc += tr("Type: %1"                 ).arg(typeName())                       + br;
  desc += tr("Constraint: %1"           ).arg(constraint)                       + br;
  desc += tr("Stack: %1"                ).arg(channel->name())                  + br;
  desc += tr("Volume information:"      )                                       + br;
  desc += tr("  Total Volume:"          )                                       + br;
  desc += tr("    %1 voxel"             ).arg(totalVoxelVolume)                 + br;
  desc += tr("    %1 nm"                ).arg(totalVolume(),0,'f',2)     + cube + br;
  desc += tr("  Inclusion Volume:"      )                                       + br;
  desc += tr("    %1 voxel"             ).arg(inclusionVoxelVolume)             + br;
  desc += tr("    %1 nm"                ).arg(inclusionVolume(),0,'f',2) + cube + br;
  desc += tr("  Exclusion Volume:"      )                                       + br;
  desc += tr("    %1 voxel"             ).arg(exclusionVoxelVolume)             + br;
  desc += tr("    %1 nm"                ).arg(exclusionVolume(),0,'f',2) + cube + br;
  desc += tr("Margins:"                 )                                       + br;
  desc += tr("  Left %1 nm"             ).arg(left())                           + br;
  desc += tr("  Top %1 nm"              ).arg(top())                            + br;
  desc += tr("  Right %1 nm"            ).arg(right())                          + br;
  desc += tr("  Bottom %1 nm"           ).arg(bottom())                         + br;
  desc += tr("  Front %1 nm (%2 slices)").arg(front()).arg(frontSl)             + br;
  desc += tr("  Back %1 nm (%2 slices)" ).arg(back()).arg(backSl)               + br;

  return desc;
}

//-----------------------------------------------------------------------------
void CountingFrame::setVisible(bool visible)
{
  if(m_visible != visible)
  {
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
}

//-----------------------------------------------------------------------------
void CountingFrame::setEnabled(bool enable)
{
  if(m_enable != enable)
  {
    QWriteLocker lockState(&m_stateMutex);
    m_enable = enable;

    QMutexLocker lock(&m_widgetMutex);
    for (auto wa : m_widgets2D)
    {
      wa->SetEnabled(m_visible && m_enable);
    }

    emit changedVisibility();
  }
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
  if(m_categoryConstraint != category)
  {
    m_categoryConstraint = category;

    emit modified(this);
  }
}

//-----------------------------------------------------------------------------
vtkCountingFrameSliceWidget *CountingFrame::createSliceWidget(RenderView *view)
{
  QMutexLocker lock(&m_widgetMutex);

  auto view2D = view2D_cast(view);
  Q_ASSERT(view2D);
  auto slice  = view2D->crosshair()[normalCoordinateIndex(view2D->plane())];
  auto spacing = m_extension->extendedItem()->output()->spacing();

  auto widget = CountingFrame2DWidgetAdapter::New();

  widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
  widget->SetRepresentationDepth(view2D->widgetDepth());
  widget->SetPlane(view2D->plane());
  widget->SetCountingFrame(channelEdgesPolyData(), m_inclusion, m_exclusion, spacing);
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
  auto spacing = m_extension->extendedItem()->output()->spacing();

  widget->SetCountingFrame(countingFramePolyData(), m_inclusion, m_exclusion, spacing);
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

    auto spacing = m_extension->extendedItem()->output()->spacing();

    for(auto widget : m_widgets2D)
    {
      widget->SetCountingFrame(channelEdgesPolyData(), m_inclusion, m_exclusion, spacing);
    }

    for(auto widget : m_widgets3D)
    {
      widget->SetCountingFrame(countingFramePolyData(), m_inclusion, m_exclusion, spacing);
    }
  }

  emit modified(this);

  apply();
}

//-----------------------------------------------------------------------------
void CountingFrame::apply()
{
  QMutexLocker lock(&m_taskMutex);

  if(m_applyTask)
  {
    disconnect(m_applyTask.get(), SIGNAL(finished()),
               this,              SLOT(onCountingFrameApplied()));

    m_applyTask->abort();
    m_applyTask = nullptr;
  }

  m_applyTask = std::make_shared<ApplyCountingFrame>(this, m_factory, m_scheduler);

  connect(m_applyTask.get(), SIGNAL(finished()),
          this,              SLOT(onCountingFrameApplied()), Qt::DirectConnection);

  Task::submit(m_applyTask);
}

//-----------------------------------------------------------------------------
void CountingFrame::onCountingFrameApplied()
{
  QMutexLocker lock(&m_taskMutex);

  if(!m_applyTask->isAborted())
  {
    emit applied(this);
  }

  disconnect(m_applyTask.get(), SIGNAL(finished()),
             this,              SLOT(onCountingFrameApplied()));

  m_applyTask = nullptr;
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

  auto edges = vtkSmartPointer<vtkPolyData>::New();
  edges->DeepCopy(m_channelEdges);

  return edges;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> CountingFrame::countingFramePolyData() const
{
  QReadLocker lock(&m_countingFrameMutex);

  auto polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->DeepCopy(m_countingFrame);

  return polydata;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> CountingFrame::innerFramePolyData() const
{
  QReadLocker lock(&m_countingFrameMutex);

  auto polydata = vtkSmartPointer<vtkPolyData>::New();
  polydata->DeepCopy(m_innerFrame);

  return polydata;
}

//-----------------------------------------------------------------------------
void CountingFrame::setId(Id id)
{
  if(m_id != id)
  {
    m_id = id;

    emit modified(this);
  }
}

//-----------------------------------------------------------------------------
bool CF::lessThan(const CountingFrame *lhs, const CountingFrame *rhs)
{
  auto lhsParts = lhs->id().split(' ');
  auto rhsParts = rhs->id().split(' ');

  // identify id's with parts, assuming the last part could be a number (i.e. layer N)
  if((lhsParts == rhsParts) && (lhsParts.size() > 1))
  {
    int pos = 0;

    // at least one part is different, as they are unique ids.
    while((lhsParts[pos] == rhsParts[pos]) && (pos < lhsParts.size())) ++pos;

    Q_ASSERT(pos < lhsParts.size());
    bool isOk = false;
    auto lhsNum = lhsParts[pos].replace("[^\\d]","").toDouble(&isOk);

    if(!isOk) return (lhsParts[pos] < rhsParts[pos]);

    auto rhsNum = rhsParts[pos].replace("[^\\d]", "").toDouble(&isOk);

    if(!isOk) return (lhsParts[pos] < rhsParts[pos]);

    return (lhsNum < rhsNum);
  }

  return (lhs->id() < rhs->id());
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
      QWriteLocker lock(&m_cf->m_marginsMutex);
      for (int i = 0; i < 3; i++)
      {
        m_cf->m_inclusion[i] = inOffset[i];
        if (m_cf->m_inclusion[i] < 0)
        {
          m_cf->m_inclusion[i] = 0;
        }

        m_cf->m_exclusion[i] = exOffset[i];
        if (m_cf->m_exclusion[i] < 0)
        {
          m_cf->m_exclusion[i] = 0;
        }
      }
    }

    m_cf->updateCountingFrame();
  }
}
