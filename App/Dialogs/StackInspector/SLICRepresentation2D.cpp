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
#include <Extensions/SLIC/StackSLIC.h>

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
#include <vtkPoints.h>
#include <vtkFollower.h>
#include <vtkGlyph3DMapper.h>
#include <vtkGlyphSource2D.h>
#include <vtkPolyData.h>

// Qt
#include <QColor>

// C++
#include <random>
#include <chrono>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Utils;

//--------------------------------------------------------------------
SLICRepresentation2D::SLICRepresentation2D(std::shared_ptr<Extensions::StackSLIC> extension)
: m_textActor {nullptr}
, m_view      {nullptr}
, m_active    {false}
, m_lastSlice {std::numeric_limits<double>::max()}
, m_planeIndex{-1}
, m_extension {extension}
, opacity     {0.3}
, useColors   {true}
{
}

//--------------------------------------------------------------------
SLICRepresentation2D::SLICRepresentation2D(std::shared_ptr<Extensions::StackSLIC> extension, float opacity, bool useColors)
: m_textActor {nullptr}
, m_view      {nullptr}
, m_active    {false}
, m_lastSlice {std::numeric_limits<double>::max()}
, m_planeIndex{-1}
, m_extension {extension}
, opacity     {opacity}
, useColors   {useColors}
{
}

//--------------------------------------------------------------------
SLICRepresentation2D::~SLICRepresentation2D()
{
  if(m_view)
  {
    uninitialize();
  }
}

//--------------------------------------------------------------------
void SLICRepresentation2D::setSLICExtension(std::shared_ptr<Extensions::StackSLIC> extension)
{
  m_extension = extension;
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
  repositionActor(m_pointsActor, 2*view2d->widgetDepth(), m_planeIndex);

  show();
  m_view->refresh();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::uninitialize()
{
  if(m_view)
  {
    if(m_active)
    {
      if(m_actor) m_view->removeActor(m_actor);
      if(m_textActor) m_view->removeActor(m_textActor);
      if(m_pointsActor) m_view->removeActor(m_pointsActor);
    }
    m_view = nullptr;
    m_textActor = nullptr;
    m_actor = nullptr;
    m_mapper = nullptr;
    m_data = nullptr;
    m_points = nullptr;
    m_pointsData = nullptr;
    m_pointsMapper = nullptr;
    m_pointsActor = nullptr;
  }
}

//--------------------------------------------------------------------
void SLICRepresentation2D::show()
{
  if(!m_view) return;

  if(!m_active)
  {
    m_active = true;
    m_view->addActor(m_textActor);
    m_view->addActor(m_actor);
    m_view->addActor(m_pointsActor);
//    m_textActor->SetVisibility(true);
//    m_textActor->Modified();
//    m_actor->SetVisibility(true);
//    m_actor->Modified();
  }
}

//--------------------------------------------------------------------
void SLICRepresentation2D::hide()
{
  if(!m_view) return;

  if(m_active)
  {
    m_active = false;
    m_view->removeActor(m_textActor);
    m_view->removeActor(m_actor);
    m_view->removeActor(m_pointsActor);
//    m_textActor->SetVisibility(false);
//    m_textActor->Modified();
//    m_actor->SetVisibility(false);
//    m_actor->Modified();
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
  if(m_view)
  {
    updateActor(frame);
  }
}

//--------------------------------------------------------------------
GUI::Representations::Managers::TemporalRepresentation2DSPtr SLICRepresentation2D::cloneImplementation()
{
  return std::make_shared<SLICRepresentation2D>(m_extension, opacity, useColors);
}

//--------------------------------------------------------------------
void SLICRepresentation2D::updateActor(const GUI::Representations::FrameCSPtr frame)
{
  auto slice = frame->crosshair[m_planeIndex]/m_extension->getSliceSpacing();
  bool computed = (m_extension != nullptr)                         &&
                  m_extension->drawSliceInImageData(slice, m_data) &&
                  m_extension->drawVoxelCenters(slice, m_points);

  if(!computed)
  {
    m_textActor->SetVisibility(true);
    m_textActor->Modified();

    m_actor->SetVisibility(false);
    m_actor->Modified();

    m_pointsActor->SetVisibility(false);
    m_pointsActor->Modified();
    return;
  }

  m_view->removeActor(m_actor);

  m_pointsData->Modified();
  m_pointsMapper->Update();

  m_pointsActor->SetVisibility(true);
  m_pointsActor->Modified();

  m_textActor->SetVisibility(m_extension->isRunning());
  m_textActor->Modified();

  m_actor->SetVisibility(true);
  m_actor->GetMapper()->UpdateWholeExtent();
  m_actor->Update();

  m_actor->SetOpacity(opacity);
  m_actor->Modified();

  m_view->addActor(m_actor);
}

//--------------------------------------------------------------------
void SLICRepresentation2D::buildVTKPipeline()
{
  // TODO: build the actor for the representation.
  // Depending on the SLIC data create a slice data using the area of the channel
  // edges, and then display that data on-screen. If there is no SLIC data, build the pipeline
  // with no data, do not update actor or enter it in the view.

  // text actor
  m_textActor = vtkSmartPointer<vtkTextActor>::New();
  m_textActor->SetPosition2(10, 40);
  m_textActor->GetTextProperty()->SetBold(true);
  m_textActor->GetTextProperty()->SetFontFamilyToArial();
  m_textActor->GetTextProperty()->SetJustificationToCentered();
  m_textActor->GetTextProperty()->SetVerticalJustificationToCentered();
  m_textActor->GetTextProperty()->SetFontSize(24);
  m_textActor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);

  std::stringstream ss;
  ss << "SLIC not computed!\nRun it first.";
  m_textActor->SetInput(ss.str().c_str());

  m_textActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  m_textActor->GetPositionCoordinate()->SetViewport(m_view->mainRenderer());
  m_textActor->GetPositionCoordinate()->SetValue(.5, .5);

  // slice actor
  m_data = vtkSmartPointer<vtkImageData>::New();
  m_data->SetOrigin(0,0,0);
  m_data->SetExtent(0,1,0,1,0,1);
  m_data->SetSpacing(1,1,1);

  auto info = m_data->GetInformation();
  vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
  vtkImageData::SetNumberOfScalarComponents(1, info);
  m_data->SetInformation(info);
  m_data->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  m_data->Modified();

//  auto lut = hueRangeLUT();
//  auto lut = hueNonConsecutiveLUT();
//  auto lut = grayscaleLUT();
  auto lut = useColors?randomLUT():grayscaleLUT();

  m_mapper = vtkSmartPointer<vtkImageMapToColors>::New();
  m_mapper->SetInputData(m_data);
  m_mapper->SetLookupTable(lut);
  m_mapper->UpdateWholeExtent();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->SetInterpolate(false);
  m_actor->GetMapper()->BorderOn();
  m_actor->GetMapper()->SetInputConnection(m_mapper->GetOutputPort());
  m_actor->GetMapper()->SetNumberOfThreads(1);
  m_actor->GetMapper()->UpdateWholeExtent();
  m_actor->Update();

  // points actor
  auto spacing = m_view->sceneResolution();
  auto minSpacing = std::numeric_limits<double>::max();
  for(auto i: {0,1})
  {
    minSpacing = std::min(minSpacing, spacing[i]);
  }

  m_points = vtkSmartPointer<vtkPoints>::New();

  m_pointsData = vtkSmartPointer<vtkPolyData>::New();
  m_pointsData->SetPoints(m_points);

  m_pointsMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
  m_pointsMapper->SetScalarVisibility(false);
  m_pointsMapper->ScalingOff();
  m_pointsMapper->SetInputData(m_pointsData);

  auto glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
  glyph2D->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);
  glyph2D->SetGlyphTypeToCross();
  glyph2D->SetFilled(true);
  glyph2D->SetCenter(0,0,0);
  glyph2D->SetScale(minSpacing*2); // two pixels in the shortest direction of X,Y.
  glyph2D->SetColor(1,1,1);
  glyph2D->Update();

  m_pointsMapper->SetSourceData(glyph2D->GetOutput());

  m_pointsActor = vtkSmartPointer<vtkFollower>::New();
  m_pointsActor->SetMapper(m_pointsMapper);
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> SLICRepresentation2D::grayscaleLUT() const
{
  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetTableRange(0,255);
  lut->SetNumberOfTableValues(256);
  lut->SetValueRange(0.0, 1.0);
  lut->SetAlphaRange(1,1);
  lut->SetRampToLinear();
  lut->SetHueRange(0, 0);
  lut->SetSaturationRange(0, 0);
  lut->Build();

  return lut;
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> SLICRepresentation2D::hueRangeLUT() const
{
  const double increment = 360./256.;

  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetTableRange(0,255);
  lut->SetNumberOfTableValues(256);

  for(int i = 0; i < 256; ++i)
  {
    auto color = QColor::fromHsv(static_cast<int>(i * increment), 255,255,255);
    lut->SetTableValue(i, color.redF(), color.greenF(), color.blue(), 1.0);
  }

  lut->Modified();

  return lut;
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> SLICRepresentation2D::hueNonConsecutiveLUT() const
{
  const double increment = 360./256.;
  int range1 = 0;
  int range2 = range1+64;
  int range3 = range2+64;
  int range4 = range3+64;

  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetTableRange(0,255);
  lut->SetNumberOfTableValues(256);

  for(int i = 0; i < 256; i+=4)
  {
    auto color = QColor::fromHsv(static_cast<int>(increment * range1++), 255,255,255);
    lut->SetTableValue(i, color.redF(), color.greenF(), color.blueF(), 1);
    color = QColor::fromHsv(static_cast<int>(increment * range2++), 255,255,255);
    lut->SetTableValue(i+1, color.redF(), color.greenF(), color.blueF(), 1);
    color = QColor::fromHsv(static_cast<int>(increment * range3++), 255,255,255);
    lut->SetTableValue(i+2, color.redF(), color.greenF(), color.blueF(), 1);
    color = QColor::fromHsv(static_cast<int>(increment * range4++), 255,255,255);
    lut->SetTableValue(i+3, color.redF(), color.greenF(), color.blueF(), 1);
  }

  lut->Modified();

  return lut;
}

//--------------------------------------------------------------------
void SLICRepresentation2D::setSLICComputationProgress(int value)
{
  if(m_view)
  {
    std::stringstream ss;
    ss << "Computing...\nProgress: " << value << " %";
    m_textActor->SetInput(ss.str().c_str());
    m_textActor->Modified();

    m_view->refresh();
  }
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkLookupTable> SLICRepresentation2D::randomLUT() const
{
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 generator(seed);

  auto lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetTableRange(0,255);
  lut->SetNumberOfTableValues(256);

  for(int i = 0; i < 256; ++i)
  {
    auto color = QColor::fromHsv(static_cast<int>(generator() % 360), 255,255,255);
    lut->SetTableValue(i, color.redF(), color.greenF(), color.blue(), 1.0);
  }

  lut->Modified();

  return lut;
}

//--------------------------------------------------------------------
void SLICRepresentation2D::opacityChanged(int value)
{
  opacity = value/100.0;
  m_actor->SetOpacity(opacity);
  if(m_view) m_view->refresh();
}

//--------------------------------------------------------------------
void SLICRepresentation2D::colorModeCheckChanged(int value)
{
  if(value == Qt::Unchecked)
  {
    useColors = false;
    m_mapper->SetLookupTable(grayscaleLUT());
  }
  else
  {
    useColors = true;
    m_mapper->SetLookupTable(randomLUT());
  }
  if(m_view) m_view->refresh();
}
