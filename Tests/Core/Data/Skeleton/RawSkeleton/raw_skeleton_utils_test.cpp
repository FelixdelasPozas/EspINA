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
#include <Core/Analysis/Data/Skeleton/RawSkeleton.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>

// Testing
#include "SkeletonTestingUtils.h"

// C++
#include <memory>

using namespace ESPINA;
using namespace ESPINA::Core;

int raw_skeleton_utils_test(int argc, char** argv)
{
  bool error = false;

  auto skeleton = Testing::createRandomTestSkeleton(200);

  auto data1    = std::make_shared<RawSkeleton>(skeleton);
  auto storage  = std::make_shared<TemporalStorage>();

  for(auto snapshot: data1->snapshot(storage, "Skeleton", 0))
  {
    storage->saveSnapshot(snapshot);
  }

  auto data2 = std::make_shared<RawSkeleton>();
  data2->setFetchContext(storage, "Skeleton", 0, data1->bounds());
  data2->fetchData();

  auto definition = toSkeletonDefinition(data1->skeleton());
  auto diskDefinition = toSkeletonDefinition(data2->skeleton());

  auto skeletonPaths = paths(definition.nodes, definition.edges, definition.strokes);
  auto diskSkeletonPaths = paths(diskDefinition.nodes, diskDefinition.edges, diskDefinition.strokes);

  if(skeletonPaths.size() != diskSkeletonPaths.size())
  {
    std::cerr << "Different number of paths." << std::endl;
    error = true;
  }

  // NOTE: this comparison sometimes turns out to be false due to rounding, comparing to std::numeric_limits::epsilon doesn't help either.
  if(skeletonPaths.first().length() - diskSkeletonPaths.first().length() > 0.0001)
  {
    std::cerr << skeletonPaths.first().length() - diskSkeletonPaths.first().length() << std::endl;
    std::cerr << "Paths have different length. skeleton: " << skeletonPaths.first().length() << " other: " << diskSkeletonPaths.first().length() << std::endl;
    error = true;
  }

  if(skeletonPaths.first().seen.size() != diskSkeletonPaths.first().seen.size())
  {
    std::cerr << "Paths have different node sizes. skeleton: " << skeletonPaths.first().seen.size() << " other: " << diskSkeletonPaths.first().seen.size() << std::endl;
    error = true;
  }

  if(definition.strokes != diskDefinition.strokes)
  {
    std::cerr << "Strokes are different." << std::endl;
    error = true;
  }

  if(definition.edges != diskDefinition.edges)
  {
    std::cerr << "Edges are different." << std::endl;
    error = true;
  }

  if(definition.nodes.size() != diskDefinition.nodes.size())
  {
    std::cerr << "Different number of nodes." << std::endl;
    error = true;
  }

  for(int i = 0; i < definition.nodes.size(); ++i)
  {
    auto nodeIn = definition.nodes.at(i);
    auto nodeOut = diskDefinition.nodes.at(i);
    if(::memcmp(nodeIn->position, nodeOut->position, 3*sizeof(double)) != 0)
    {
      std::cerr << "Different nodes position." << std::endl;
      error = true;
      break;
    }

    if(nodeIn->connections.size() != nodeOut->connections.size())
    {
      std::cerr << "Different number of connections." << std::endl;
      error = true;
      break;
    }

    for(int j = 0; j < nodeIn->connections.size(); ++j)
    {
      auto connIn = nodeIn->connections.keys().at(j);

      bool found = false;
      for(auto k = 0; k < nodeOut->connections.size(); ++k)
      {
        auto connOut = nodeOut->connections.keys().at(k);
        found |= (::memcmp(connIn->position, connOut->position, 3*sizeof(double)) == 0);

        if(found)
        {
          if(nodeIn->connections[connIn] != nodeOut->connections[connOut])
          {
            std::cerr << "Different edge value in connection." << std::endl;
            error = true;
            break;
          }
        }
        if(found) break;
      }

      if(!found)
      {
        std::cerr << "NodeIn conn: " << nodeIn->connections.size() << " NodeOut conn: " << nodeOut->connections.size() << std::endl;
        auto pos = connIn->position;
        for(auto k = 0; k < nodeOut->connections.size(); ++k)
        {
          auto pos2 = nodeOut->connections.keys().at(k)->position;
          std::cerr << "Different connected nodes position: " << j << " In: " << i << " pos "<< pos[0] << " " << pos[1] << " " << pos[2] << " out " << pos2[0] << " " << pos2[1] << " " << pos2[2] << std::endl;
        }
        error = true;
        break;
      }
    }
  }

  // test loops
  definition.nodes.first()->connections.insert(definition.nodes.last(), 0);

  auto loopNodes = loops(definition.nodes);
  if(loopNodes.size() != 1)
  {
    std::cerr << "Unexpected number of loops: expected 1, got " << loopNodes.size() << std::endl;
    error = true;
    return error;
  }

  if(loopNodes.first().size() != definition.nodes.size() + 1)
  {
    std::cerr << "Unexpected number of loop nodes, expected " << definition.nodes.size() << " got " << loopNodes.first().size() << std::endl;
    error = true;
  }

  return error;
}

