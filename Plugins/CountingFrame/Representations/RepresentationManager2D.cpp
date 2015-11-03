/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Plugin
#include "RepresentationManager2D.h"

// ESPINA
#include <GUI/Representations/Frame.h>
#include <GUI/View/RenderView.h>

using namespace ESPINA;
using namespace ESPINA::CF;
using ESPINA::GUI::Representations::RepresentationManagerSPtr;

//-----------------------------------------------------------------------------
RepresentationManager2D::RepresentationManager2D(CountingFrameManager &manager, ViewTypeFlags supportedViews)
: RepresentationManager{supportedViews, ManagerFlags()}
, m_plane  {ESPINA::Plane::UNDEFINED}
, m_depth  {0}
, m_manager(manager)
{
  connect(&m_manager, SIGNAL(countingFrameCreated(CountingFrame*)),
          this,       SLOT(onCountingFrameCreated(CountingFrame*)));
  connect(&m_manager, SIGNAL(countingFrameDeleted(CountingFrame*)),
          this,       SLOT(onCountingFrameDeleted(CountingFrame*)));
}

//-----------------------------------------------------------------------------
RepresentationManager2D::~RepresentationManager2D()
{
  for(auto cf : m_widgets.keys())
  {
    deleteWidget(cf);
  }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList RepresentationManager2D::pick(const NmVector3 &point, vtkProp *actor) const
{
  return ViewItemAdapterList();
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::setPlane(Plane plane)
{
  m_plane = plane;
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::setRepresentationDepth(Nm depth)
{
  m_depth = depth;
}

//-----------------------------------------------------------------------------
bool RepresentationManager2D::needsRepresentationUpdate(const GUI::Representations::FrameCSPtr frame)
{
  return !m_widgets.isEmpty() && GUI::Representations::RepresentationManager::needsRepresentationUpdate(frame);
}

//-----------------------------------------------------------------------------
bool RepresentationManager2D::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return isNormalDifferent(crosshair, lastFrame()->crosshair);
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::onCountingFrameCreated(CountingFrame *cf)
{
  if (isActive())
  {
    auto widget = createWidget(cf);

    m_widgets.insert(cf, widget);

    emitRenderRequest(m_view->state().createFrame());
  }
  else
  {
    m_pendingCFs << cf;
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::onCountingFrameDeleted(CountingFrame *cf)
{
  if (m_pendingCFs.contains(cf))
  {
    m_pendingCFs.removeOne(cf);
  }
  else
  {
    Q_ASSERT(m_widgets.keys().contains(cf));

    deleteWidget(cf);

    emitRenderRequest(m_view->state().createFrame());
  }
}

//-----------------------------------------------------------------------------
bool RepresentationManager2D::hasRepresentations() const
{
  return !m_widgets.isEmpty();
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::updateFrameRepresentations(const GUI::Representations::FrameCSPtr frame)
{
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::onShow(const GUI::Representations::FrameCSPtr frame)
{
  for (auto cf : m_pendingCFs)
  {
    m_widgets[cf] = createWidget(cf);
  }

  m_pendingCFs.clear();

  emitRenderRequest(frame);
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::onHide(const GUI::Representations::FrameCSPtr frame)
{
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::displayRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  for (auto widget : m_widgets)
  {
    showWidget(widget, frame->time);
  }
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::hideRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  for (auto widget : m_widgets)
  {
    hideWidget(widget);
  }
}

//-----------------------------------------------------------------------------
RepresentationManagerSPtr RepresentationManager2D::cloneImplementation()
{
  return std::make_shared<RepresentationManager2D>(m_manager, supportedViews());
}

//-----------------------------------------------------------------------------
Nm RepresentationManager2D::slicingPosition(TimeStamp t) const
{
  auto crosshair = frame(t)->crosshair;

  Q_ASSERT(m_plane != Plane::UNDEFINED);
  return crosshair[normalCoordinateIndex(m_plane)];
}

//-----------------------------------------------------------------------------
vtkCountingFrameSliceWidget *RepresentationManager2D::createWidget(CountingFrame *cf)
{
  auto widget = cf->createSliceWidget(m_view);
  Q_ASSERT(widget);

  return widget;
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::showWidget(vtkCountingFrameSliceWidget *widget, TimeStamp t)
{
  widget->SetEnabled(true);
  widget->SetSlice(slicingPosition(t));
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::hideWidget(vtkCountingFrameSliceWidget *widget)
{
  widget->SetEnabled(false);
}

//-----------------------------------------------------------------------------
void RepresentationManager2D::deleteWidget(CountingFrame *cf)
{
  auto widget = m_widgets[cf];

  hideWidget(widget);

  cf->deleteSliceWidget(widget);

  m_widgets.remove(cf);
}

//-----------------------------------------------------------------------------
bool RepresentationManager2D::isNormalDifferent(const NmVector3 &p1, const NmVector3 &p2) const
{
  auto normal = normalCoordinateIndex(m_plane);

  return p1[normal] != p2[normal];
}

