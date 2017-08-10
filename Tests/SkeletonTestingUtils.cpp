/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Data/SkeletonDataUtils.h>

// Testing
#include "SkeletonTestingUtils.h"

// VTK
#include <cassert>
#include <random>

using namespace ESPINA::Core;

vtkSmartPointer<vtkPolyData> ESPINA::Testing::createRandomTestSkeleton(int numberOfNodes)
{
  assert(numberOfNodes >= 2);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, 100);

  SkeletonDefinition skeleton;

  SkeletonStroke stroke;
  stroke.name = "Stroke";
  stroke.colorHue = 1;
  stroke.type = 0;
  stroke.useMeasure = true;

  skeleton.strokes << stroke;

  SkeletonEdge edge;
  edge.strokeIndex = 0;
  edge.strokeNumber = 1;

  skeleton.edges << edge;

  skeleton.count[stroke] = 1;

  for(int i = 0; i < numberOfNodes; ++i)
  {
    auto node = new SkeletonNode{dis(gen), dis(gen), dis(gen)};

    if(!skeleton.nodes.isEmpty())
    {
      node->connections.insert(skeleton.nodes.last(), 0);
      skeleton.nodes.last()->connections.insert(node, 0);
    }

    skeleton.nodes << node;
  }

  return toPolyData(skeleton);
}

vtkSmartPointer<vtkPolyData> ESPINA::Testing::createSimpleTestSkeleton()
{
  SkeletonDefinition skeleton;
  skeleton.nodes << new SkeletonNode{0,0,0};
  skeleton.nodes << new SkeletonNode{1,0,0};
  skeleton.nodes << new SkeletonNode{0,1,0};
  skeleton.nodes << new SkeletonNode{0,0,1};

  SkeletonStroke stroke;
  stroke.name = "Stroke";
  stroke.colorHue = 1;
  stroke.type = 0;
  stroke.useMeasure = true;

  skeleton.strokes << stroke;

  SkeletonEdge edge;
  edge.strokeIndex = 0;
  edge.strokeNumber = 1;

  skeleton.edges << edge;

  skeleton.count[stroke] = 1;

  for(int i = 0; i < skeleton.nodes.size(); ++i)
  {
    auto node = skeleton.nodes.at(i);

    if(i != skeleton.nodes.size() - 1)
    {
      node->connections.insert(skeleton.nodes.at(i+1), 0);
      skeleton.nodes.at(i+1)->connections.insert(node, 0);
    }
  }

  return toPolyData(skeleton);
}
