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
#include <vtkIntArray.h>

using namespace ESPINA;

//--------------------------------------------------------------------
Core::SkeletonNodes Core::toNodes(const vtkSmartPointer<vtkPolyData> skeleton)
{
  Q_ASSERT(skeleton != nullptr && skeleton->GetNumberOfPoints() != 0 && skeleton->GetNumberOfLines() != 0);

  auto points = skeleton->GetPoints();
  auto lines  = skeleton->GetLines();
  SkeletonNodes result;

  // Points in our skeleton will be added while traversing lines.
  lines->InitTraversal();
  vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
  {
    while (lines->GetNextCell(idList))
    {
      if (idList->GetNumberOfIds() != 2) continue;

      SkeletonNode *pointA = nullptr;
      SkeletonNode *pointB = nullptr;
      vtkIdType data[2];
      data[0] = idList->GetId(0);
      data[1] = idList->GetId(1);
      double dataCoords[2][3];
      points->GetPoint(data[0], dataCoords[0]);
      points->GetPoint(data[1], dataCoords[1]);

      // Find the points.
      for (auto i = 0; i < result.size(); ++i)
      {
        if (::memcmp(result[i]->position, dataCoords[0], 3 * sizeof(double)) == 0)
          pointA = result[i];

        if (::memcmp(result[i]->position, dataCoords[1], 3 * sizeof(double)) == 0)
          pointB = result[i];
      }

      if (pointA == nullptr)
      {
        pointA = new SkeletonNode{dataCoords[0]};
        result << pointA;
      }

      if (pointB == nullptr)
      {
        pointB = new SkeletonNode{dataCoords[1]};
        result << pointB;
      }

      if (!pointA->connections.contains(pointB))
      {
        pointA->connections << pointB;
      }

      if (!pointB->connections.contains(pointA))
      {
        pointB->connections << pointA;
      }
    }
  }

  int idNum = 0;
  for(auto node: result)
  {
    if(node->connections.size() != 2)
    {
      node->id = QString::number(++idNum);
    }
  }

  return result;
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Core::toPolyData(const SkeletonNodes nodes)
{
  QMap<SkeletonNode *, vtkIdType> locator;
  QMap<vtkIdType, QList<vtkIdType>> relationsLocator;

  auto points = vtkSmartPointer<vtkPoints>::New();
  auto lines  = vtkSmartPointer<vtkCellArray>::New();
  auto ids    = vtkSmartPointer<vtkIntArray>::New();
  ids->SetName("Connections");
  ids->SetNumberOfValues(nodes.size());

  for (auto node : nodes)
  {
    locator.insert(node, points->InsertNextPoint(node->position));
    ids->InsertValue(locator[node], node->connections.size());
  }

  for (auto node : nodes)
  {
    relationsLocator.insert(locator[node], QList<vtkIdType>());

    for (auto connectedNode : node->connections)
    {
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
ESPINA::Core::PathList Core::paths(const SkeletonNodes& skeleton)
{
  PathList result, stack;

  auto lessThan = [](const SkeletonNode *left, const SkeletonNode *right) { return left < right; };

  auto checkIfReported = [lessThan, skeleton] (const Path currentPath, const PathList result)
  {
    for(auto path: result)
    {
      if((path.begin == currentPath.end && path.end == currentPath.begin) ||
         (path.end == currentPath.end && path.begin == currentPath.begin))
      {
        if(path.seen.size() != currentPath.seen.size()) continue;

        auto seen1 = path.seen;
        auto seen2 = currentPath.seen;

        qSort(seen1.begin(), seen1.end(), lessThan);
        qSort(seen2.begin(), seen2.end(), lessThan);

        if(seen1 == seen2)
        {
          return true;
        }
      }
    }
    return false;
  };

  if(skeleton.isEmpty()) return result;

  if(skeleton.size() == 1)
  {
    struct Path path;
    path.begin = path.end = skeleton.first();
    path.seen << path.begin << path.begin;

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
    Q_ASSERT(node != first);

    struct Path path;
    path.begin = first;
    path.end = node;
    path.seen << first;

    stack << path;
  }

  QList<Core::SkeletonNode *> alreadyExpanded;

  int j = 0;
  while(!stack.isEmpty() && stack.size() < 100 && j < 1000)
  {
    ++j;

    auto currentPath = stack.takeFirst();
    bool finished = false;

    if(currentPath.begin == currentPath.end)
    {
      currentPath.seen << currentPath.end;
      currentPath.note = "Loop";
      if(!checkIfReported(currentPath, result))
      {
        result << currentPath;
      }
      continue;
    }

    while(!finished && stack.size() < 100 && j < 1000)
    {
      if(currentPath.begin == currentPath.end)
      {
        currentPath.note = "Loop";
        currentPath.seen << currentPath.end;

        if(!checkIfReported(currentPath, result))
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
            if(!checkIfReported(currentPath, result))
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
              if(!checkIfReported(currentPath, result))
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

            if(!checkIfReported(toResult, result))
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
                if(!checkIfReported(toStack, result))
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

  // for debugging purposes, check loops removed.
  for(auto path: result) qDebug() << path;

  return result;
}

//--------------------------------------------------------------------
QDebug Core::operator <<(QDebug stream, const struct Path& path)
{
  stream << "Path " << path.begin->id << "<->" << path.end->id << " - length: "
         << path.seen.size() << " nodes. Notes: " << (path.note.isEmpty() ? "None" : path.note);

  return stream;
}
