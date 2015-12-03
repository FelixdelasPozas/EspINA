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

#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/Representations/Frame.h>

#include <GUI/View/RenderView.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;

//------------------------------------------------------------------------
TemporalPrototypes::TemporalPrototypes(TemporalRepresentation2DSPtr prototype2D,
                                       TemporalRepresentation3DSPtr prototype3D,
                                       const QString               &name)
: m_prototype2D{prototype2D}
, m_prototype3D{prototype3D}
, m_name       {name}
{
}

//------------------------------------------------------------------------
ViewTypeFlags TemporalPrototypes::supportedViews() const
{
  ViewTypeFlags result;

  if (m_prototype2D) result |= ViewType::VIEW_2D;
  if (m_prototype3D) result |= ViewType::VIEW_3D;

  return result;
}

//------------------------------------------------------------------------
TemporalRepresentation2DSPtr TemporalPrototypes::createRepresentation2D() const
{
  return m_prototype2D->clone();
}

//------------------------------------------------------------------------
TemporalRepresentation3DSPtr TemporalPrototypes::createRepresentation3D() const
{
  return m_prototype3D->clone();
}

//------------------------------------------------------------------------
QString TemporalPrototypes::name() const
{
  return m_name;
}

//------------------------------------------------------------------------
TemporalManager::TemporalManager(TemporalPrototypesSPtr prototypes, ManagerFlags flags)
: RepresentationManager{prototypes->supportedViews(), flags}
, m_prototypes         {prototypes}
, m_plane              {Plane::UNDEFINED}
, m_depth              {0}
{
  setName(QString("TemporalManager::%1").arg(prototypes->name()));
}

//------------------------------------------------------------------------
TemporalManager::~TemporalManager()
{
}

//------------------------------------------------------------------------
ViewItemAdapterList TemporalManager::pick(const NmVector3 &point, vtkProp *actor) const
{
  return ViewItemAdapterList();
}

//------------------------------------------------------------------------
void TemporalManager::setPlane(Plane plane)
{
  m_plane = plane;
}

//------------------------------------------------------------------------
void TemporalManager::setRepresentationDepth(Nm depth)
{
  m_depth = depth;
}

//------------------------------------------------------------------------
bool TemporalManager::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return (m_representation && m_representation->acceptCrosshairChange(crosshair));
}

//------------------------------------------------------------------------
bool TemporalManager::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return (m_representation && m_representation->acceptSceneResolutionChange(resolution));
}

//------------------------------------------------------------------------
bool TemporalManager::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return (m_representation && m_representation->acceptSceneBoundsChange(bounds));
}

//------------------------------------------------------------------------
bool TemporalManager::acceptInvalidationFrame(GUI::Representations::FrameCSPtr frame) const
{
  return (m_representation && m_representation->acceptInvalidationFrame(frame));
}

//------------------------------------------------------------------------
bool TemporalManager::hasRepresentations() const
{
  return true;
}

//------------------------------------------------------------------------
void TemporalManager::updateFrameRepresentations(const FrameCSPtr frame)
{
  // intentionally empty, representations must be computed on display().
}

//------------------------------------------------------------------------
void TemporalManager::onShow(const FrameCSPtr frame)
{
  // if already initialized return
  if(m_representation) return;

  connect(&(m_view->state()), SIGNAL(afterFrameChanged(GUI::Representations::FrameCSPtr)),
          this,               SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

  if (m_plane != Plane::UNDEFINED)
  {
    auto representation2D = m_prototypes->createRepresentation2D();

    representation2D->setPlane(m_plane);
    representation2D->setRepresentationDepth(m_depth);

    m_representation = representation2D;
  }
  else
  {
    m_representation = m_prototypes->createRepresentation3D();
  }

  m_representation->initialize(m_view);

  emitRenderRequest(frame);
}

//------------------------------------------------------------------------
void TemporalManager::onHide(const FrameCSPtr frame)
{
  // intentionally empty.
}

//------------------------------------------------------------------------
void TemporalManager::displayRepresentations(const FrameCSPtr frame)
{
  m_representation->display(frame);
}

//------------------------------------------------------------------------
void TemporalManager::hideRepresentations(const FrameCSPtr frame)
{
  m_representation->hide();
  m_representation->uninitialize();

  disconnect(&(m_view->state()), SIGNAL(afterFrameChanged(GUI::Representations::FrameCSPtr)),
             this,               SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));
}

//------------------------------------------------------------------------
RepresentationManagerSPtr TemporalManager::cloneImplementation()
{
  return std::make_shared<TemporalManager>(m_prototypes, flags());
}
