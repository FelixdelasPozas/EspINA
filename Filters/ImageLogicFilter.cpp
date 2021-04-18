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
#include "ImageLogicFilter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Utils/Bounds.h>
#include <Core/Utils/BinaryMask.hxx>

// ITK
#include <itkImageRegionConstIterator.h>

// C++
#include <algorithm>

using namespace ESPINA;
using namespace ESPINA::Core;

//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(InputSList inputs, Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
, m_operation{Operation::NOSIGN}
, m_hue      {0}
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::setOperation(Operation op)
{
  m_operation = op;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate(Output::Id oId) const
{
  return m_outputs.empty() || m_inputs.empty();
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::execute()
{
  execute(0);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::execute(Output::Id oId)
{
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() > 1);

  // NOTE: Updating this filter will result in invalidating previous outputs
  m_outputs.clear();

  switch (m_operation)
  {
    case Operation::ADDITION:
      if(hasVolumetricData(m_inputs[0]->output()))
      {
        volumetricAddition();
      }
      else
      {
        skeletonAddition();
      }
      break;
    case Operation::SUBTRACTION:
      if(hasVolumetricData(m_inputs[0]->output()))
      {
        volumetricSubtraction();
      }
      else
      {
        skeletonSubtraction();
      }
      break;
    case Operation::NOSIGN:
    default:
      Q_ASSERT(false);
      break;
  };

  reportProgress(100);
  if (!canExecute()) return;
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::volumetricAddition()
{
  auto firstVolume    = readLockVolume(m_inputs[0]->output());
  auto boundingBounds = firstVolume->bounds();
  auto spacing        = boundingBounds.spacing();

  reportProgress(0);
  if (!canExecute()) return;

  for(auto input: m_inputs)
  {
    auto inputVolume = readLockVolume(input->output());
    boundingBounds = boundingBox(inputVolume->bounds(), boundingBounds);
  }

  reportProgress(50);
  if (!canExecute()) return;

  auto volume = std::make_shared<SparseVolume<itkVolumeType>>(boundingBounds, spacing);

  for(auto input: m_inputs)
  {
    auto inputImage  = readLockVolume(input->output())->itkImage();
    auto region      = inputImage->GetLargestPossibleRegion();
    auto inputBounds = equivalentBounds<itkVolumeType>(inputImage, region);
    auto mask        = std::make_shared<BinaryMask<unsigned char>>(inputBounds, input->output()->spacing());
    mask->setForegroundValue(SEG_VOXEL_VALUE);

    BinaryMask<unsigned char>::region_iterator mit(mask.get(), inputBounds);
    itk::ImageRegionConstIterator<itkVolumeType> it(inputImage, region);

    mit.goToBegin();
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      if(it.Get() == SEG_VOXEL_VALUE)
        mit.Set();

      ++it;
      ++mit;
    }

    volume->draw(mask, mask->foregroundValue());
  }

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
  m_outputs[0]->setSpacing(spacing);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::skeletonAddition()
{
  auto data     = readLockSkeleton(m_inputs[0]->output());
  auto spacing  = m_inputs[0]->output()->spacing();
  auto skeleton = Core::toSkeletonDefinition(data->skeleton());

  auto additionOp = [&skeleton, this](const InputSPtr input)
  {
    auto otherSkeleton = Core::toSkeletonDefinition(readLockSkeleton(input->output())->skeleton());

    // add different nodes
    for(auto otherNode: otherSkeleton.nodes)
    {
      auto equalNodeOp = [otherNode](const SkeletonNode *node) { return std::memcmp(otherNode->position, node->position, 3* sizeof(double)) == 0; };
      auto it = std::find_if(skeleton.nodes.constBegin(), skeleton.nodes.constEnd(), equalNodeOp);

      if(it == skeleton.nodes.constEnd())
      {
        auto newNode = new SkeletonNode{otherNode->position};
        newNode->flags = otherNode->flags;

        skeleton.nodes << newNode;
      }
    }

    // add different strokes
    for(auto otherStroke: otherSkeleton.strokes)
    {
      auto equalStrokeOp = [&otherStroke](const SkeletonStroke &stroke) { return (stroke.name == otherStroke.name); };
      auto it = std::find_if(skeleton.strokes.constBegin(), skeleton.strokes.constEnd(), equalStrokeOp);

      if(it == skeleton.strokes.constEnd())
      {
        otherStroke.colorHue = m_hue;
        skeleton.strokes << otherStroke;
        skeleton.count.insert(otherStroke, 0);
      }
    }

    // add edges
    QMap<int, int> oldToNew;
    for(auto otherNode: otherSkeleton.nodes)
    {
      auto equalNodeOp = [otherNode](const SkeletonNode *node) { return std::memcmp(otherNode->position, node->position, 3* sizeof(double)) == 0; };
      auto it = std::find_if(skeleton.nodes.constBegin(), skeleton.nodes.constEnd(), equalNodeOp);
      Q_ASSERT(it != skeleton.nodes.constEnd()); // exists

      for(auto connectionNode: otherNode->connections.keys())
      {
        auto equalCNodeOp = [connectionNode](const SkeletonNode *node) { return std::memcmp(connectionNode->position, node->position, 3* sizeof(double)) == 0; };
        auto cIt = std::find_if(skeleton.nodes.constBegin(), skeleton.nodes.constEnd(), equalCNodeOp);
        Q_ASSERT(cIt != skeleton.nodes.constEnd()); // exists

        if((*it)->connections.keys().contains(*cIt)) continue;

        auto otherEdge = otherNode->connections[connectionNode];
        if(!oldToNew.keys().contains(otherEdge))
        {
          auto otherStroke = otherSkeleton.strokes.at(otherSkeleton.edges.at(otherEdge).strokeIndex);
          auto equalStrokeOp = [&otherStroke](const SkeletonStroke &stroke) { return (stroke.name == otherStroke.name); };
          auto sIt = std::find_if(skeleton.strokes.constBegin(), skeleton.strokes.constEnd(), equalStrokeOp);
          Q_ASSERT(sIt != skeleton.strokes.constEnd());

          SkeletonEdge edge;
          edge.strokeIndex = skeleton.strokes.indexOf(*sIt);
          edge.strokeNumber = ++skeleton.count[*sIt];
          edge.parentEdge = -1;

          skeleton.edges << edge;
          oldToNew.insert(otherEdge, skeleton.edges.indexOf(edge));
        }

        (*it)->connections.insert(*cIt, oldToNew[otherEdge]);
      }
    }

    // adjust edges parents now that all edges are in
    for(auto otherNode: otherSkeleton.nodes)
    {
      auto equalNodeOp = [otherNode](const SkeletonNode *node) { return std::memcmp(otherNode->position, node->position, 3* sizeof(double)) == 0; };
      auto it = std::find_if(skeleton.nodes.constBegin(), skeleton.nodes.constEnd(), equalNodeOp);
      Q_ASSERT(it != skeleton.nodes.constEnd()); // exists

      for(auto connectionNode: otherNode->connections.keys())
      {
        auto equalCNodeOp = [connectionNode](const SkeletonNode *node) { return std::memcmp(connectionNode->position, node->position, 3* sizeof(double)) == 0; };
        auto cIt = std::find_if(skeleton.nodes.constBegin(), skeleton.nodes.constEnd(), equalCNodeOp);
        Q_ASSERT(cIt != skeleton.nodes.constEnd()); // exists

        auto otherEdge = otherNode->connections[connectionNode];
        auto parent = otherSkeleton.edges.at(otherEdge).parentEdge;
        if(parent == -1) continue;

        parent = oldToNew.contains(parent) ? oldToNew[parent] : -1;
        skeleton.edges[oldToNew[otherEdge]].parentEdge = parent;
      }
    }

    otherSkeleton.clear();

    auto pos = this->m_inputs.indexOf(input);
    reportProgress((100/m_inputs.size())*pos);
  };

  // do the magic
  std::for_each(m_inputs.constBegin() + 1, m_inputs.constEnd(), additionOp);

  Core::cleanSkeletonStrokes(skeleton);
  Core::removeIsolatedNodes(skeleton.nodes);
  Core::mergeSamePositionNodes(skeleton.nodes);
  Core::adjustStrokeNumbers(skeleton);

  auto skeletonPolydata = Core::toPolyData(skeleton);

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  m_outputs[0]->setData(std::make_shared<RawSkeleton>(skeletonPolydata, spacing));
  m_outputs[0]->setSpacing(spacing);

  skeleton.clear();
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::volumetricSubtraction()
{
  auto firstVolume = readLockVolume(m_inputs[0]->output());
  auto firstBounds = firstVolume->bounds();
  auto spacing     = firstBounds.spacing();

  auto outputVolume = std::make_shared<SparseVolume<itkVolumeType>>(firstBounds, spacing);
  outputVolume->draw(firstVolume->itkImage());

  for(auto i = 1; i < m_inputs.size(); ++i)
  {
    auto secondBounds = readLockVolume(m_inputs[i]->output())->bounds();

    if(intersect(firstBounds, secondBounds, spacing))
    {
      auto intersectionBounds = intersection(firstBounds, secondBounds, spacing);
      auto inputImage         = readLockVolume(m_inputs[i]->output())->itkImage(intersectionBounds);
      auto region             = inputImage->GetLargestPossibleRegion();
      auto inputBounds        = equivalentBounds<itkVolumeType>(inputImage, region);
      auto mask               = std::make_shared<BinaryMask<unsigned char>>(inputBounds, m_inputs[i]->output()->spacing());
      mask->setForegroundValue(SEG_BG_VALUE);

      BinaryMask<unsigned char>::region_iterator mit(mask.get(), inputBounds);
      itk::ImageRegionConstIterator<itkVolumeType> it(inputImage, region);

      mit.goToBegin();
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        if(it.Get() == SEG_VOXEL_VALUE)
          mit.Set();

        ++it;
        ++mit;
      }

      outputVolume->draw(mask, mask->foregroundValue());
    }

    reportProgress((100/m_inputs.size())*i);
    if (!canExecute()) return;
  }

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  auto bounds = minimalBounds<itkVolumeType>(outputVolume->itkImage(), SEG_BG_VALUE);
  outputVolume->resize(bounds);

  m_outputs[0]->setData(outputVolume);
  m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
  m_outputs[0]->setSpacing(spacing);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::skeletonSubtraction()
{
  auto data     = readLockSkeleton(m_inputs[0]->output());
  auto spacing  = m_inputs[0]->output()->spacing();
  auto skeleton = Core::toSkeletonDefinition(data->skeleton());

  auto subtractionOp = [&skeleton, this](const InputSPtr input)
  {
    auto otherSkeleton = Core::toSkeletonDefinition(readLockSkeleton(input->output())->skeleton());

    // add different nodes
    for(auto otherNode: otherSkeleton.nodes)
    {
      auto equalNodeOp = [otherNode](const SkeletonNode *node) { return std::memcmp(otherNode->position, node->position, 3* sizeof(double)) == 0; };
      auto it = std::find_if(skeleton.nodes.constBegin(), skeleton.nodes.constEnd(), equalNodeOp);

      if(it != skeleton.nodes.constEnd())
      {
        skeleton.nodes.removeAll(*it);
        delete *it; // removes connections also
      }
    }
  };

  // do the magic
  std::for_each(m_inputs.constBegin() + 1, m_inputs.constEnd(), subtractionOp);

  Core::cleanSkeletonStrokes(skeleton);
  Core::removeIsolatedNodes(skeleton.nodes);
  Core::mergeSamePositionNodes(skeleton.nodes);
  Core::adjustStrokeNumbers(skeleton);

  auto skeletonPolydata = Core::toPolyData(skeleton);

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  m_outputs[0]->setData(std::make_shared<RawSkeleton>(skeletonPolydata, spacing));
  m_outputs[0]->setSpacing(spacing);

  skeleton.clear();
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::restoreState(const State& state)
{
  if(state.compare("Operation=ADDITION"))
  {
    m_operation = Operation::ADDITION;
  }
  else
  {
    if(state.compare("Operation=SUBTRACTION"))
    {
      m_operation = Operation::SUBTRACTION;
    }
    else
    {
      Q_ASSERT(false);
    }
  }
}

//-----------------------------------------------------------------------------
State ImageLogicFilter::state() const
{
  State state;

  if (m_operation == Operation::ADDITION)
  {
    state = State("Operation=ADDITION");
  }
  else
  {
    if (m_operation == Operation::SUBTRACTION)
    {
      state = State("Operation=SUBTRACTION");
    }
    else
    {
      Q_ASSERT(false);
    }
  }

  return state;
}

//-----------------------------------------------------------------------------
Snapshot ImageLogicFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate() const
{
  return m_outputs.empty();
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::ignoreStorageContent() const
{
  return false;
}
