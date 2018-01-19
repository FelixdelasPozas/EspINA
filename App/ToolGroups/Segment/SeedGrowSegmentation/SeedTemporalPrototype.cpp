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
#include "SeedTemporalPrototype.h"
#include <Core/Utils/EspinaException.h>
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/Utils.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkGlyphSource2D.h>
#include <vtkGlyph2D.h>
#include <vtkFollower.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGlyph3DMapper.h>
#include <vtkTransform.h>
#include <vtkProperty.h>

// C++
#include <limits>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::View::Utils;

//--------------------------------------------------------------------
SeedTemporalRepresentation::SeedTemporalRepresentation(SeedGrowSegmentationFilter *filter)
: m_points               {nullptr}
, m_polyData             {nullptr}
, m_glyph2D              {nullptr}
, m_glyphMapper          {nullptr}
, m_actor                {nullptr}
, m_view                 {nullptr}
, m_filter               {filter}
, m_planeIndex           {-1}
, m_lastSlice            {-std::numeric_limits<Nm>::min()}
, m_active               {false}
{
  m_seed = filter->seed();
}

//--------------------------------------------------------------------
SeedTemporalRepresentation::~SeedTemporalRepresentation()
{
  if(m_view) m_view->removeActor(m_actor);

  m_points      = nullptr;
  m_polyData    = nullptr;
  m_glyph2D     = nullptr;
  m_actor       = nullptr;
  m_glyphMapper = nullptr;
}

//--------------------------------------------------------------------
bool SeedTemporalRepresentation::acceptCrosshairChange(const NmVector3& crosshair) const
{
  if(!m_view) return false;

  return m_lastSlice != crosshair[m_planeIndex];
}

//--------------------------------------------------------------------
bool SeedTemporalRepresentation::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return true;
}

//--------------------------------------------------------------------
bool SeedTemporalRepresentation::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return true;
}

//--------------------------------------------------------------------
bool SeedTemporalRepresentation::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//--------------------------------------------------------------------
void SeedTemporalRepresentation::updateActor(const GUI::Representations::FrameCSPtr frame)
{
  if(!m_view) return;

  if(!m_points)
  {
    buildVTKPipeline();
  }

  auto view2d  = view2D_cast(m_view);

  m_points->Reset();
  auto point = m_seed;
  point[m_planeIndex] = view2d->slicingPosition();

  auto isVisible = (point == m_seed);

  if(isVisible)
  {
    m_lastSlice = view2d->slicingPosition();
    m_points->SetNumberOfPoints(1);
    m_points->SetPoint(0, point[0], point[1], point[2]);

    switch (m_planeIndex)
    {
      case 0:
      case 1:
        {
          auto transform = vtkSmartPointer<vtkTransform>::New();
          transform->RotateWXYZ(90, (m_planeIndex == 0 ? 0 : 1), (m_planeIndex == 1 ? 0 : 1), 0);

          auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
          transformFilter->SetTransform(transform);
          transformFilter->SetInputData(m_glyph2D->GetOutput());
          transformFilter->DebugOn();
          transformFilter->GlobalWarningDisplayOn();
          transformFilter->Update();

          m_glyphMapper->SetSourceData(transformFilter->GetOutput());
        }
        break;
      default:
      case 2:
        m_glyphMapper->SetSourceData(m_glyph2D->GetOutput());
        break;
    }
  }

  m_points->Modified();
  m_polyData->Modified();
  m_glyphMapper->Update();
  m_actor->SetVisibility(isVisible);
  m_actor->Modified();
}

//--------------------------------------------------------------------
void SeedTemporalRepresentation::initialize(RenderView* view)
{
  if(m_view || !isView2D(view)) return;

  auto view2d  = view2D_cast(view);
  m_view       = view;
  m_planeIndex = normalCoordinateIndex(view2d->plane());
  m_lastSlice  = m_view->crosshair()[m_planeIndex];

  buildVTKPipeline();

  repositionActor(m_actor, view2d->widgetDepth(), m_planeIndex);

  m_view->addActor(m_actor);
  m_view->refresh();
}

//--------------------------------------------------------------------
void SeedTemporalRepresentation::uninitialize()
{
  m_view->removeActor(m_actor);
  m_view = nullptr;
}

//--------------------------------------------------------------------
void SeedTemporalRepresentation::show()
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
void SeedTemporalRepresentation::hide()
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
void SeedTemporalRepresentation::display(const GUI::Representations::FrameCSPtr& frame)
{
  if(m_view) updateActor(frame);
}

//--------------------------------------------------------------------
GUI::Representations::Managers::TemporalRepresentation2DSPtr SeedTemporalRepresentation::cloneImplementation()
{
  return std::make_shared<SeedTemporalRepresentation>(m_filter);
}

//--------------------------------------------------------------------
void SeedTemporalRepresentation::buildVTKPipeline()
{
  if(!m_view) return;

  auto spacing = m_view->sceneResolution();
  if(!isValidSpacing(spacing)) return;

  m_points = vtkSmartPointer<vtkPoints>::New();

  auto point = m_view->crosshair();
  if(m_seed == point)
  {
    m_points->SetNumberOfPoints(1);
    m_points->SetPoint(0, m_seed[0], m_seed[1], m_seed[2]);
  }

  m_polyData = vtkSmartPointer<vtkPolyData>::New();
  m_polyData->SetPoints(m_points);

  m_glyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_glyphMapper->SetScalarVisibility(false);
  m_glyphMapper->SetInputData(m_polyData);

  m_glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  m_glyph2D->SetGlyphTypeToCross();
  m_glyph2D->SetFilled(false);
  m_glyph2D->SetCenter(0,0,0);
  m_glyph2D->SetScale(5);
  m_glyph2D->SetColor(1,1,1);
  m_glyph2D->Update();

  switch (m_planeIndex)
  {
    case 0:
    case 1:
      {
        auto transform = vtkSmartPointer<vtkTransform>::New();
        transform->RotateWXYZ(90, (m_planeIndex == 0 ? 0 : 1), (m_planeIndex == 1 ? 0 : 1), 0);

        auto transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        transformFilter->SetTransform(transform);
        transformFilter->SetInputData(m_glyph2D->GetOutput());
        transformFilter->Update();

        m_glyphMapper->SetSourceData(transformFilter->GetOutput());
      }
      break;
    default:
    case 2:
      m_glyphMapper->SetSourceData(m_glyph2D->GetOutput());
      break;
  }

  m_actor = vtkSmartPointer<vtkFollower>::New();
  m_actor->SetMapper(m_glyphMapper);
  m_actor->GetProperty()->SetColor(1,1,1);
  m_actor->GetProperty()->SetLineWidth(3);
}
