/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "ContourRepresentation.h"
#include "GraphicalRepresentationEmptySettings.h"
#include "ContourRepresentationSettings.h"
#include <Core/ColorEngines/TransparencySelectionHighlighter.h>
#include <Core/EspinaTypes.h>
#include <GUI/QtWidget/SliceView.h>
#include <GUI/QtWidget/VolumeView.h>
#include <GUI/vtkWidgets/vtkVoxelContour2D.h>

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

using namespace EspINA;

TransparencySelectionHighlighter *ContourRepresentation::s_highlighter = new TransparencySelectionHighlighter();

//-----------------------------------------------------------------------------
ContourRepresentation::ContourRepresentation(SegmentationVolumeSPtr data,
                                             EspinaRenderView      *view)
: SegmentationGraphicalRepresentation(view)
, m_data(data)
, m_width(medium)
, m_pattern(normal)
, m_minSpacing(0)
{
  setLabel(tr("Contour"));
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSettings *ContourRepresentation::settingsWidget()
{
  return new  ContourRepresentationSettings();
}

//-----------------------------------------------------------------------------
void ContourRepresentation::setColor(const QColor &color)
{
  GraphicalRepresentation::setColor(color);

  if (m_actor != NULL)
  {
    vtkSmartPointer<vtkLookupTable> lut = s_highlighter->lut(m_color, m_highlight);

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
  SegmentationGraphicalRepresentation::setHighlighted(highlighted);

  if (m_actor != NULL)
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
  if (m_actor == NULL)
    return false;

  return m_actor.GetPointer() == actor;
}

//-----------------------------------------------------------------------------
bool ContourRepresentation::isInside(Nm point[3])
{
  Q_ASSERT(m_data.get());
  Q_ASSERT(m_data->toITK().IsNotNull());

  itkVolumeType::IndexType voxel = m_data->index(point[0], point[1], point[2]);

  return m_data->volumeRegion().IsInside(voxel)
  && m_data->toITK()->GetPixel(voxel) == SEG_VOXEL_VALUE;
}

//-----------------------------------------------------------------------------
void ContourRepresentation::initializePipeline()
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  SliceView *view = reinterpret_cast<SliceView *>(m_view);

  m_reslice = vtkSmartPointer<vtkImageReslice>::New();
  m_reslice->SetInputConnection(m_data->toVTK());
  m_reslice->SetOutputDimensionality(2);
  m_reslice->SetResliceAxes(slicingMatrix(view));
  m_reslice->AutoCropOutputOn();
  m_reslice->SetNumberOfThreads(1);
  m_reslice->Update();

  m_voxelContour = vtkSmartPointer<vtkVoxelContour2D>::New();
  m_voxelContour->SetInputConnection(m_reslice->GetOutputPort());
  m_voxelContour->Update();

  m_tubes = vtkSmartPointer<vtkTubeFilter>::New();
  m_tubes->SetInput(m_voxelContour->GetOutput());
  m_tubes->SetCapping(false);
  m_tubes->SetGenerateTCoordsToUseLength();
  m_tubes->SetNumberOfSides(4);
  m_tubes->SetOffset(1.0);
  m_tubes->SetOnRatio(1.5);

  m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_mapper->SetInputConnection(m_tubes->GetOutputPort());
  m_mapper->SetColorModeToDefault();
  m_mapper->ScalarVisibilityOff();
  m_mapper->StaticOff();

  updateWidth();
  m_mapper->Update();

  double rgba[4];
  s_highlighter->lut(m_color, m_highlight)->GetTableValue(1, rgba);

  updatePattern();

  m_textureIcon = vtkSmartPointer<vtkImageCanvasSource2D>::New();
  m_textureIcon->SetScalarTypeToUnsignedChar();
  m_textureIcon->SetExtent(0, 31, 0, 31, 0, 0);
  m_textureIcon->SetNumberOfScalarComponents(4);

  m_texture = vtkSmartPointer<vtkTexture>::New();
  m_texture->SetInputConnection(m_textureIcon->GetOutputPort());
  m_texture->SetEdgeClamp(false);
  m_texture->RepeatOn();
  m_texture->InterpolateOff();
  m_texture->ReleaseDataFlagOn();

  generateTexture();

  m_actor = vtkSmartPointer<vtkActor>::New();
  m_actor->SetMapper(m_mapper);
  m_actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
  m_actor->GetProperty()->SetOpacity(rgba[3]);
  m_actor->GetProperty()->Modified();
  m_actor->SetVisibility(isVisible());
  m_actor->SetDragable(false);
  m_actor->Modified();
  m_actor->SetTexture(m_texture);

  // need to reposition the actor so it will always be over the channels actors'
  double pos[3];
  m_actor->GetPosition(pos);
  pos[view->plane()] = view->segmentationDepth();
  m_actor->SetPosition(pos);
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updateRepresentation()
{
  if (m_actor != NULL)
  {
    m_reslice->UpdateWholeExtent();
    m_mapper->UpdateWholeExtent();
    m_tubes->Modified();
    m_actor->Modified();
  }
}

//-----------------------------------------------------------------------------
QList<vtkProp*> ContourRepresentation::getActors()
{
  QList<vtkProp *> list;

  if (m_actor == NULL)
    initializePipeline();

  list << m_actor.GetPointer();

  return list;
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSPtr ContourRepresentation::cloneImplementation(SliceView *view)
{
  ContourRepresentation *representation = new ContourRepresentation(m_data, view);
  representation->setView(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updateVisibility(bool visible)
{
  if (m_actor != NULL)
    m_actor->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void ContourRepresentation::updatePipelineConnections()
{
  if (m_actor == NULL)
    return;

  if (m_reslice->GetInputConnection(0,0) != m_data->toVTK())
  {
    m_reslice->SetInputConnection(m_data->toVTK());
    m_reslice->Update();
  }
}

//-----------------------------------------------------------------------------
void ContourRepresentation::setLineWidth(ContourRepresentation::LineWidth width)
{
  if (m_width == width)
    return;

  m_width = width;
  updateWidth();

  foreach (GraphicalRepresentationSPtr clone, m_clones)
  {
    ContourRepresentation *contourClone = dynamic_cast<ContourRepresentation *>(clone.get());
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

  foreach (GraphicalRepresentationSPtr clone, m_clones)
  {
    ContourRepresentation *contourClone = dynamic_cast<ContourRepresentation *>(clone.get());
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
      m_mapper->SetInputConnection(m_voxelContour->GetOutputPort());
      updatePattern();
    }
    else
    {
      m_mapper->SetInputConnection(m_tubes->GetOutputPort());
      m_tubes->SetRadius(m_voxelContour->getMinimumSpacing() * ((int)m_width)/10.0);
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

    if (m_actor.GetPointer() != NULL)
    {
      m_actor->GetProperty()->SetLineStipplePattern(linePattern);
      m_actor->SetTexture(NULL);
      m_actor->GetProperty()->Modified();
    }
  }
  else
  {
    if (m_actor.GetPointer() != NULL)
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
  m_textureIcon->UpdateWholeExtent();
}