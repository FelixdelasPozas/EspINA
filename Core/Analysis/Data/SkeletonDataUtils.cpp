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
#include "SkeletonDataUtils.h"

// C++
#include <cmath>
#include <cstring>

// Qt
#include <QtGlobal>
#include <QMap>

// VTK
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <vtkPointData.h>
#include <vtkMath.h>
#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkType.h>
#include <vtkStringArray.h>

using namespace ESPINA;
using namespace ESPINA::Core;

//--------------------------------------------------------------------
Core::SkeletonNodes Core::toNodes(const vtkSmartPointer<vtkPolyData> skeleton)
{
  if(skeleton == nullptr || skeleton->GetNumberOfPoints() == 0 || skeleton->GetNumberOfLines() == 0) return SkeletonNodes();

  auto points = skeleton->GetPoints();
  auto lines  = skeleton->GetLines();
  auto labels = vtkStringArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("Annotations"));
  auto idList = vtkSmartPointer<vtkIdList>::New();

  SkeletonNodes nodes;
  // get positions and annotations.
  for(int i = 0; i < points->GetNumberOfPoints(); ++i)
  {
    SkeletonNode *node = new SkeletonNode{points->GetPoint(i)};
    node->id = QString(labels->GetValue(i).c_str());
    nodes << node;
  }

  // get connections
  lines->InitTraversal();
  while (lines->GetNextCell(idList))
  {
    if (idList->GetNumberOfIds() != 2) continue;

    vtkIdType data[2];
    data[0] = idList->GetId(0);
    data[1] = idList->GetId(1);

    nodes.at(data[0])->connections << nodes.at(data[1]);
    nodes.at(data[1])->connections << nodes.at(data[0]);
  }

  return nodes;
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Core::toPolyData(const SkeletonNodes &nodes)
{
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(nodes.size());
  auto lines  = vtkSmartPointer<vtkCellArray>::New();
  auto ids    = vtkSmartPointer<vtkStringArray>::New();
  ids->SetName("Annotations");
  ids->SetNumberOfValues(nodes.size());

  // positions and annotations.
  QMap<SkeletonNode *, vtkIdType> locator;
  for(vtkIdType i = 0; i < nodes.size(); ++i)
  {
    auto node = nodes.at(static_cast<int>(i));
    points->SetPoint(i, node->position);
    ids->SetValue(i, node->id.toStdString().c_str());
    locator.insert(node, i);
  }

  QMap<vtkIdType, QList<vtkIdType>> relationsLocator;

  // build locator to avoid changing the nodes data.
  for (auto node : nodes)
  {
    relationsLocator.insert(locator[node], QList<vtkIdType>());

    for (auto connectedNode : node->connections)
    {
      if(connectedNode == node) continue;
      relationsLocator[locator[node]] << locator[connectedNode];
    }
  }

  for (auto nodeId : relationsLocator.keys())
  {
    for (auto connectionId : relationsLocator[nodeId])
    {
      auto line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, nodeId);
      line->GetPointIds()->SetId(1, connectionId);
      lines->InsertNextCell(line);

      // remove duplicated lines
      relationsLocator[connectionId].removeAll(nodeId);
    }
  }

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);
  polyData->GetPointData()->AddArray(ids);

  return polyData;
}

//--------------------------------------------------------------------
void Core::annotateNodes(SkeletonNodes nodes)
{
  int number = 0;
  for(auto node: nodes)
  {
    node->id.clear();
    if(node->connections.size() != 2)
    {
      node->id = QString::number(++number);
    }
  }

  int loopNumber = 0;
  for(auto component: Core::connectedComponents(nodes))
  {
    // annotates loops
    for(auto path: Core::paths(component))
    {
      if(path.note == "Loop" && path.begin->id.isEmpty())
      {
        path.begin->id = QString("Loop %1").arg(++loopNumber);
      }
    }
  }
}

//--------------------------------------------------------------------
int Core::closestNode(const double position[3], const SkeletonNodes nodes)
{
  double distance = VTK_DOUBLE_MAX;
  int closestNode = VTK_INT_MAX;

  for(auto i = 0; i < nodes.size(); ++i)
  {
    auto nodeDistance = vtkMath::Distance2BetweenPoints(position, nodes[i]->position);
    if(distance > nodeDistance)
    {
      distance = nodeDistance;
      closestNode = i;
    }
  }

  return closestNode;
}

//--------------------------------------------------------------------
double Core::closestDistanceAndNode(const double position[3], const SkeletonNodes nodes, int& node_i, int& node_j, double worldPosition[3])
{
  node_i = node_j = VTK_INT_MAX;

  double *pos_i, *pos_j;
  double projection[3];
  double result = VTK_DOUBLE_MAX;
  unsigned int segmentNode1Index = VTK_INT_MAX;
  unsigned int segmentNode2Index = VTK_INT_MAX;

  // build temporary map to accelerate access to lines
  QMap<SkeletonNode *, unsigned int> locator;
  for(int i = 0; i < nodes.size(); ++i)
  {
    locator[nodes[i]] = i;
  }

  for (int i = 0; i < nodes.size(); i++)
  {
    pos_i = nodes[i]->position;

    auto connections = nodes[i]->connections;
    for(int j = 0; j < connections.size(); ++j)
    {
      pos_j = nodes[locator[connections[j]]]->position;

      double v[3]{pos_j[0]-pos_i[0], pos_j[1]-pos_i[1], pos_j[2]-pos_i[2]};
      double w[3]{position[0]-pos_i[0], position[1]-pos_i[1], position[2]-pos_i[2]};

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
        ::memcpy(projection, pos_i, 3*sizeof(double));
        segmentNode1Index = segmentNode2Index = i;
      }
      else
      {
        if(r >= 1)
        {
          ::memcpy(projection, pos_j, 3*sizeof(double));
          segmentNode1Index = segmentNode2Index = locator[connections[j]];
        }
        else
        {
          projection[0] = pos_i[0] + r*(pos_j[0] - pos_i[0]);
          projection[1] = pos_i[1] + r*(pos_j[1] - pos_i[1]);
          projection[2] = pos_i[2] + r*(pos_j[2] - pos_i[2]);
          segmentNode1Index = i;
          segmentNode2Index = locator[connections[j]];
        }
      }

      double pointDistance = vtkMath::Distance2BetweenPoints(projection, position);

      if(result > pointDistance)
      {
        node_i = segmentNode1Index;
        node_j = segmentNode2Index;
        ::memcpy(worldPosition, projection, 3*sizeof(double));
        result = pointDistance;
      }
    }
  }

  return ::sqrt(result);
}

//--------------------------------------------------------------------
bool Core::operator==(const SkeletonNode &left, const SkeletonNode &right)
{
  return ((::memcmp(left.position, right.position, 3*sizeof(double))) &&
          (left.connections == right.connections)                     &&
          (left.id == right.id));
}

//--------------------------------------------------------------------
bool Core::operator==(const Core::Path &left, const Core::Path &right)
{
  auto lessThan = [](const SkeletonNode *left, const SkeletonNode *right) { return left < right; };

  if(left.seen.size() != right.seen.size()) return false;

  if((left.begin == right.end && left.end == right.begin) || (left.end == right.end && left.begin == right.begin))
  {
    auto seen1 = left.seen;
    auto seen2 = right.seen;

    qSort(seen1.begin(), seen1.end(), lessThan);
    qSort(seen2.begin(), seen2.end(), lessThan);

    if(seen1 == seen2) return true;
  }

  return false;
}

//--------------------------------------------------------------------
ESPINA::Core::PathList Core::paths(const SkeletonNodes& skeleton)
{
  PathList result, stack;

  auto checkIfDuplicated = [] (const Path currentPath, const PathList result)
  {
    for(auto path: result)
    {
      if(path == currentPath) return true;
    }
    return false;
  };

  if(skeleton.isEmpty()) return result;

  if(skeleton.size() == 1)
  {
    struct Path path;
    path.begin = path.end = skeleton.first();
    path.seen << path.begin << path.begin;
    path.begin->id = "Isolated";

    result << path;
    return result;
  }

  SkeletonNode *first = nullptr;
  for(auto node: skeleton)
  {
    if(node->connections.size() != 2)
    {
      first = node;
      break;
    }
  }

  if(first == nullptr)
  {
    // its a closed loop
    struct Path path;
    path.begin = path.end = skeleton.first();
    path.seen = skeleton;
    path.seen << skeleton.first();
    path.note = "Loop";

    result << path;
    return result;
  }

  for(auto node: first->connections)
  {
    if(node == first) continue;

    struct Path path;
    path.begin = first;
    path.end = node;
    path.seen << first;

    stack << path;
  }

  QList<Core::SkeletonNode *> alreadyExpanded;

  while(!stack.isEmpty())
  {
    auto currentPath = stack.takeFirst();
    bool finished = false;

    if(currentPath.begin == currentPath.end)
    {
      currentPath.seen << currentPath.end;
      currentPath.note = "Loop";
      if(!checkIfDuplicated(currentPath, result))
      {
        result << currentPath;
      }
      continue;
    }

    while(!finished)
    {
      if(currentPath.begin == currentPath.end)
      {
        currentPath.note = "Loop";
        currentPath.seen << currentPath.end;

        if(!checkIfDuplicated(currentPath, result))
        {
          result << currentPath;
        }
        break;
      }

      switch(currentPath.end->connections.size())
      {
        case 1:
          {
            currentPath.seen << currentPath.end;
            if(!checkIfDuplicated(currentPath, result))
            {
              result << currentPath;
            }
            finished = true;
          }
          break;
        case 2:
          {
            auto node = currentPath.end->connections.at(0) == currentPath.seen.last() ? currentPath.end->connections.at(1) : currentPath.end->connections.at(0);
            currentPath.seen << currentPath.end;
            currentPath.end = node;

            if(currentPath.end == currentPath.begin)
            {
              currentPath.seen << currentPath.end;
              currentPath.note = "Loop";
              if(!checkIfDuplicated(currentPath, result))
              {
                result << currentPath;
              }
              finished = true;
            }
          }
          break;
        default:
          {
            Path toResult;
            toResult.begin = currentPath.begin;
            toResult.end   = currentPath.end;
            toResult.seen  = currentPath.seen;
            toResult.seen << currentPath.end;
            toResult.note  = (currentPath.begin == currentPath.end ? "Loop" : "");

            if(!checkIfDuplicated(toResult, result))
            {
              result << toResult;
            }

            if(!alreadyExpanded.contains(currentPath.end))
            {
              alreadyExpanded << currentPath.end;
              for(auto node: currentPath.end->connections)
              {
                if(node == currentPath.seen.last()) continue;

                Path toStack;
                toStack.begin = currentPath.end;
                toStack.seen << currentPath.end;
                toStack.end   = node;

                toStack.seen << node;
                if(!checkIfDuplicated(toStack, result))
                {
                  toStack.seen.takeLast();
                  stack << toStack;
                }
              }
            }

            finished = true;
          }
          break;
      }
    }
  }

  return result;
}

//--------------------------------------------------------------------
QDebug Core::operator <<(QDebug stream, const SkeletonNodes& skeleton)
{
  stream << "Skeleton:\n- Size:" << skeleton.size() << "nodes.";

  auto components = Core::connectedComponents(skeleton);

  stream << "\n- connected components:" << components.size();

  for(auto component: components)
  {
    auto pathList = Core::paths(component);

    stream << "\n- component" << components.indexOf(component) + 1 << "paths:" << pathList.size();
    for(auto path: Core::paths(component))
    {
      stream << "\n    path" << pathList.indexOf(path) +1 << path;
    }
  }
  QStringList nodeNames;
  for(auto node: skeleton)
  {
    if(!node->id.isEmpty())
    {
      nodeNames << node->id;
    }
  }

  stream << "\n- relevant nodes number:" << nodeNames.size() << nodeNames;

  return stream;
}

//--------------------------------------------------------------------
QDebug Core::operator <<(QDebug stream, const struct Path& path)
{
  stream << "Path " << path.begin->id << "<->" << path.end->id << " - length: "
         << path.seen.size() << " nodes. Notes: " << (path.note.isEmpty() ? "None" : path.note);

  return stream;
}

//--------------------------------------------------------------------
QList<Core::SkeletonNodes> Core::connectedComponents(const SkeletonNodes& skeleton)
{
  QSet<SkeletonNode *> visited;
  QSet<SkeletonNode *> visitedTotal;

  std::function<void(SkeletonNode*)> visit = [&visited, &visitedTotal, &visit] (SkeletonNode *visitor)
  {
    if(!visited.contains(visitor))
    {
      visited.insert(visitor);
      visitedTotal.insert(visitor);
      for(auto connection: visitor->connections)
      {
        visit(connection);
      }
    }
  };

  QList<SkeletonNodes> result;
  if(skeleton.isEmpty()) return result;

  bool visitedAll = false;
  auto node = skeleton.first();

  while(!visitedAll)
  {
    visit(node);

    result << visited.toList();
    visited.clear();

    if(visitedTotal.size() == skeleton.size())
    {
      visitedAll = true;
    }
    else
    {
      for(auto skeletonNode: skeleton)
      {
        if(!visitedTotal.contains(skeletonNode))
        {
          node = skeletonNode;
          break;
        }
      }
    }
  }

  return result;
}

//--------------------------------------------------------------------
QList<SkeletonNodes> Core::loops(const SkeletonNodes& skeleton)
{
  QList<SkeletonNodes> result;
  if(skeleton.isEmpty()) return result;

  SkeletonNodes visited;

  auto lessThan = [](const SkeletonNode *left, const SkeletonNode *right) { return left < right; };

  auto checkIfDuplicated = [lessThan](SkeletonNodes path, QList<SkeletonNodes> &paths)
  {
    qSort(path.begin(), path.end(), lessThan);

    for(auto item: paths)
    {
      qSort(item.begin(), item.end(), lessThan);

      if(item == path) return true;
    }
    return false;
  };

  std::function<void(SkeletonNode*, SkeletonNodes)> visit = [&visit, &result, checkIfDuplicated] (SkeletonNode *visitor, SkeletonNodes visited)
  {
    SkeletonNode *lastVisited = nullptr; // needed because edges are bidirectional in our graphs.
    if(!visited.contains(visitor))
    {
      if(!visited.isEmpty()) lastVisited = visited.last();
      visited << visitor;
      for(auto connection: visitor->connections)
      {
        if(connection == lastVisited) continue;
        visit(connection, visited);
      }
    }
    else
    {
      auto index = visited.indexOf(visitor);
      visited << visitor;

      auto path = visited.mid(index);
      if(!checkIfDuplicated(path, result))
      {
        result << path;
      }
    }
  };

  visit(skeleton.first(), visited);

  return result;
}
