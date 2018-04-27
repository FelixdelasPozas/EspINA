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
#include <App/ToolGroups/Segment/Skeleton/ConnectionPointsTemporalRepresentation2D.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/Utils.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkGlyph3DMapper.h>
#include <vtkFollower.h>
#include <vtkProperty.h>
#include <vtkGlyphSource2D.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkRenderer.h>

// C++
#include <limits>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Utils;
using namespace ESPINA::GUI::Representations::Settings;

//--------------------------------------------------------------------
ConnectionPointsTemporalRepresentation2D::ConnectionPointsTemporalRepresentation2D(ConnectionSettingsSPtr settings)
: m_points     {nullptr}
, m_polyData   {nullptr}
, m_glyphMapper{nullptr}
, m_glyph2D    {nullptr}
, m_actor      {nullptr}
, m_scale      {4}
, m_view       {nullptr}
, m_planeIndex {-1}
, m_lastSlice  {-std::numeric_limits<Nm>::max()}
, m_active     {false}
, m_settings   {settings}
{
  connect(m_settings.get(), SIGNAL(modified()), this, SLOT(onRepresentationModified()));
}

//--------------------------------------------------------------------
ConnectionPointsTemporalRepresentation2D::~ConnectionPointsTemporalRepresentation2D()
{
  uninitialize();
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::onRepresentationModified()
{
  auto size = m_settings->connectionSize();

  if(m_scale != size)
  {
    m_scale = size;

    if(m_view)
    {
      auto spacing = m_view->sceneResolution();
      auto minSpacing = std::numeric_limits<double>::max();
      for(auto i: {0,1,2})
      {
        if(i == m_planeIndex) continue;
        minSpacing = std::min(minSpacing, spacing[i]);
      }

      m_glyph2D->SetScale(m_scale*minSpacing);
      m_glyph2D->Update();

      updateActor(m_lastSlice);

      m_view->refresh();
    }
  }
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::initialize(RenderView* view)
{
  if(m_view || !isView2D(view)) return;

  auto view2d  = view2D_cast(view);
  m_view       = view;
  m_planeIndex = normalCoordinateIndex(view2d->plane());
  m_lastSlice  = m_view->crosshair()[m_planeIndex]-1; // to force initialization.
  m_scale      = m_settings->connectionSize();

  buildPipeline();

  repositionActor(m_actor, view2d->widgetDepth(), m_planeIndex);

  m_view->addActor(m_actor);
  m_view->refresh();
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::uninitialize()
{
  m_actor       = nullptr;
  m_glyph2D     = nullptr;
  m_glyphMapper = nullptr;
  m_polyData    = nullptr;
  m_points      = nullptr;
  m_view        = nullptr;
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::show()
{
  if(!m_view) return;

  if(!m_active)
  {
    m_active = true;
    m_actor->SetVisibility(true);
    m_actor->Modified();
  }
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::hide()
{
  if(!m_view) return;

  if(m_active)
  {
    m_active = false;
    m_actor->SetVisibility(false);
    m_actor->Modified();
  }
}

//--------------------------------------------------------------------
bool ConnectionPointsTemporalRepresentation2D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  if(!m_view) return false;

  return m_lastSlice != crosshair[m_planeIndex];
}

//--------------------------------------------------------------------
bool ConnectionPointsTemporalRepresentation2D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return true;
}

//--------------------------------------------------------------------
bool ConnectionPointsTemporalRepresentation2D::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return true;
}

//--------------------------------------------------------------------
bool ConnectionPointsTemporalRepresentation2D::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::display(const GUI::Representations::FrameCSPtr& frame)
{
  if(!m_points) buildPipeline();

  auto slice = frame->crosshair[m_planeIndex];
  if(m_lastSlice != slice)
  {
    updateActor(slice);
    m_lastSlice = slice;
  }
}

//--------------------------------------------------------------------
GUI::Representations::Managers::TemporalRepresentation2DSPtr ConnectionPointsTemporalRepresentation2D::cloneImplementation()
{
  return std::make_shared<ConnectionPointsTemporalRepresentation2D>(m_settings);
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::onConnectionPointAdded(const NmVector3 point)
{
  if(!m_connections.contains(point))
  {
    auto spacing = m_view->sceneResolution();

    NmVector3 adjustedPoint;
    for(auto i: {0,1,2})
    {
      auto half = spacing[i]/2.;
      adjustedPoint[i] = std::round((point[i]+half)/spacing[i])*spacing[i] - half;
    }
    m_connections << adjustedPoint;

    updateActor(m_lastSlice);

    if(m_view) m_view->mainRenderer()->Render();
  }
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::onConnectionPointRemoved(const NmVector3 point)
{
  if(m_connections.contains(point))
  {
    m_connections.removeAll(point);

    updateActor(m_lastSlice);

    if(m_view) m_view->mainRenderer()->Render();
  }
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::clearPoints()
{
  m_connections.clear();

  updateActor(m_lastSlice);

  if(m_view) m_view->mainRenderer()->Render();
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::buildPipeline()
{
  if(m_points || !m_view) return;

  m_points = vtkSmartPointer<vtkPoints>::New();

  m_polyData = vtkSmartPointer<vtkPolyData>::New();
  m_polyData->SetPoints(m_points);

  m_glyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_glyphMapper->SetScalarVisibility(false);
  m_glyphMapper->SetStatic(false);
  m_glyphMapper->ScalingOff();
  m_glyphMapper->SetInputData(m_polyData);

  auto spacing = m_view->sceneResolution();
  auto minSpacing = std::numeric_limits<double>::max();
  for(auto i: {0,1,2})
  {
    if(i == m_planeIndex) continue;
    minSpacing = std::min(minSpacing, spacing[i]);
  }

  m_glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_glyph2D->SetGlyphTypeToCircle();
  m_glyph2D->SetFilled(false);
  m_glyph2D->SetCenter(0, 0, 0);
  m_glyph2D->SetScale(m_scale*minSpacing);
  m_glyph2D->SetColor(1, 1, 1);
  m_glyph2D->Update();

  switch (m_planeIndex)
  {
    case 0:
    case 1:
    {
      auto transform = vtkSmartPointer<vtkTransform>::New();
      transform->RotateWXYZ(90, (m_planeIndex == 0 ? 0 : 1), (m_planeIndex == 1 ? 0 : 1), 0);

      auto cTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      cTransformFilter->SetTransform(transform);
      cTransformFilter->SetInputData(m_glyph2D->GetOutput());
      cTransformFilter->Update();

      auto eTransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      eTransformFilter->SetTransform(transform);
      eTransformFilter->Update();

      m_glyphMapper->SetSourceData(cTransformFilter->GetOutput());
    }
      break;
    default:
    case 2:
      m_glyphMapper->SetSourceData(m_glyph2D->GetOutput());
      break;
  }

  m_actor = vtkSmartPointer<vtkFollower>::New();
  m_actor->SetMapper(m_glyphMapper);

  auto view2d = view2D_cast(m_view);
  auto depth  = view2d->widgetDepth();

  repositionActor(m_actor, depth, m_planeIndex);
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::updateActor(const Nm slice)
{
  if(m_view)
  {
    m_points->Reset();

    if (!m_connections.isEmpty())
    {
      for (auto point : m_connections)
      {
        if (point[m_planeIndex] == slice)
        {
          m_points->InsertNextPoint(point[0], point[1], point[2]);
        }
      }
    }

    m_points->Modified();
    m_polyData->Modified();
    m_glyphMapper->Update();
    m_actor->Modified();
  }
}
