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
#include <Support/Representations/RepresentationUtils.h>

// VTK
#include <vtkImageCanvasSource2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkTexture.h>
#include <vtkTubeFilter.h>

using namespace ESPINA::RepresentationUtils;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;

namespace ESPINA
{
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
  RepresentationPipeline::ActorList SegmentationContourPipeline::createActors(const ViewItemAdapter     *item,
                                                                              const RepresentationState &state)
  {
    auto segmentation = dynamic_cast<const SegmentationAdapter *>(item);
    auto planeIndex   = normalCoordinateIndex(m_plane);

    ActorList actors;

    if (isVisible(state) && hasVolumetricData(segmentation->output()))
    {
      auto volume = readLockVolume(segmentation->output());
      Bounds sliceBounds = volume->bounds();

      Nm reslicePoint = crosshairPosition(m_plane, state);

      if (sliceBounds[2*planeIndex] <= reslicePoint && reslicePoint < sliceBounds[2*planeIndex+1])
      {
        sliceBounds.setLowerInclusion(true);
        sliceBounds.setUpperInclusion(toAxis(planeIndex), true);
        sliceBounds[2*planeIndex] = sliceBounds[2*planeIndex+1] = reslicePoint;

        auto slice        = vtkImage(volume, sliceBounds);
        auto patternValue = representationPattern(state);
        auto widthValue   = representationWidth(state);

        auto voxelContour = vtkSmartPointer<vtkVoxelContour2D>::New();
        voxelContour->SetInputData(slice);
        voxelContour->UpdateWholeExtent();

        auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetUpdateExtent(slice->GetExtent());
        mapper->SetColorModeToDefault();
        mapper->ScalarVisibilityOff();
        mapper->StaticOff();

        auto color = m_colorEngine->color(segmentation);
        double rgba[4];
        s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

        auto actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(rgba[0],rgba[1],rgba[2]);
        actor->GetProperty()->SetOpacity(opacity(state) * color.alphaF());
        actor->GetProperty()->Modified();
        actor->SetDragable(false);

        if(widthValue != Width::tiny)
        {
          auto tubes = vtkSmartPointer<vtkTubeFilter>::New();
          tubes->SetInputData(voxelContour->GetOutput());
          tubes->SetUpdateExtent(slice->GetExtent());
          tubes->SetCapping(false);
          tubes->SetGenerateTCoordsToUseLength();
          tubes->SetNumberOfSides(4);
          tubes->SetOffset(1.0);
          tubes->SetOnRatio(1.5);
          tubes->UpdateWholeExtent();

          mapper->SetInputData(tubes->GetOutput());
          mapper->Update();

          tubes->SetRadius(voxelContour->getMinimumSpacing() * (static_cast<int>(widthValue)/10.0));
          tubes->UpdateWholeExtent();

          auto textureIcon = vtkSmartPointer<vtkImageCanvasSource2D>::New();
          textureIcon->SetScalarTypeToUnsignedChar();
          textureIcon->SetExtent(0, 31, 0, 31, 0, 0);
          textureIcon->SetNumberOfScalarComponents(4);
          generateTexture(textureIcon.Get(), representationPattern(state));

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

          actor->GetProperty()->SetLineStipplePattern(hexPatternValue(patternValue));
          actor->GetProperty()->Modified();
        }

        // need to reposition the actor so it will always be over the channels actors'
        double pos[3];
        actor->GetPosition(pos);
        pos[planeIndex] += segmentationDepth(state);
        actor->SetPosition(pos);

        actor->Modified();

        actors << actor;
      }
    }

    return actors;
  }

  //----------------------------------------------------------------------------
  bool SegmentationContourPipeline::pick(ViewItemAdapter *item, const NmVector3 &point) const
  {
    // TODO 2015-04-20 implement pick
    return false;
  }

  //----------------------------------------------------------------------------
  void SegmentationContourPipeline::generateTexture(vtkImageCanvasSource2D *textureIcon, Pattern value) const
  {
    textureIcon->SetDrawColor(255,255,255,255);  // solid white
    textureIcon->FillBox(0,31,0,31);             // for background

    textureIcon->SetDrawColor(0,0,0,0); // transparent

    switch(value)
    {
      case Pattern::dotted:
        textureIcon->FillBox(16, 31, 0, 15);    // checkered pattern
        textureIcon->FillBox(0, 15, 16, 31);
        break;
      case Pattern::dashed:
        textureIcon->FillBox(24, 31, 0, 7);      // small transparent square
        textureIcon->FillBox(0, 7, 24, 31);      // small transparent square
        break;
      case Pattern::normal:
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
      case Pattern::dotted:
        linePattern = 0xAAAA;
        break;
      case Pattern::dashed:
        linePattern = 0xFF00;
        break;
      case Pattern::normal:
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

    return static_cast<SegmentationContourPipeline::Width>(value);
  }

  //----------------------------------------------------------------------------
  SegmentationContourPipeline::Pattern SegmentationContourPipeline::representationPattern(RepresentationState state) const
  {
    auto value = state.getValue<int>(PATTERN);

    return static_cast<SegmentationContourPipeline::Pattern>(value);
  }

} // namespace ESPINA
