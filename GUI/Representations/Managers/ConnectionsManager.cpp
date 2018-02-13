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
, m_points             {nullptr}
, m_polyData           {nullptr}
, m_transformFilter    {nullptr}
, m_glyph              {nullptr}
, m_glyph2D            {nullptr}
, m_glyph3D            {nullptr}
, m_actor              {nullptr}
, m_scale              {4}
{
  setFlag(HAS_ACTORS, false);
  setName("ConnectionsManager");
  setDescription("Displays a representation of connection points.");

  connectSignals();

  getConnectionData();
}

//--------------------------------------------------------------------
ConnectionsManager::~ConnectionsManager()
{
  m_actor    = nullptr;
  m_glyph2D  = nullptr;
  m_glyph3D  = nullptr;
  m_glyph    = nullptr;
  m_polyData = nullptr;
  m_points   = nullptr;
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

  if (!hasActors() && hasRepresentations() && m_actor)
  {
    setFlag(HAS_ACTORS, true);
    m_view->addActor(m_actor);
  }
}

//--------------------------------------------------------------------
void ConnectionsManager::hideRepresentations(const FrameCSPtr frame)
{
  if(hasActors() && m_actor)
  {
    setFlag(HAS_ACTORS, false);
    m_view->removeActor(m_actor);
  }
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
  return std::make_shared<ConnectionsManager>(supportedViews(), m_model);
}

//--------------------------------------------------------------------
void ConnectionsManager::updateActor(const FrameCSPtr frame)
{
  if(!m_view) return;

  if(!m_points)
  {
    buildVTKPipeline(frame);
  }

  auto view2d  = view2D_cast(m_view);
  auto spacing = frame->resolution;

  m_points->Reset();

  if(view2d)
  {
    auto planeIndex = normalCoordinateIndex(view2d->plane());
    auto minSpacing = std::numeric_limits<double>::max();
    for(auto i: {0,1,2})
    {
      if(i == planeIndex) continue;
      minSpacing = std::min(minSpacing, spacing[i]);
    }

    m_glyph2D->SetScale(m_scale*minSpacing);
    m_glyph2D->Update();

    if(m_transformFilter)
    {
      m_transformFilter->Update();
    }

    for(auto connection: m_connections)
    {
      if(connection.point[planeIndex] == frame->crosshair[planeIndex])
      {
        connection.point[planeIndex] += view2d->widgetDepth();
        m_points->InsertNextPoint(connection.point[0], connection.point[1], connection.point[2]);
      }
    }
  }
  else
  {
    auto minSpacing = std::min(spacing[0], std::min(spacing[1], spacing[2]));

    m_glyph3D->SetRadius(m_scale*minSpacing);
    m_glyph3D->Update();

    for(auto connection: m_connections)
    {
      m_points->InsertNextPoint(connection.point[0], connection.point[1], connection.point[2]);
    }
  }

  m_points->Modified();
  m_polyData->Modified();
  m_glyph->Update();
  m_actor->Modified();
}

//--------------------------------------------------------------------
void ConnectionsManager::setRepresentationSize(int size)
{
  size = std::min(std::max(1, size), 30);

  for(auto child: m_childs)
  {
    auto clone = dynamic_cast<ConnectionsManager *>(child);
    if(clone) clone->setRepresentationSize(size);
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

  if(m_points)
  {
    m_points->Reset();
  }
}

//--------------------------------------------------------------------
void ConnectionsManager::shutdown()
{
  disconnect(m_model.get(), SIGNAL(connectionAdded(Connection)),
             this,          SLOT(onConnectionAdded(Connection)));

  disconnect(m_model.get(), SIGNAL(connectionRemoved(Connection)),
             this,          SLOT(onConnectionRemoved(Connection)));

  disconnect(m_model.get(), SIGNAL(aboutToBeReset()),
             this,          SLOT(resetConnections()));

  resetConnections();

  RepresentationManager::shutdown();
}

//--------------------------------------------------------------------
void ConnectionsManager::buildVTKPipeline(const FrameCSPtr frame)
{
  if(!m_view) return;

  auto spacing = frame->resolution;
  if(!isValidSpacing(spacing)) return;
  auto view2d  = view2D_cast(m_view);

  m_points = vtkSmartPointer<vtkPoints>::New();

  m_polyData = vtkSmartPointer<vtkPolyData>::New();
  m_polyData->SetPoints(m_points);

  m_glyph = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_glyph->SetScalarVisibility(false);
  m_glyph->SetStatic(true);
  m_glyph->SetInputData(m_polyData);

  if(view2d)
  {
    auto planeIndex = normalCoordinateIndex(view2d->plane());
    auto minSpacing = std::numeric_limits<double>::max();
    for(auto i: {0,1,2})
    {
      if(i == planeIndex) continue;
      minSpacing = std::min(minSpacing, spacing[i]);
    }

    m_glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
    m_glyph2D->SetGlyphTypeToCircle();
    m_glyph2D->SetFilled(false);
    m_glyph2D->SetCenter(0,0,0);
    m_glyph2D->SetScale(m_scale*minSpacing);
    m_glyph2D->SetColor(1,1,1);
    m_glyph2D->Update();

    switch(planeIndex)
    {
      case 0:
      case 1:
        {
          auto transform = vtkSmartPointer<vtkTransform>::New();
          transform->RotateWXYZ(90, (planeIndex == 0 ? 0 : 1), (planeIndex == 1 ? 0 : 1), 0);

          m_transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
          m_transformFilter->SetTransform(transform);
          m_transformFilter->SetInputData(m_glyph2D->GetOutput());
          m_transformFilter->Update();

          m_glyph->SetSourceData(m_transformFilter->GetOutput());
        }
        break;
      default:
      case 2:
        m_glyph->SetSourceData(m_glyph2D->GetOutput());
        break;
    }
  }
  else
  {
    auto min = std::min(spacing[0], std::min(spacing[1], spacing[2]));

    m_glyph3D = vtkSmartPointer<vtkSphereSource>::New();
    m_glyph3D->SetCenter(0.0, 0.0, 0.0);
    m_glyph3D->SetRadius(m_scale*min);
    m_glyph3D->Update();

    m_glyph->SetSourceData(m_glyph3D->GetOutput());
  }

  m_actor = vtkSmartPointer<vtkFollower>::New();
  m_actor->SetMapper(m_glyph);
}
