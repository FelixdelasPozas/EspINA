/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "ContourRepresentation.h"
#include "RepresentationEmptySettings.h"
#include "ContourRepresentationSettings.h"
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>
#include <Core/EspinaTypes.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <GUI/View/Widgets/Contour/vtkVoxelContour2D.h>

// VTK
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageReslice.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkTubeFilter.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkTexture.h>

using namespace ESPINA;

const Representation::Type ContourRepresentation::TYPE = "Contour";

TransparencySelectionHighlighter *ContourRepresentation::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
ContourRepresentation::ContourRepresentation(DefaultVolumetricDataSPtr data,
                                             RenderView               *view)
: Representation{view}
, m_planeIndex  {-1}
, m_reslicePoint{-1}
, m_data        {data}
, m_voxelContour{nullptr}
, m_textureIcon {nullptr}
, m_texture     {nullptr}
, m_tubes       {nullptr}
, m_mapper      {nullptr}
, m_actor       {nullptr}
, m_width       {medium}
, m_pattern     {normal}
, m_minSpacing  {0}
{
  setType(TYPE);
}

//-----------------------------------------------------------------------------
RepresentationSettings *ContourRepresentation::settingsWidget()
{
  return new ContourRepresentationSettings();
}

//-----------------------------------------------------------------------------
void ContourRepresentation::setColor(const QColor &color)
{
  Representation::setColor(color);

  if (m_actor != nullptr)
  {
    auto lut = s_highlighter->lut(m_color, m_highlight);
    double rgba[4];
    s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

    m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->GetProperty()->Modified();
  }
}

//-----------------------------------------------------------------------------
void ContourRepresentation::setHighlighted(bool highlighted)
{
  Representation::setHighlighted(highlighted);

  if (m_actor != nullptr)
  {
    double rgba[4];
    s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

    m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->GetProperty()->Modified();
  }
}

//-----------------------------------------------------------------------------
bool ContourRepresentation::hasActor(vtkProp *actor) const
{
  if (m_actor == nullptr)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
bool ContourRepresentation::isInside(const NmVector3 &point) const
{
  Q_ASSERT(m_data.get());

  Bounds bounds{ '[', point[0], point[0], point[1], point[1], point[2], point[2], ']'};

  if (!intersect(m_data->bounds(), bounds))
    return false;

  auto voxel = m_data->itkImage(bounds);

  return (SEG_VOXEL_VALUE == *(static_cast<unsigned char*>(voxel->GetBufferPointer())));
}

//-----------------------------------------------------------------------------
void ContourRepresentation::initializePipeline()
{
  if (m_planeIndex == -1)
    return;

  Bounds imageBounds = m_data->bounds();
  bool valid = imageBounds[2*m_planeIndex] <= m_crosshair[m_planeIndex] && m_crosshair[m_planeIndex] <= imageBounds[2*m_planeIndex+1];

  vtkSmartPointer<vtkImageData> image = nullptr;

  if (valid)
  {
    m_reslicePoint = m_crosshair[m_planeIndex];
    imageBounds[2*m_planeIndex] = imageBounds[(2*m_planeIndex)+1] = m_reslicePoint;
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);

    image = vtkImage(m_data, imageBounds);
  }
  else
  {
    m_reslicePoint = -1;
    int extent[6] = { 0,1,0,1,0,1 };
    extent[2*m_planeIndex + 1] = extent[2*m_planeIndex];
    image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(extent);

    auto info = image->GetInformation();
    vtkImageData::SetScalarType(VTK_UNSIGNED_CHAR, info);
    vtkImageData::SetNumberOfScalarComponents(1, info);
    image->SetInformation(info);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    image->Modified();
    memset(image->GetScalarPointer(), SEG_BG_VALUE, image->GetNumberOfPoints());
  }

  m_voxelContour = vtkSmartPointer<vtkVoxelContour2D>::New();
  m_voxelContour->SetInputData(image);
  m_voxelContour->UpdateWholeExtent();

  m_tubes = vtkSmartPointer<vtkTubeFilter>::New();
  m_tubes->SetInputData(m_voxelContour->GetOutput());
  m_tubes->SetUpdateExtent(image->GetExtent());
  m_tubes->SetCapping(false);
  m_tubes->SetGenerateTCoordsToUseLength();
  m_tubes->SetNumberOfSides(4);
  m_tubes->SetOffset(1.0);
  m_tubes->SetOnRatio(1.5);
  m_tubes->UpdateWholeExtent();

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetInputConnection(m_tubes->GetOutputPort());
  m_mapper->SetUpdateExtent(image->GetExtent());
  m_mapper->SetColorModeToDefault();
  m_mapper->ScalarVisibilityOff();
  m_mapper->StaticOff();

  m_textureIcon = vtkSmartPointer<vtkImageCanvasSource2D>::New();
  m_textureIcon->SetScalarTypeToUnsignedChar();
  m_textureIcon->SetExtent(0, 31, 0, 31, 0, 0);
  m_textureIcon->SetNumberOfScalarComponents(4);
  generateTexture();

  m_texture = vtkSmartPointer<vtkTexture>::New();
  m_texture->SetInputConnection(m_textureIcon->GetOutputPort());
  m_texture->SetEdgeClamp(false);
  m_texture->RepeatOn();
  m_texture->InterpolateOff();
  m_texture->Modified();

  updateWidth();
  m_mapper->Update();

  double rgba[4];
  s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
  m_actor->GetProperty()->SetOpacity(rgba[3]);
  m_actor->GetProperty()->Modified();
  m_actor->SetVisibility(isVisible());
  m_actor->SetDragable(false);
  m_actor->SetTexture(m_texture);
  m_actor->Modified();

  updatePattern();

  m_lastUpdatedTime = m_data->lastModified();
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updateRepresentation()
{
  setCrosshairPoint(m_view->crosshairPoint());

  Bounds imageBounds = m_data->bounds();

  bool valid = imageBounds[2*m_planeIndex] <= m_crosshair[m_planeIndex] && m_crosshair[m_planeIndex] <= imageBounds[2*m_planeIndex+1];

  if (m_actor != nullptr && ((m_crosshair[m_planeIndex] != m_reslicePoint) || needUpdate()) && valid && isVisible())
  {
    m_reslicePoint = m_crosshair[m_planeIndex];

    imageBounds[2*m_planeIndex] = imageBounds[(2*m_planeIndex)+1] = m_reslicePoint;
    imageBounds.setLowerInclusion(true);
    imageBounds.setUpperInclusion(toAxis(m_planeIndex), true);

    auto image = vtkImage(m_data, imageBounds);

    m_voxelContour->SetInputData(image);
    m_voxelContour->UpdateWholeExtent();
    m_mapper->UpdateWholeExtent();
    m_tubes->UpdateWholeExtent();

    double rgba[4];
    s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

    m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
    m_actor->GetProperty()->SetOpacity(rgba[3]);
    m_actor->GetProperty()->Modified();

    double pos[3];
    auto view2d = dynamic_cast<View2D*>(m_view);
    m_actor->GetPosition(pos);
    pos[normalCoordinateIndex(view2d->plane())] += view2d->segmentationDepth();
    m_actor->SetPosition(pos);
    m_actor->Modified();

    m_lastUpdatedTime = m_data->lastModified();
  }

  if (m_actor != nullptr)
    m_actor->SetVisibility(valid && isVisible());
}

//-----------------------------------------------------------------------------
QList<vtkProp*> ContourRepresentation::getActors()
{
  QList<vtkProp *> list;

  if (m_actor == nullptr)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
RepresentationSPtr ContourRepresentation::cloneImplementation(View2D *view)
{
  auto representation = new ContourRepresentation(m_data, view);
  representation->setView(view);
  representation->setPlane(view->plane());

  return RepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updateVisibility(bool visible)
{
  if(visible && m_actor != nullptr && needUpdate())
    updateRepresentation();

  if (m_actor != nullptr)
    m_actor->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void ContourRepresentation::setLineWidth(ContourRepresentation::LineWidth width)
{
  if (m_width == width)
    return;

  m_width = width;
  updateWidth();

  for (auto clone: m_clones)
  {
    auto contourClone = dynamic_cast<ContourRepresentation *>(clone.get());
    contourClone->setLineWidth(width);
  }
}

//-----------------------------------------------------------------------------
ContourRepresentation::LineWidth ContourRepresentation::lineWidth() const
{
  return m_width;
}

//-----------------------------------------------------------------------------
void ContourRepresentation::setLinePattern(ContourRepresentation::LinePattern pattern)
{
  if (m_pattern == pattern)
    return;

  m_pattern = pattern;
  updatePattern();

  for (auto clone: m_clones)
  {
    auto contourClone = dynamic_cast<ContourRepresentation *>(clone.get());
    contourClone->setLinePattern(pattern);
  }
}

//-----------------------------------------------------------------------------
ContourRepresentation::LinePattern ContourRepresentation::linePattern() const
{
  return m_pattern;
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updateWidth()
{
  if (m_tubes)
  {
    if (m_width == tiny)
    {
      m_mapper->SetInputData(m_voxelContour->GetOutput());
      updatePattern();
    }
    else
    {
      m_mapper->SetInputData(m_tubes->GetOutput());
      m_tubes->SetRadius(m_voxelContour->getMinimumSpacing() * ((int)m_width)/10.0);
      m_tubes->UpdateWholeExtent();
    }
  }
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updatePattern()
{
  if (m_width == tiny)
  {
    int linePattern;
    switch(m_pattern)
    {
      case dotted:
        linePattern = 0xAAAA;
        break;
      case dashed:
        linePattern = 0xFF00;
        break;
      case normal:
      default:
        linePattern = 0xFFFF;
        break;
    }

    if (m_actor.GetPointer() != nullptr)
    {
      m_actor->GetProperty()->SetLineStipplePattern(linePattern);
      m_actor->SetTexture(nullptr);
      m_actor->GetProperty()->Modified();
    }
  }
  else
  {
    if (m_actor.GetPointer() != nullptr)
    {
      generateTexture();
      m_actor->SetTexture(m_texture);
    }
  }
}

//-----------------------------------------------------------------------------
void ContourRepresentation::generateTexture()
{
  m_textureIcon->SetDrawColor(255,255,255,255);  // solid white
  m_textureIcon->FillBox(0,31,0,31);             // for background

  m_textureIcon->SetDrawColor(0,0,0,0); // transparent

  switch(m_pattern)
  {
    case dotted:
      m_textureIcon->FillBox(16, 31, 0, 15);    // checkered pattern
      m_textureIcon->FillBox(0, 15, 16, 31);
      break;
    case dashed:
      m_textureIcon->FillBox(24, 31, 0, 7);      // small transparent square
      m_textureIcon->FillBox(0, 7, 24, 31);      // small transparent square
      break;
    case normal:
    default:
      // nothing to do
      break;
  }
  m_textureIcon->Update();
}
