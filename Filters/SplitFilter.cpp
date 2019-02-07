/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "SplitFilter.h"
#include <Core/Types.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/StatePair.h>

// ITK
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

// VTK
#include <vtkImageStencilData.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageToImageStencil.h>
#include <vtkPlane.h>

// Qt
#include <QDir>

using namespace ESPINA;
using namespace ESPINA::Core;

const QString PLANE_ORIGIN = "PlaneOrigin";
const QString PLANE_NORMAL = "PlaneNormal";

//-----------------------------------------------------------------------------
SplitFilter::SplitFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Filter{inputs, type, scheduler}
, m_ignoreCurrentOutputs{false}
, m_stencil             {nullptr}
, m_plane               {nullptr}
{
}

//-----------------------------------------------------------------------------
void SplitFilter::execute()
{
  reportProgress(0);
  if (!canExecute()) return;

  Q_ASSERT(m_inputs.size() == 1);
  auto input  = m_inputs[0]->output();

  if (!m_stencil && !fetchCacheStencil())
  {
    if (m_outputs.isEmpty())
    {
      Q_ASSERT(false);
    }
    else
    {
      // read-only filter.
      return;
    }
  }

  int shift[3]; // Stencil origin differ from creation to fetch

  double origin[3];
  double spacing[3];

  m_stencil->GetOrigin(origin);
  m_stencil->GetSpacing(spacing);

  for (const auto i: {0,1,2}) shift[i] = vtkMath::Round(origin[i] / spacing[i]);

  if(hasVolumetricData(input))
  {
    auto volume = readLockVolume(input);

    auto itkVolume = volume->itkImage();
    auto itkRegion = itkVolume->GetLargestPossibleRegion();

    auto split1 = itkVolumeType::New();
    split1->SetRegions(itkVolume->GetLargestPossibleRegion());
    split1->SetSpacing(itkVolume->GetSpacing());
    split1->SetOrigin(itkVolume->GetOrigin());
    split1->Allocate();
    split1->Update();

    auto split2 = itkVolumeType::New();
    split2->SetRegions(itkVolume->GetLargestPossibleRegion());
    split2->SetSpacing(itkVolume->GetSpacing());
    split2->SetOrigin(itkVolume->GetOrigin());
    split2->Allocate();
    split2->Update();

    reportProgress(25);
    if (!canExecute()) return;

    bool hasVoxels1 = false;
    bool hasVoxels2 = false;

    itk::ImageRegionConstIterator<itkVolumeType> it(itkVolume, itkVolume->GetLargestPossibleRegion());
    itk::ImageRegionIterator<itkVolumeType> split1it(split1, split1->GetLargestPossibleRegion());
    itk::ImageRegionIterator<itkVolumeType> split2it(split2, split2->GetLargestPossibleRegion());

    it.GoToBegin();
    split1it.GoToBegin();
    split2it.GoToBegin();

    for(; !it.IsAtEnd(); ++it, ++split1it, ++split2it)
    {
      auto value = it.Value();
      itkVolumeType::IndexType index = it.GetIndex();
      if (m_stencil->IsInside(index[0] - shift[0], index[1] - shift[1], index[2] - shift[2]))
      {
        split1it.Set(value);
        split2it.Set(SEG_BG_VALUE);
        hasVoxels1 |= (value == SEG_VOXEL_VALUE);
      }
      else
      {
        split1it.Set(SEG_BG_VALUE);
        split2it.Set(value);
        hasVoxels2 |= (value == SEG_VOXEL_VALUE);
      }
    }

    reportProgress(50);
    if (!canExecute()) return;

    itkVolumeType::Pointer volumes[2]{split1, split2};

    if (hasVoxels1 && hasVoxels2)
    {
      auto iSpacing = input->spacing();

      for(auto i: {0, 1})
      {
        auto bounds  = minimalBounds<itkVolumeType>(volumes[i], SEG_BG_VALUE);
        auto iVolume = std::make_shared<SparseVolume<itkVolumeType>>(bounds, iSpacing);

        iVolume->draw(volumes[i], bounds);

        if (!m_outputs.contains(i))
        {
          m_outputs[i] = std::make_shared<Output>(this, i, iSpacing);
        }
        m_outputs[i]->setData(iVolume);
        m_outputs[i]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[i].get()));
        m_outputs[i]->setSpacing(iSpacing);

        reportProgress(75 + 25*i);
        if (!canExecute()) return;
      }

      m_ignoreCurrentOutputs = false;
    }
  }
  else
  {
    if(hasSkeletonData(input))
    {
      const auto data    = readLockSkeleton(input);
      const auto spacing = data->bounds().spacing();
      auto skeleton      = Core::toSkeletonDefinition(data->skeleton());

      Core::SkeletonDefinition splits[2];

      for(auto node: skeleton.nodes)
      {
        if (m_plane->Evaluate(m_plane->GetNormal(), m_plane->GetOrigin(), node->position) > 0)
        {
          splits[0].nodes << node;
        }
        else
        {
          splits[1].nodes << node;
        }
      }

      reportProgress(25);
      if (!canExecute()) return;

      if(!splits[0].nodes.isEmpty() && !splits[1].nodes.isEmpty())
      {
        for(auto node: splits[0].nodes)
        {
          for(auto connNode: node->connections.keys())
          {
            if(!splits[0].nodes.contains(connNode))
            {
              double t = 0;
              double intersection[3];

              if(0 != m_plane->IntersectWithLine(node->position, connNode->position, t, intersection))
              {
                for(const auto j: {0,1,2}) intersection[j] = vtkMath::Round(intersection[j]/spacing[j]) * spacing[j];
                auto edge = node->connections[connNode];

                auto intersection1 = new SkeletonNode{intersection};
                intersection1->connections.insert(node, edge);
                splits[0].nodes << intersection1;
                node->connections.insert(intersection1, edge);
                node->connections.remove(connNode);

                auto intersection2 = new SkeletonNode{intersection};
                intersection2->connections.insert(connNode, edge);
                splits[1].nodes << intersection2;
                connNode->connections.insert(intersection2, edge);
                connNode->connections.remove(node);
              }
            }
          }
        }
        reportProgress(50);
        if (!canExecute()) return;

        for (auto i: { 0, 1 })
        {
          splits[i].strokes = skeleton.strokes;
          splits[i].edges   = skeleton.edges;

          // this could delete nodes in skeleton definition, so we need to join nodes later before clearing the structs.
          Core::cleanSkeletonStrokes(splits[i]);
          Core::removeIsolatedNodes(splits[i].nodes);
          Core::mergeSamePositionNodes(splits[i].nodes);
          Core::adjustStrokeNumbers(splits[i]);

          auto splitSkeleton = Core::toPolyData(splits[i]);

          if (!m_outputs.contains(i))
          {
            m_outputs[i] = std::make_shared<Output>(this, i, spacing);
          }
          m_outputs[i]->setData(std::make_shared<RawSkeleton>(splitSkeleton, spacing));
          m_outputs[i]->setSpacing(spacing);

          reportProgress(75 + 25 * i);
          if (!canExecute()) return;
        }
      }

      m_ignoreCurrentOutputs = false;

      // we don't need to clear() splits because the nodes will be deleted here and the rest (count, strokes, edges) are structs.
      skeleton.nodes = splits[0].nodes + splits[1].nodes;
      skeleton.clear();
    }
  }
}

//----------------------------------------------------------------------------
bool SplitFilter::ignoreStorageContent() const
{
  return false;
}

//----------------------------------------------------------------------------
bool SplitFilter::needUpdate() const
{
  return !validOutput(0) || ignoreStorageContent();
}

//-----------------------------------------------------------------------------
bool SplitFilter::fetchCacheStencil() const
{
  if (m_ignoreCurrentOutputs)
  {
    return false;
  }

  bool returnVal = false;

  if (storage()->exists(stencilFile()))
  {
    QString fileName = storage()->absoluteFilePath(stencilFile());
    const QString utfFilename = fileName.toUtf8();
    const QString asciiFilename = utfFilename.toLatin1();

    auto stencilReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    stencilReader->SetFileName(asciiFilename.toStdString().c_str());
    stencilReader->ReadAllFieldsOn();
    stencilReader->Update();

    auto image = vtkImageData::SafeDownCast(stencilReader->GetOutput());
    auto spacing = output(0)->spacing();
    image->SetSpacing(spacing[0], spacing[1], spacing[2]);
    image->Modified();

    auto convert = vtkSmartPointer<vtkImageToImageStencil>::New();
    convert->SetInputData(image);
    convert->ThresholdBetween(1,1);
    convert->Update();

    m_stencil = vtkSmartPointer<vtkImageStencilData>::New();
    m_stencil->DeepCopy(convert->GetOutput());

    changeStencilSpacing(output(0)->spacing());

    returnVal = true;
  }

  return returnVal;
}

//-----------------------------------------------------------------------------
void SplitFilter::changeStencilSpacing(const NmVector3& spacing) const
{
  if(areEqual(spacing[0], 0) || areEqual(spacing[1], 0) || areEqual(spacing[2], 0))
  {
    qWarning() << name() << "(" << uuid() << ") attempting to use invalid spacing" << spacing;
  }
  else
  {
    double stencilSpacing[3], stencilOrigin[3];
    m_stencil->GetSpacing(stencilSpacing);

    if((spacing[0] == stencilSpacing[0]) &&
       (spacing[1] == stencilSpacing[1]) &&
       (spacing[2] == stencilSpacing[2]))
    {
      return;
    }

    m_stencil->GetOrigin(stencilOrigin);

    NmVector3 ratio{spacing[0]/stencilSpacing[0],
                    spacing[1]/stencilSpacing[1],
                    spacing[2]/stencilSpacing[2]};

    for (int i = 0; i < 3; ++i)
    {
      stencilOrigin[i] *= ratio[i];
      stencilSpacing[i] = spacing[i];
    }

    m_stencil->SetOrigin(stencilOrigin);
    m_stencil->SetSpacing(stencilSpacing);
  }

  m_stencil->Modified();
}

//-----------------------------------------------------------------------------
void SplitFilter::setStencil(vtkSmartPointer<vtkImageStencilData> stencil)
{
  m_stencil = stencil;
  m_ignoreCurrentOutputs = true;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkImageStencilData> SplitFilter::stencil() const
{
  if (!m_stencil)
  {
    fetchCacheStencil();
  }

  return m_stencil;
}

//-----------------------------------------------------------------------------
void SplitFilter::changeSpacing(const NmVector3& origin, const NmVector3& spacing)
{
  if (m_stencil)
  {
    changeStencilSpacing(spacing);
  }

  Filter::changeSpacing(origin, spacing);
}

//-----------------------------------------------------------------------------
Snapshot SplitFilter::saveFilterSnapshot() const
{
  Snapshot snapshot;

  // NOTE: not aborting if the file is not found fixes a bug in earlier versions that didn't store
  // the vti file in the seg, this renders this filter a read-only filter, that is, cannot be executed
  // again.
  if(m_stencil || fetchCacheStencil())
  {
    auto convert = vtkSmartPointer<vtkImageStencilToImage>::New();
    convert->SetInputData(m_stencil);
    convert->SetInsideValue(1);
    convert->SetOutsideValue(0);
    convert->SetOutputScalarTypeToUnsignedChar();
    convert->Update();

    auto stencilImage = vtkSmartPointer<vtkImageData>::New();
    stencilImage->DeepCopy(convert->GetOutput());
    stencilImage->SetOrigin(m_stencil->GetOrigin());
    stencilImage->SetSpacing(m_stencil->GetSpacing());
    stencilImage->Modified();

    auto stencilWriter = vtkSmartPointer<vtkGenericDataObjectWriter>::New();
    stencilWriter->SetInputData(stencilImage);
    stencilWriter->SetFileTypeToBinary();
    stencilWriter->SetWriteToOutputString(true);
    stencilWriter->Write();

    snapshot << SnapshotData{stencilFile(), QByteArray{stencilWriter->GetOutputString(), static_cast<int>(stencilWriter->GetOutputStringLength())}};
  }

  return snapshot;
}

//-----------------------------------------------------------------------------
void SplitFilter::restoreState(const State& state)
{
  if(!state.isEmpty())
  {
    double planeOrigin[3];
    double planeNormal[3];

    for (auto token : state.split(';'))
    {
      auto tokens = token.split('=');
      if (tokens.size() != 2) continue;

      if (PLANE_ORIGIN == tokens[0])
      {
        auto origin = tokens[1].split(",");
        if(origin.size() != 3) continue;

        for(int i = 0; i < 3; ++i)
        {
          planeOrigin[i] = origin[i].toDouble();
        }
      }

      if (PLANE_NORMAL == tokens[0])
      {
        auto normal = tokens[1].split(",");
        if(normal.size() != 3) continue;

        for(int i = 0; i < 3; ++i)
        {
          planeNormal[i] = normal[i].toDouble();
        }
      }
    }

    m_plane = vtkSmartPointer<vtkPlane>::New();
    m_plane->SetOrigin(planeOrigin[0], planeOrigin[1], planeOrigin[2]);
    m_plane->SetNormal(planeNormal[0], planeNormal[1], planeNormal[2]);
    m_plane->Modified();
  }
}

//-----------------------------------------------------------------------------
State SplitFilter::state() const
{
  State state;

  if(m_plane != nullptr)
  {
    NmVector3 origin{m_plane->GetOrigin()[0], m_plane->GetOrigin()[1], m_plane->GetOrigin()[2]};
    NmVector3 normal{m_plane->GetNormal()[0], m_plane->GetNormal()[1], m_plane->GetNormal()[2]};

    state += StatePair(PLANE_ORIGIN, origin);
    state += StatePair(PLANE_NORMAL, normal);
  }

  return state;
}

//-----------------------------------------------------------------------------
void SplitFilter::setStencilPlane(vtkSmartPointer<vtkPlane> plane)
{
  if(plane)
  {
    m_plane = vtkSmartPointer<vtkPlane>::New();
    m_plane->SetOrigin(plane->GetOrigin());
    m_plane->SetNormal(plane->GetNormal());
    m_plane->Modified();
  }
}
