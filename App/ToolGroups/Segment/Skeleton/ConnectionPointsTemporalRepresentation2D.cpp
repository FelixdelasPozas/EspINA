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

//--------------------------------------------------------------------
ConnectionPointsTemporalRepresentation2D::ConnectionPointsTemporalRepresentation2D()
: m_cPoints     {nullptr}
, m_ePoints     {nullptr}
, m_cPolyData   {nullptr}
, m_ePolyData   {nullptr}
, m_cGlyphMapper{nullptr}
, m_eGlyphMapper{nullptr}
, m_glyph2D     {nullptr}
, m_entryGlyph2D{nullptr}
, m_cActor      {nullptr}
, m_eActor      {nullptr}
, m_scale       {9}
, m_view        {nullptr}
, m_planeIndex  {-1}
, m_lastSlice   {-std::numeric_limits<Nm>::max()}
, m_active      {false}
{
}

//--------------------------------------------------------------------
ConnectionPointsTemporalRepresentation2D::~ConnectionPointsTemporalRepresentation2D()
{
  if(m_view)
  {
    m_view->removeActor(m_cActor);
    m_view->removeActor(m_eActor);
  }

  m_cActor       = nullptr;
  m_eActor       = nullptr;
  m_glyph2D      = nullptr;
  m_cGlyphMapper = nullptr;
  m_eGlyphMapper = nullptr;
  m_cPolyData    = nullptr;
  m_ePolyData    = nullptr;
  m_cPoints      = nullptr;
  m_ePoints      = nullptr;
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::setRepresentationSize(const int size)
{
  if(m_scale != size)
  {
    m_scale = size;

    if(m_view) m_view->refresh();
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

  buildPipeline();

  repositionActor(m_cActor, view2d->widgetDepth(), m_planeIndex);
  repositionActor(m_eActor, view2d->widgetDepth(), m_planeIndex);

  m_view->addActor(m_cActor);
  m_view->addActor(m_eActor);
  m_view->refresh();
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::uninitialize()
{
  m_view->removeActor(m_cActor);
  m_view->removeActor(m_eActor);
  m_view = nullptr;
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::show()
{
  if(!m_view) return;

  if(!m_active)
  {
    m_active = true;
    m_cActor->SetVisibility(true);
    m_cActor->Modified();
    m_eActor->SetVisibility(true);
    m_eActor->Modified();
  }
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::hide()
{
  if(!m_view) return;

  if(m_active)
  {
    m_active = false;
    m_cActor->SetVisibility(false);
    m_cActor->Modified();
    m_eActor->SetVisibility(false);
    m_eActor->Modified();
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
  if(!m_cPoints) buildPipeline();

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
  return std::make_shared<ConnectionPointsTemporalRepresentation2D>();
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::onConnectionPointAdded(const NmVector3 point)
{
  if(!m_connections.contains(point))
  {
    m_connections << point;

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
void ConnectionPointsTemporalRepresentation2D::onEntryPointAdded(const NmVector3 point)
{
  if(!m_entryPoints.contains(point))
  {
    m_entryPoints << point;

    updateActor(m_lastSlice);

    if(m_view) m_view->mainRenderer()->Render();
  }
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::clearPoints()
{
  m_connections.clear();
  m_entryPoints.clear();

  updateActor(m_lastSlice);

  if(m_view) m_view->mainRenderer()->Render();
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::buildPipeline()
{
  if(m_cPoints || !m_view) return;

  m_cPoints = vtkSmartPointer<vtkPoints>::New();

  m_cPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_cPolyData->SetPoints(m_cPoints);

  m_cGlyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_cGlyphMapper->SetScalarVisibility(false);
  m_cGlyphMapper->SetStatic(false);
  m_cGlyphMapper->SetInputData(m_cPolyData);

  m_ePoints = vtkSmartPointer<vtkPoints>::New();

  m_ePolyData = vtkSmartPointer<vtkPolyData>::New();
  m_ePolyData->SetPoints(m_ePoints);

  m_eGlyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_eGlyphMapper->SetScalarVisibility(false);
  m_eGlyphMapper->SetStatic(false);
  m_eGlyphMapper->SetInputData(m_ePolyData);

  auto spacing = m_view->sceneResolution();
  spacing[m_planeIndex] = spacing[(m_planeIndex + 1) % 3];
  auto multiplier = 2 * std::min(spacing[0], std::min(spacing[1], spacing[2]));

  m_glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_glyph2D->SetGlyphTypeToCircle();
  m_glyph2D->SetFilled(false);
  m_glyph2D->SetCenter(0, 0, 0);
  m_glyph2D->SetScale(m_scale*multiplier);
  m_glyph2D->SetColor(1, 1, 1);
  m_glyph2D->Update();

  m_entryGlyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_entryGlyph2D->SetGlyphTypeToCircle();
  m_entryGlyph2D->SetFilled(true);
  m_entryGlyph2D->SetCenter(0, 0, 0);
  m_entryGlyph2D->SetScale(m_scale*multiplier);
  m_entryGlyph2D->SetColor(0, 1, 0);
  m_entryGlyph2D->Update();

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
      eTransformFilter->SetInputData(m_entryGlyph2D->GetOutput());
      eTransformFilter->Update();

      m_cGlyphMapper->SetSourceData(cTransformFilter->GetOutput());
      m_eGlyphMapper->SetSourceData(eTransformFilter->GetOutput());
    }
      break;
    default:
    case 2:
      m_cGlyphMapper->SetSourceData(m_glyph2D->GetOutput());
      m_eGlyphMapper->SetSourceData(m_entryGlyph2D->GetOutput());
      break;
  }

  m_cActor = vtkSmartPointer<vtkFollower>::New();
  m_cActor->SetMapper(m_cGlyphMapper);
  m_cActor->GetProperty()->SetColor(1,1,1);
  m_cActor->GetProperty()->SetLineWidth(2);
  m_cActor->GetProperty()->Modified();

  m_eActor = vtkSmartPointer<vtkFollower>::New();
  m_eActor->SetMapper(m_eGlyphMapper);
  m_eActor->GetProperty()->SetColor(0,1,0);
  m_eActor->GetProperty()->SetLineWidth(2);
  m_eActor->GetProperty()->Modified();

  auto view2d = view2D_cast(m_view);
  auto depth  = view2d->widgetDepth();

  repositionActor(m_cActor, depth, m_planeIndex);
  repositionActor(m_eActor, depth, m_planeIndex);
}

//--------------------------------------------------------------------
void ConnectionPointsTemporalRepresentation2D::updateActor(const Nm slice)
{
  m_cPoints->Reset();
  m_ePoints->Reset();

  if (!m_connections.isEmpty())
  {
    for (auto point : m_connections)
    {
      if (point[m_planeIndex] == slice)
      {
        m_cPoints->InsertNextPoint(point[0], point[1], point[2]);
      }
    }
  }

  m_cPoints->Modified();
  m_cPolyData->Modified();
  m_cGlyphMapper->Update();
  m_cActor->Modified();

  if (!m_entryPoints.empty())
  {
    for (auto point : m_entryPoints)
    {
      if (point[m_planeIndex] == slice)
      {
        m_ePoints->InsertNextPoint(point[0], point[1], point[2]);
      }
    }
  }

  m_ePoints->Modified();
  m_ePolyData->Modified();
  m_eGlyphMapper->Update();
  m_eActor->Modified();
}
