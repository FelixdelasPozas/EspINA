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

// VTK
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::View;

//----------------------------------------------------------------------------
ViewState::ViewState(Timer &timer, RepresentationInvalidator &invalidator)
: m_timer(timer)
, m_invalidator(invalidator)
, m_fitToSlices{true}
, m_coordinateSystem(std::make_shared<CoordinateSystem>())
, m_selection(new Selection(invalidator))
{
  connect(m_coordinateSystem.get(), SIGNAL(resolutionChanged(NmVector3)),
          this,                     SLOT(onResolutionChanged(NmVector3)));
  connect(m_coordinateSystem.get(), SIGNAL(boundsChanged(Bounds)),
          this,                     SLOT(onBoundsChanged(Bounds)));
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
ESPINA::NmVector3 ViewState::crosshair() const
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

  emit viewFocusChanged(center);

  changeCrosshair(center);
}

//----------------------------------------------------------------------------
void ViewState::resetCamera()
{
  emit resetCameraRequested();
}

//----------------------------------------------------------------------------
void ViewState::refresh()
{
  emit refreshRequested();
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
  auto t = m_timer.increment();
  emit widgetsAdded(factory, t);
}

//----------------------------------------------------------------------------
void ViewState::removeTemporalRepresentations(Representations::Managers::TemporalPrototypesSPtr factory)
{
  auto t = m_timer.increment();
  emit widgetsRemoved(factory, t);
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
void ViewState::changeCrosshair(const NmVector3 &point)
{
  if (m_crosshair != point)
  {
    m_crosshair = point;

    m_timer.activate(); // we need to increment on every crosshair change
                        // to improve speed on consecutive slice changes
    auto t = m_timer.increment();

    emit crosshairChanged(point, t);
  }
}

//-----------------------------------------------------------------------------
void ViewState::onResolutionChanged(const NmVector3 &resolution)
{
  auto t = m_timer.increment();

  emit sceneResolutionChanged(resolution, t);
}

//-----------------------------------------------------------------------------
void ViewState::onBoundsChanged(const Bounds &bounds)
{
  auto t = m_timer.increment();

  emit sceneBoundsChanged(bounds, t);
}

//-----------------------------------------------------------------------------
void ESPINA::GUI::View::updateSceneState(ViewState &state, ViewItemAdapterSList viewItems)
{
  Bounds    bounds;
  NmVector3 resolution;

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

  state.coordinateSystem()->setBounds(bounds);
  state.coordinateSystem()->setResolution(resolution);
}
