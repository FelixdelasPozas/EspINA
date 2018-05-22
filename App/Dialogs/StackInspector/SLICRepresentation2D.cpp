/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/StackInspector/SLICRepresentation2D.h>
#include <GUI/View/Utils.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vtkCoordinate.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Utils;

//--------------------------------------------------------------------
SLICRepresentation2D::SLICRepresentation2D()
: m_actor     {nullptr}
, m_view      {nullptr}
, m_active    {false}
, m_lastSlice {std::numeric_limits<double>::max()}
, m_planeIndex{-1}
{
}

//--------------------------------------------------------------------
SLICRepresentation2D::~SLICRepresentation2D()
{
  if(m_view) m_view->removeActor(m_actor);

  m_actor = nullptr;
}

//--------------------------------------------------------------------
void SLICRepresentation2D::setSLICData()
{
  // TODO: depends on how the SLIC data is stored and interpreted.
  // The manager shouldn't be able to shwo anything if there is no SLIC data,
  // maybe show a text saying that SLIC is not computed yet.
}

//--------------------------------------------------------------------
void SLICRepresentation2D::initialize(RenderView* view)
{
  if(m_view || !isView2D(view)) return;

  auto view2d  = view2D_cast(view);
  m_view       = view;
  m_planeIndex = normalCoordinateIndex(view2d->plane());

  buildVTKPipeline();

  // TODO: this will be needed when the real actor is computed.
  // repositionActor(m_actor, view2d->widgetDepth(), 2);

  m_view->addActor(m_actor);
  m_view->refresh();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::uninitialize()
{
  if(m_view)
  {
    if(m_actor) m_view->removeActor(m_actor);
    m_view = nullptr;
    m_actor = nullptr;
  }
}

//--------------------------------------------------------------------
void SLICRepresentation2D::show()
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
void SLICRepresentation2D::hide()
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
bool SLICRepresentation2D::acceptCrosshairChange(const NmVector3& crosshair) const
{
  if(!m_view) return false;

  return m_lastSlice != crosshair[m_planeIndex];
}

//--------------------------------------------------------------------
bool SLICRepresentation2D::acceptSceneResolutionChange(const NmVector3& resolution) const
{
  return true;
}

//--------------------------------------------------------------------
bool SLICRepresentation2D::acceptSceneBoundsChange(const Bounds& bounds) const
{
  return true;
}

//--------------------------------------------------------------------
bool SLICRepresentation2D::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//--------------------------------------------------------------------
void SLICRepresentation2D::display(const GUI::Representations::FrameCSPtr& frame)
{
  if(m_view) updateActor(frame);
}

//--------------------------------------------------------------------
GUI::Representations::Managers::TemporalRepresentation2DSPtr SLICRepresentation2D::cloneImplementation()
{
  return std::make_shared<SLICRepresentation2D>();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::updateActor(const GUI::Representations::FrameCSPtr frame)
{
  // TODO: change the actor to show the slic for the given frame.
  // Compute SLIC for given frame and update the actor built in buildVTKPipeline...
  std::stringstream ss;
  ss << "SLIC Representation\nFrame: " << frame->crosshair[m_planeIndex] << " nanometers";
  m_actor->SetInput(ss.str().c_str());

  m_actor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  m_actor->GetPositionCoordinate()->SetViewport(m_view->mainRenderer());
  m_actor->GetPositionCoordinate()->SetValue(.5, .5);

  m_actor->Modified();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::buildVTKPipeline()
{
  // TODO: build the actor for the representation.
  // Depending on the SLIC data create a slice data using the area of the channel
  // edges, and then display that data on-screen. If there is no SLIC data, build the pipeline
  // with no dada, do not update actor or enter it in the view.

  m_actor = vtkSmartPointer<vtkTextActor>::New();
  m_actor->SetPosition2(10, 40);
  m_actor->GetTextProperty()->SetBold(true);
  m_actor->GetTextProperty()->SetFontFamilyToArial();
  m_actor->GetTextProperty()->SetJustificationToCentered();
  m_actor->GetTextProperty()->SetVerticalJustificationToCentered();
  m_actor->GetTextProperty()->SetFontSize(24);
  m_actor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);
}
