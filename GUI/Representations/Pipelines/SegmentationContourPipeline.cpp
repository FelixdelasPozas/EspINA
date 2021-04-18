/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/vtkVoxelContour2D.h>
#include <GUI/Representations/Pipelines/SegmentationContourPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/View/Utils.h>
#include <GUI/Utils/RepresentationUtils.h>

// VTK
#include <vtkImageCanvasSource2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkTexture.h>
#include <vtkTubeFilter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::RepresentationUtils;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::View::Utils;

QString SegmentationContourPipeline::WIDTH   = "WIDTH";
QString SegmentationContourPipeline::PATTERN = "PATTERN";

IntensitySelectionHighlighter SegmentationContourPipeline::s_highlighter;

//----------------------------------------------------------------------------
SegmentationContourPipeline::SegmentationContourPipeline(Plane plane, ColorEngineSPtr colorEngine)
: RepresentationPipeline{"SegmentationContour"}
, m_colorEngine         {colorEngine}
, m_plane               {plane}
{
}

//----------------------------------------------------------------------------
RepresentationState SegmentationContourPipeline::representationState(const ViewItemAdapter *item,
                                                                     const RepresentationState &settings)
{
  auto segmentation = segmentationPtr(item);

  RepresentationState state;

  state.apply(segmentationPipelineSettings(segmentation));
  state.apply(settings);

  return state;
}

//----------------------------------------------------------------------------
RepresentationPipeline::ActorList SegmentationContourPipeline::createActors(ConstViewItemAdapterPtr    item,
                                                                            const RepresentationState &state)
{
  auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);
  auto planeIndex   = normalCoordinateIndex(m_plane);

  ActorList actors;

  if (isVisible(state) && hasVolumetricData(segmentation->output()))
  {
    Bounds sliceBounds = item->bounds();

    Nm reslicePoint = crosshairPosition(m_plane, state);

    if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2*planeIndex+1])
    {
      sliceBounds.setLowerInclusion(true);
      sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
      sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

      auto slice   = vtkImage(readLockVolume(segmentation->output(), DataUpdatePolicy::Ignore), sliceBounds);
      auto pattern = representationPattern(state);
      auto width   = representationWidth(state);

      auto voxelContour = vtkSmartPointer<vtkVoxelContour2D>::New();
      voxelContour->SetInputData(slice);
      voxelContour->UpdateWholeExtent();

      auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
      mapper->SetColorModeToDefault();
      mapper->ScalarVisibilityOff();
      mapper->StaticOff();
      mapper->UpdateWholeExtent();

      auto actor = vtkSmartPointer<vtkActor>::New();
      actor->SetMapper(mapper);
      actor->GetProperty()->Modified();
      actor->SetDragable(false);

      if(width != Width::TINY)
      {
        auto tubes = vtkSmartPointer<vtkTubeFilter>::New();
        tubes->SetInputData(voxelContour->GetOutput());
        tubes->SetCapping(false);
        tubes->SetGenerateTCoordsToUseLength();
        tubes->SetNumberOfSides(4);
        tubes->SetOffset(1.0);
        tubes->SetOnRatio(1.0); // Review: previous truncated value 1.5
        tubes->UpdateWholeExtent();

        mapper->SetInputData(tubes->GetOutput());
        mapper->Update();

        tubes->SetRadius(voxelContour->getMinimumSpacing() * (widthValue(width)/10.0));
        tubes->UpdateWholeExtent();

        auto textureIcon = vtkSmartPointer<vtkImageCanvasSource2D>::New();
        textureIcon->SetScalarTypeToUnsignedChar();
        textureIcon->SetExtent(0, 31, 0, 31, 0, 0);
        textureIcon->SetNumberOfScalarComponents(4);
        generateTexture(textureIcon.Get(), pattern);

        auto texture = vtkSmartPointer<vtkTexture>::New();
        texture->SetInputData(textureIcon->GetOutput());
        texture->SetEdgeClamp(false);
        texture->RepeatOn();
        texture->InterpolateOff();
        texture->Modified();

        actor->SetTexture(texture);
      }
      else
      {
        mapper->SetInputData(voxelContour->GetOutput());
        mapper->Update();

        actor->GetProperty()->SetLineStipplePattern(hexPatternValue(pattern));
        actor->GetProperty()->Modified();
      }

      // need to reposition the actor so it will always be over the channels actors'
      repositionActor(actor, segmentationDepth(state), planeIndex);

      actors << actor;
    }
  }

  return actors;
}

//----------------------------------------------------------------------------
void SegmentationContourPipeline::updateColors(RepresentationPipeline::ActorList& actors, ConstViewItemAdapterPtr item, const RepresentationState& state)
{
  if (actors.size() == 1)
  {
    auto segmentation = segmentationPtr(item);
    QColor color;
    if(segmentation->colorEngine())
    {
      color = segmentation->colorEngine()->color(segmentation);
    }
    else
    {
      color = m_colorEngine->color(segmentation);
    }

    color = s_highlighter.color(color, item->isSelected());

    auto actor    = dynamic_cast<vtkActor *>(actors.first().Get());
    auto property = actor->GetProperty();

    property->SetColor(color.redF(), color.greenF(), color.blueF());
    property->SetOpacity(opacity(state) * color.alphaF());
  }
}

//----------------------------------------------------------------------------
bool SegmentationContourPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
{
  // TODO 2015-04-20 implement pick
  return false;
}

//----------------------------------------------------------------------------
void SegmentationContourPipeline::generateTexture(vtkImageCanvasSource2D *textureIcon, Pattern value) const
{
  textureIcon->SetDrawColor(255,255,255,255);  // solid white
  textureIcon->FillBox(0,31,0,31);             // for background

  textureIcon->SetDrawColor(0,0,0,0);          // transparent

  switch(value)
  {
    case Pattern::DOTTED:
      textureIcon->FillBox(16, 31, 0, 15);     // checkered pattern
      textureIcon->FillBox(0, 15, 16, 31);
      break;
    case Pattern::DASHED:
      textureIcon->FillBox(24, 31, 0, 7);      // small transparent square
      textureIcon->FillBox(0, 7, 24, 31);      // small transparent square
      break;
    case Pattern::NORMAL:
    default:
      // nothing to do
      break;
  }
  textureIcon->Update();
}

//----------------------------------------------------------------------------
int SegmentationContourPipeline::hexPatternValue(Pattern value) const
{
  int linePattern;

  switch(value)
  {
    case Pattern::DOTTED:
      linePattern = 0xAAAA;
      break;
    case Pattern::DASHED:
      linePattern = 0xFF00;
      break;
    case Pattern::NORMAL:
    default:
      linePattern = 0xFFFF;
      break;
  }

  return linePattern;
}

//----------------------------------------------------------------------------
SegmentationContourPipeline::Width SegmentationContourPipeline::representationWidth(RepresentationState state) const
{
  auto value = state.getValue<int>(WIDTH);

  return toWidth(value);
}

//----------------------------------------------------------------------------
SegmentationContourPipeline::Pattern SegmentationContourPipeline::representationPattern(RepresentationState state) const
{
  auto value = state.getValue<int>(PATTERN);

  return toPattern(value);
}

//----------------------------------------------------------------------------
int SegmentationContourPipeline::widthValue(Width width)
{
  return static_cast<int>(width);
}

//----------------------------------------------------------------------------
int SegmentationContourPipeline::patternValue(Pattern pattern)
{
  return static_cast<int>(pattern);
}

//----------------------------------------------------------------------------
SegmentationContourPipeline::Pattern SegmentationContourPipeline::toPattern(int value)
{
  return static_cast<SegmentationContourPipeline::Pattern>(value);
}

//----------------------------------------------------------------------------
SegmentationContourPipeline::Width SegmentationContourPipeline::toWidth(int value)
{
  return static_cast<SegmentationContourPipeline::Width>(value);
}
