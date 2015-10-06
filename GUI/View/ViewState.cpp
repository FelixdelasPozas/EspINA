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

// ESPINA
#include "ViewState.h"
#include <GUI/Representations/Frame.h>

// VTK
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::View;

//----------------------------------------------------------------------------
ViewState::ViewState(Timer &timer, RepresentationInvalidator &invalidator)
: m_timer(timer)
, m_invalidator(invalidator)
, m_fitToSlices{true}
, m_coordinateSystem(std::make_shared<CoordinateSystem>())
, m_selection(new Selection(invalidator))
{
}

//----------------------------------------------------------------------------
Timer &ViewState::timer() const
{
  return m_timer;
}

//----------------------------------------------------------------------------
RepresentationInvalidator &ViewState::representationInvalidator() const
{
  return m_invalidator;
}

//----------------------------------------------------------------------------
NmVector3 ViewState::crosshair() const
{
  return m_crosshair;
}

//----------------------------------------------------------------------------
SelectionSPtr ViewState::selection() const
{
  return m_selection;
}

//----------------------------------------------------------------------------
void ViewState::setFitToSlices(bool value)
{
  m_fitToSlices = value;
  //TODO REVIEW
}

//----------------------------------------------------------------------------
bool ViewState::fitToSlices() const
{
  return m_fitToSlices;
}

//----------------------------------------------------------------------------
void ViewState::setEventHandler(EventHandlerSPtr handler)
{
  if (m_eventHandler != handler)
  {
    if (m_eventHandler)
    {
      m_eventHandler->setInUse(false);
    }

    m_eventHandler = handler;

    if (m_eventHandler)
    {
      m_eventHandler->setInUse(true);
    }

    emit eventHandlerChanged();
  }
}

//----------------------------------------------------------------------------
void ViewState::unsetEventHandler(EventHandlerSPtr handler)
{
  if (m_eventHandler == handler)
  {
    setEventHandler(nullptr);
  }
}

//----------------------------------------------------------------------------
EventHandlerSPtr ViewState::eventHandler() const
{
  return m_eventHandler;
}

//----------------------------------------------------------------------------
void ViewState::focusViewOn(const NmVector3 &point)
{
  auto center = crosshairPoint(point);

//   emit viewFocusChanged(center);

  changeCrosshair(center, true);
}

//----------------------------------------------------------------------------
void ViewState::resetCamera()
{
  auto frame = createFrame();

  frame->reset = true;

  emit resetCamera(frame);
}

//----------------------------------------------------------------------------
void ViewState::refresh()
{
//   emit refreshRequested();
}

//----------------------------------------------------------------------------
void ViewState::setCoordinateSystem(const NmVector3 &resolution, const Bounds &bounds)
{
  m_coordinateSystem->setBounds(bounds);
  m_coordinateSystem->setResolution(resolution);

  emit frameChanged(createFrame());
}

//----------------------------------------------------------------------------
void ViewState::setCrosshair(const NmVector3 &point)
{
  //qDebug() << "Requested crosshair" << point;
  changeCrosshair(crosshairPoint(point));
}

//----------------------------------------------------------------------------
void ViewState::setCrosshairPlane(const Plane plane, const Nm position)
{
  NmVector3 crosshair = m_crosshair;

  crosshair[normalCoordinateIndex(plane)] = position;

  setCrosshair(crosshair);
}

//----------------------------------------------------------------------------
NmVector3 ViewState::crosshairPoint(const NmVector3 &point) const
{
  if(m_fitToSlices)
  {
    return voxelCenter(point);
  }
  else
  {
    Nm x = vtkMath::Floor(point[0]);
    Nm y = vtkMath::Floor(point[1]);
    Nm z = vtkMath::Floor(point[2]);

    return NmVector3{x,y,z};
  }
}

//----------------------------------------------------------------------------
NmVector3 ViewState::voxelCenter(const NmVector3 &point) const
{
  return m_coordinateSystem->voxelCenter(point);
}

//----------------------------------------------------------------------------
void ViewState::addTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory)
{
  emit widgetsAdded(factory, createFrame());
}

//----------------------------------------------------------------------------
void ViewState::removeTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory)
{
  emit widgetsRemoved(factory, createFrame());
}

//----------------------------------------------------------------------------
void ViewState::addSliceSelectors(SliceSelectorSPtr selector, SliceSelectionType type)
{
  emit sliceSelectorAdded(selector, type);
}

//----------------------------------------------------------------------------
void ViewState::removeSliceSelectors(SliceSelectorSPtr selector)
{
  emit sliceSelectorRemoved(selector);
}

//----------------------------------------------------------------------------
CoordinateSystemSPtr ViewState::coordinateSystem() const
{
  return m_coordinateSystem;
}

//----------------------------------------------------------------------------
void ViewState::changeCrosshair(const NmVector3 &point, bool focus)
{
  if (m_crosshair != point || focus)
  {
    m_crosshair = point;

    m_timer.activate(); // we need to increment on every crosshair change
                        // to improve speed on consecutive slice changes
    auto frame = createFrame();

    frame->focus = focus;

    emit frameChanged(frame);
  }
}

//-----------------------------------------------------------------------------
FrameSPtr ViewState::createFrame()
{
  return createFrame(m_crosshair);
}

//-----------------------------------------------------------------------------
FrameSPtr ViewState::createFrame(const NmVector3 &point)
{
  auto frame = std::make_shared<Frame>();

  frame->time       = m_timer.increment();
  frame->crosshair  = point;
  frame->resolution = m_coordinateSystem->resolution();
  frame->bounds     = m_coordinateSystem->bounds();

  return frame;
}

//-----------------------------------------------------------------------------
void ESPINA::GUI::View::updateSceneState(ViewState &state, ViewItemAdapterSList viewItems)
{
  Bounds    bounds{0, 1, 0, 1, 0, 1};
  NmVector3 resolution{1,1,1};

  if (!viewItems.isEmpty())
  {
    auto output = viewItems.first()->output();

    bounds     = output->bounds();
    resolution = output->spacing();

    for (int i = 1; i < viewItems.size(); ++i)
    {
      output = viewItems[i]->output();

      auto itemBounds  = output->bounds();
      auto itemSpacing = output->spacing();

      for (int i = 0; i < 3; i++)
      {
        resolution[i]   = std::min(resolution[i],   itemSpacing[i]);
        bounds[2*i]     = std::min(bounds[2*i]    , itemBounds[2*i]);
        bounds[(2*i)+1] = std::max(bounds[(2*i)+1], itemBounds[(2*i)+1]);
      }
    }

  }

  state.setCoordinateSystem(resolution, bounds);
}
