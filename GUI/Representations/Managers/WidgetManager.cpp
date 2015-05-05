/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <GUI/Representations/Managers/WidgetManager.h>

#include "GUI/View/Widgets/WidgetFactory.h"
#include <GUI/View/RenderView.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;

//------------------------------------------------------------------------
WidgetManager::WidgetManager(WidgetFactorySPtr factory, ManagerFlags flags)
: RepresentationManager(factory->supportedViews(), flags)
, m_factory(factory)
{
}

//------------------------------------------------------------------------
WidgetManager::~WidgetManager()
{
}

//------------------------------------------------------------------------
TimeRange WidgetManager::readyRangeImplementation() const
{
  return m_pendingActions.timeRange();
}

//------------------------------------------------------------------------
ViewItemAdapterPtr WidgetManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  return nullptr;
}

//------------------------------------------------------------------------
void WidgetManager::setPlane(Plane plane)
{
  m_plane = plane;
}

//------------------------------------------------------------------------
void WidgetManager::setRepresentationDepth(Nm depth)
{
  m_depth = 2*depth;
}

//------------------------------------------------------------------------
bool WidgetManager::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  Q_ASSERT(m_widget);
  return m_widget->acceptCrosshairChange(crosshair);
}

//------------------------------------------------------------------------
bool WidgetManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  Q_ASSERT(m_widget);
  return m_widget->acceptSceneResolutionChange(resolution);
}

//------------------------------------------------------------------------
bool WidgetManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return false;
}

//------------------------------------------------------------------------
bool WidgetManager::hasRepresentations() const
{
  return true;
}

//------------------------------------------------------------------------
void WidgetManager::updateRepresentations(const NmVector3 &crosshair, const NmVector3 &resolution, const Bounds &bounds, TimeStamp t)
{
  if (m_plane != Plane::UNDEFINED)
  {
    auto widget2D = m_factory->createWidget2D();

    widget2D->setPlane(m_plane);
    widget2D->setRepresentationDepth(m_depth);

    m_widget = widget2D;
  }
  else
  {
    m_widget = m_factory->createWidget3D();
  }

  m_widget->initialize(m_view);
  m_widget->setCrosshair(crosshair);
  m_widget->setSceneResolution(resolution);

  m_pendingActions.addValue(Action(INIT, NmVector3()), t);

  emitRenderRequest(t);
}

//------------------------------------------------------------------------
void WidgetManager::changeCrosshair(const NmVector3 &crosshair, TimeStamp t)
{
  m_pendingActions.addValue(Action(CROSSHAIR, crosshair), t);
}

//------------------------------------------------------------------------
void WidgetManager::changeSceneResolution(const NmVector3 &resolution, TimeStamp t)
{
  m_pendingActions.addValue(Action(RESOLUTION, resolution), t);
}

//------------------------------------------------------------------------
void WidgetManager::onShow(TimeStamp t)
{
}

//------------------------------------------------------------------------
void WidgetManager::onHide(TimeStamp t)
{
}

//------------------------------------------------------------------------
void WidgetManager::displayRepresentations(TimeStamp t)
{
  if (!m_widget->isEnabled()) m_widget->show();

  auto action = m_pendingActions.value(t, Action(INIT, NmVector3()));

  if (action.first == CROSSHAIR)
  {
    m_widget->setCrosshair(action.second);
  }
  else if (action.first == RESOLUTION)
  {
    m_widget->setSceneResolution(action.second);
  }

  m_pendingActions.invalidatePreviousValues(t);
}

//------------------------------------------------------------------------
void WidgetManager::hideRepresentations(TimeStamp t)
{
  m_widget->hide();
  m_widget->uninitialize();
}

//------------------------------------------------------------------------
RepresentationManagerSPtr WidgetManager::cloneImplementation()
{
  return std::make_shared<WidgetManager>(m_factory, flags());
}
