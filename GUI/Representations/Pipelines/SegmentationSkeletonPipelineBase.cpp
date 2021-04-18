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
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Pipelines/SegmentationSkeletonPipelineBase.h>
#include <GUI/Utils/RepresentationUtils.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>

// VTK
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkDataArray.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkMapper.h>
#include <vtkActor2D.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkFollower.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkTexture.h>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::RepresentationUtils;

IntensitySelectionHighlighter SegmentationSkeletonPipelineBase::s_highlighter;

//--------------------------------------------------------------------
SegmentationSkeletonPipelineBase::SegmentationSkeletonPipelineBase(const QString &id, ColorEngineSPtr colorEngine)
: RepresentationPipeline{id}
, m_colorEngine         {colorEngine}
{
}

//--------------------------------------------------------------------
void SegmentationSkeletonPipelineBase::updateColors(RepresentationPipeline::ActorList& actors,
                                                    ConstViewItemAdapterPtr            item,
                                                    const RepresentationState&         state)
{
  auto segmentation = segmentationPtr(item);
  if(!segmentation || !m_colorEngine) return;

  QColor color;
  if(segmentation->colorEngine() != nullptr)
  {
    color = segmentation->colorEngine()->color(segmentation);
  }
  else
  {
    color = m_colorEngine->color(segmentation);
  }

  for(auto actor: actors)
  {
    auto follower = vtkFollower::SafeDownCast(actor.Get());
    if(follower) continue;

    auto actor2D = vtkActor2D::SafeDownCast(actor.Get());

    if(actor2D)
    {
      actor2D->SetVisibility(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected());
      auto mapper = vtkLabelPlacementMapper::SafeDownCast(actor2D->GetMapper());
      if(mapper)
      {
        mapper->SetBackgroundColor(color.redF(), color.greenF(), color.blueF());
        mapper->Update();
      }

      actor2D->Modified();
    }

    auto actorVTK = vtkActor::SafeDownCast(actor.Get());

    if(actorVTK)
    {
      auto truncatedMapper = vtkGlyph3DMapper::SafeDownCast(actorVTK->GetMapper());
      if(truncatedMapper) continue;

      auto data = vtkPolyData::SafeDownCast(actorVTK->GetMapper()->GetInput());
      if(!data) return;

      auto colors      = vtkUnsignedCharArray::SafeDownCast(data->GetCellData()->GetScalars());
      auto cellChanges = vtkIntArray::SafeDownCast(data->GetCellData()->GetAbstractArray("ChangeColor"));

      if (colors)
      {
        const auto limit = data->GetNumberOfLines();
        data->GetLines()->InitTraversal();
        for (int i = 0; i < limit; ++i)
        {
          double rgba[4];

          if (!cellChanges || cellChanges->GetValue(i) == 0)
          {
            s_highlighter.lut(color, item->isSelected())->GetTableValue(1, rgba);
          }
          else
          {
            unsigned char values[3];
            colors->GetTypedTuple(i, values);

            auto custom = QColor::fromRgb(values[0], values[1], values[2]);
            s_highlighter.lut(custom, item->isSelected())->GetTableValue(1, rgba);
          }

          colors->SetTuple3(i, rgba[0] * 255, rgba[1] * 255, rgba[2] * 255);
        }

        colors->Modified();
      }
      else
      {
        actorVTK->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
      }

      auto width = item->isSelected() ? 2 : 0;
      width += SegmentationSkeletonPoolSettings::getWidth(state);

      actorVTK->GetMapper()->Update();
      actorVTK->GetProperty()->SetLineWidth(width);
      actorVTK->Modified();
    }
  }
}

//--------------------------------------------------------------------
bool SegmentationSkeletonPipelineBase::pick(ConstViewItemAdapterPtr item, const NmVector3& point) const
{
  if(hasSkeletonData(item->output()))
  {
    auto skeleton = readLockSkeleton(item->output(), DataUpdatePolicy::Ignore)->skeleton();
    auto lines = skeleton->GetLines();
    auto spacing = item->output()->spacing();
    double projection[3];
    auto points = skeleton->GetPoints();
    long long npts;
    long long *pts;

    // NOTE: vtkLine::DistanceToLine(p0, lineP0, lineP1, t, closest) is pure shit. Thank god for a little
    // knowledge of computational geometry.
    lines->InitTraversal();
    while(lines->GetNextCell(npts, pts))
    {
      for(int i = 0; i < npts-1; ++i)
      {
        double pos_i[3], pos_j[3];
        points->GetPoint(pts[i], pos_i);
        points->GetPoint(pts[i+1], pos_j);

        double v[3]{pos_j[0]-pos_i[0], pos_j[1]-pos_i[1], pos_j[2]-pos_i[2]};
        double w[3]{point[0]-pos_i[0], point[1]-pos_i[1], point[2]-pos_i[2]};

        double dotwv = 0;
        double dotvv = 0;
        for(auto ii: {0,1,2})
        {
          dotwv += w[ii]*v[ii];
          dotvv += v[ii]*v[ii];
        }

        double r = dotwv / dotvv;

        if(r <= 0)
        {
          std::memcpy(projection, pos_i, 3*sizeof(double));
        }
        else
        {
          if(r >= 1)
          {
            std::memcpy(projection, pos_j, 3*sizeof(double));
          }
          else
          {
            projection[0] = pos_i[0] + r*(pos_j[0] - pos_i[0]);
            projection[1] = pos_i[1] + r*(pos_j[1] - pos_i[1]);
            projection[2] = pos_i[2] + r*(pos_j[2] - pos_i[2]);
          }
        }

        double distance = std::pow(projection[0] - point[0], 2) + std::pow(projection[1] - point[1], 2) + std::pow(projection[2] - point[2], 2);
        auto maxSpacing = std::max(spacing[0], std::max(spacing[1], spacing[2]));

        if(distance < (maxSpacing * maxSpacing)) return true;
      }
    }
  }

  return false;
}

//--------------------------------------------------------------------
RepresentationState SegmentationSkeletonPipelineBase::representationState(ConstViewItemAdapterPtr    item,
                                                                          const RepresentationState &settings)
{
  RepresentationState state;

  auto segmentation = segmentationPtr(item);

  if(segmentation)
  {
    state.apply(segmentationPipelineSettings(segmentation));
    state.apply(settings);
  }

  return state;
}

//--------------------------------------------------------------------
void ESPINA::GUI::Representations::SegmentationSkeletonPipelineBase::stippledLine(vtkSmartPointer<vtkActor>& actor, int pattern, int repeat)
{
  // Create image texture
  int dimension = 128 * repeat;
  const unsigned char ON = 255;
  const unsigned char OFF = 0;

  auto image = vtkSmartPointer<vtkImageData>::New();
  image->SetDimensions(dimension, 1, 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
  image->SetExtent(0, dimension - 1, 0, 0, 0, 0);
  auto pixel = static_cast<unsigned char *>(image->GetScalarPointer());
  for(int k = 0; k < 8; ++k)
  {
    for (int i = 0; i < 16; ++i)
    {
      unsigned int mask = (1 << i);
      unsigned int bit = (pattern & mask) >> i;
      unsigned char value = static_cast<unsigned char>(bit);

      for (int j = 0; j < repeat; ++j)
      {
        *(pixel+0) = ON;
        *(pixel+1) = ON;
        *(pixel+2) = ON;
        *(pixel+3) = (value == 0 ? OFF : ON);
        pixel += 4;
      }
    }
  }

  auto polyData = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInput());

  // Create texture coordinates
  auto tcoords = vtkSmartPointer<vtkDoubleArray>::New();
  tcoords->SetNumberOfComponents(1);
  const auto numPoints = polyData->GetNumberOfPoints();

  tcoords->SetNumberOfTuples(numPoints);
  for (int i = 0; i < numPoints; ++i)
  {
    double value = std::fmod(static_cast<double>(i) * .5, 128.*repeat);
    tcoords->SetTypedTuple(i, &value);
  }

  polyData->GetPointData()->SetTCoords(tcoords);

  auto texture = vtkSmartPointer<vtkTexture>::New();
  texture->SetInputData(image);
  texture->InterpolateOff();
  texture->RepeatOn();
  texture->Modified();

  actor->SetTexture(texture);
}
