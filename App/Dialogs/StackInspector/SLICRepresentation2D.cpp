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
#include <vtkImageActor.h>
#include <vtkImageMapToColors.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkImageMapper3D.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Utils;

//--------------------------------------------------------------------
SLICRepresentation2D::SLICRepresentation2D()
: m_textActor     {nullptr}
, m_view      {nullptr}
, m_active    {false}
, m_lastSlice {std::numeric_limits<double>::max()}
, m_planeIndex{-1}
{
}

//--------------------------------------------------------------------
SLICRepresentation2D::~SLICRepresentation2D()
{
  if(m_view) m_view->removeActor(m_textActor);

  m_textActor = nullptr;
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
  repositionActor(m_actor, view2d->widgetDepth(), m_planeIndex);

  m_view->addActor(m_textActor);
  m_view->addActor(m_actor);
  m_view->refresh();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::uninitialize()
{
  if(m_view)
  {
    if(m_actor) m_view->removeActor(m_actor);
    if(m_textActor) m_view->removeActor(m_textActor);
    m_view = nullptr;
    m_textActor = nullptr;
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
    m_textActor->SetVisibility(true);
    m_textActor->Modified();
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
    m_textActor->SetVisibility(false);
    m_textActor->Modified();
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
  m_textActor->SetInput(ss.str().c_str());

  m_textActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  m_textActor->GetPositionCoordinate()->SetViewport(m_view->mainRenderer());
  m_textActor->GetPositionCoordinate()->SetValue(.5, .5);

  m_textActor->Modified();

  /*m_actor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  m_actor->GetPositionCoordinate()->SetViewport(m_view->mainRenderer());
  m_actor->GetPositionCoordinate()->SetValue(.5, .5);*/
  m_actor->GetMapper()->BorderOn();
  m_actor->SetOpacity(0.6);
  m_actor->GetMapper()->SetNumberOfThreads(1);
  m_actor->GetMapper()->UpdateWholeExtent();
  m_actor->Modified();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::buildVTKPipeline()
{
  // TODO: build the actor for the representation.
  // Depending on the SLIC data create a slice data using the area of the channel
  // edges, and then display that data on-screen. If there is no SLIC data, build the pipeline
  // with no dada, do not update actor or enter it in the view.

  m_textActor = vtkSmartPointer<vtkTextActor>::New();
  m_textActor->SetPosition2(10, 40);
  m_textActor->GetTextProperty()->SetBold(true);
  m_textActor->GetTextProperty()->SetFontFamilyToArial();
  m_textActor->GetTextProperty()->SetJustificationToCentered();
  m_textActor->GetTextProperty()->SetVerticalJustificationToCentered();
  m_textActor->GetTextProperty()->SetFontSize(24);
  m_textActor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);

  vtkSmartPointer<vtkUnsignedCharArray> array = vtkSmartPointer<vtkUnsignedCharArray>::New();
  array->SetNumberOfComponents(1);
  array->SetNumberOfTuples(699*536*115);
  for(int z = 0; z < 115; z++) {
    for(int y = 0; y < 536; y++) {
      for(int x = 0; x < 699; x++) {
        array->SetValue(699*536*z + 699*y + x, ((x+y)%2)*255);
      }
    }
  }
  vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
  data->SetOrigin(0,0,0);
  data->SetSpacing(1,1,2);
  data->SetDimensions(699,536,115);
  data->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  data->GetPointData()->SetScalars(array.GetPointer());
  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetTableRange(0,255);
  lut->SetValueRange(0.0, 1.0);
  lut->SetAlphaRange(1.0,1.0);
  lut->SetNumberOfColors(256);
  lut->SetRampToLinear();
  //lut->SetHueRange(color.hueF(), color.hueF());
  //lut->SetSaturationRange(0.0, color.saturationF());
  lut->Build();
  m_mapper = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapper->SetInputData(data);
  m_mapper->SetLookupTable(lut);
  m_mapper->UpdateWholeExtent();
  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapper->GetOutputPort());
  m_actor->GetMapper()->SetNumberOfThreads(1);
  m_actor->SetOpacity(0.6);
  m_actor->GetMapper()->UpdateWholeExtent();
  m_actor->Update();
}
