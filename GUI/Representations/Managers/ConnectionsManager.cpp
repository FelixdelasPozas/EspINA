/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <GUI/Representations/Managers/ConnectionsManager.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkGlyphSource2D.h>
#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <vtkFollower.h>
#include <vtkGlyph3DMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;

//--------------------------------------------------------------------
ConnectionsManager::ConnectionsManager(ViewTypeFlags flags, ModelAdapterSPtr model)
: RepresentationManager{flags, flags.testFlag(ESPINA::VIEW_2D) ? RepresentationManager::NEEDS_ACTORS : RepresentationManager::EXPORTS_3D|RepresentationManager::NEEDS_ACTORS}
, m_model              {model}
, m_scale              {7}
{
  setFlag(HAS_ACTORS, false);
  m_glyph = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_glyph->SetScalarVisibility(false);
  m_glyph->SetStatic(true);

  m_actor = vtkSmartPointer<vtkFollower>::New();
  m_actor->SetMapper(m_glyph);
  m_actor->GetProperty()->SetColor(1,1,1);
  m_actor->GetProperty()->SetLineWidth(2);

  connectSignals();

  getConnectionData();
}

//--------------------------------------------------------------------
ViewItemAdapterList ConnectionsManager::pick(const NmVector3& point, vtkProp* actor) const
{
  return ViewItemAdapterList();
}

//--------------------------------------------------------------------
void ConnectionsManager::onConnectionAdded(Connection connection)
{
  if(!m_connections.contains(connection))
  {
    m_connections << connection;
  }
}

//--------------------------------------------------------------------
void ConnectionsManager::onConnectionRemoved(Connection connection)
{
  if(m_connections.contains(connection))
  {
    m_connections.removeOne(connection);
  }
}

//--------------------------------------------------------------------
bool ConnectionsManager::hasRepresentations() const
{
  return !m_connections.isEmpty();
}

//--------------------------------------------------------------------
void ConnectionsManager::updateFrameRepresentations(const FrameCSPtr frame)
{
  updateActor(frame);
}

//--------------------------------------------------------------------
bool ConnectionsManager::acceptCrosshairChange(const NmVector3& crosshair) const
{
  return lastFrame()->crosshair != crosshair;
}

//--------------------------------------------------------------------
bool ConnectionsManager::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return lastFrame()->resolution != resolution;
}

//--------------------------------------------------------------------
bool ConnectionsManager::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return lastFrame()->bounds != bounds;
}

//--------------------------------------------------------------------
bool ConnectionsManager::acceptInvalidationFrame(const FrameCSPtr frame) const
{
  return true;
}

//--------------------------------------------------------------------
void ConnectionsManager::displayRepresentations(const FrameCSPtr frame)
{
  updateActor(frame);

  if (!hasActors() && m_connections.size() > 0)
  {
    setFlag(HAS_ACTORS, true);
    m_view->addActor(m_actor);
  }
}

//--------------------------------------------------------------------
void ConnectionsManager::hideRepresentations(const FrameCSPtr frame)
{
  setFlag(HAS_ACTORS, false);
  m_view->removeActor(m_actor);
}

//--------------------------------------------------------------------
void ConnectionsManager::onHide(const FrameCSPtr frame)
{
  disconnect(&(m_view->state()), SIGNAL(afterFrameChanged(GUI::Representations::FrameCSPtr)),
             this,               SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));
}

//--------------------------------------------------------------------
void ConnectionsManager::onShow(const FrameCSPtr frame)
{
  connect(&(m_view->state()), SIGNAL(afterFrameChanged(GUI::Representations::FrameCSPtr)),
          this,               SLOT(emitRenderRequest(GUI::Representations::FrameCSPtr)));

  emitRenderRequest(frame);
}

//--------------------------------------------------------------------
RepresentationManagerSPtr ConnectionsManager::cloneImplementation()
{
  auto clone = std::make_shared<ConnectionsManager>(supportedViews(), m_model);
  m_clones << clone;

  return clone;
}

//--------------------------------------------------------------------
void ConnectionsManager::updateActor(const FrameCSPtr frame)
{
  auto view2d = view2D_cast(m_view);
  auto points = vtkSmartPointer<vtkPoints>::New();

  if(view2d)
  {
    auto planeIndex = normalCoordinateIndex(view2d->plane());
    auto spacing    = frame->resolution;
    spacing[planeIndex] = 1;
    auto max = std::max(spacing[0], std::max(spacing[1], spacing[2]));

    auto glyphSource = vtkSmartPointer<vtkGlyphSource2D>::New();
    glyphSource->SetGlyphTypeToCircle();
    glyphSource->SetFilled(false);
    glyphSource->SetCenter(0,0,0);
    glyphSource->SetScale(m_scale*max);
    glyphSource->SetColor(1,1,1);
    glyphSource->Update();

    switch(planeIndex)
    {
      case 0:
      case 1:
        {
          auto transform = vtkSmartPointer<vtkTransform>::New();
          transform->RotateWXYZ(90, (planeIndex == 0 ? 0 : 1), (planeIndex == 1 ? 0 : 1), 0);

          auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
          transformFilter->SetTransform(transform);
          transformFilter->SetInputData(glyphSource->GetOutput());
          transformFilter->Update();

          m_glyph->SetSourceData(transformFilter->GetOutput());
        }
        break;
      default:
      case 2:
        m_glyph->SetSourceData(glyphSource->GetOutput());
        break;
    }

    for(auto connection: m_connections)
    {
      if(connection.point[planeIndex] == frame->crosshair[planeIndex])
      {
        connection.point[planeIndex] += view2d->widgetDepth();
        points->InsertNextPoint(connection.point[0], connection.point[1], connection.point[2]);
      }
    }
  }
  else
  {
    auto spacing    = frame->resolution;
    auto max = std::min(spacing[0], std::min(spacing[1], spacing[2]));

    auto glyphSource = vtkSmartPointer<vtkSphereSource>::New();
    glyphSource->SetCenter(0.0, 0.0, 0.0);
    glyphSource->SetRadius(m_scale*max);
    glyphSource->Update();

    m_glyph->SetSourceData(glyphSource->GetOutput());

    for(auto connection: m_connections)
    {
      points->InsertNextPoint(connection.point[0], connection.point[1], connection.point[2]);
    }
  }

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->Modified();

  m_glyph->SetInputData(polyData);
  m_glyph->Update();
  m_actor->Modified();
}

//--------------------------------------------------------------------
void ConnectionsManager::setRepresentationSize(int size)
{
  size = std::min(std::max(3, size), 15);

  for(auto clone: m_clones)
  {
    clone->setRepresentationSize(size);
  }

  if(isActive() && (size != m_scale))
  {
    m_scale = size;
    auto frame = m_view->state().createFrame();

    invalidateFrames(frame);
    waitForDisplay(frame);
    emitRenderRequest(frame);
  }
}

//--------------------------------------------------------------------
void ConnectionsManager::connectSignals()
{
  connect(m_model.get(), SIGNAL(connectionAdded(Connection)),
          this,          SLOT(onConnectionAdded(Connection)));

  connect(m_model.get(), SIGNAL(connectionRemoved(Connection)),
          this,          SLOT(onConnectionRemoved(Connection)));

  connect(m_model.get(), SIGNAL(aboutToBeReset()),
          this,          SLOT(resetConnections()));
}

//--------------------------------------------------------------------
void ConnectionsManager::getConnectionData()
{
  for(auto seg: m_model->segmentations())
  {
    auto connections = m_model->connections(seg);

    for(auto connection: connections)
    {
      if(!m_connections.contains(connection))
      {
        m_connections << connection;
      }
    }
  }
}

//--------------------------------------------------------------------
void ConnectionsManager::resetConnections()
{
  m_connections.clear();
}