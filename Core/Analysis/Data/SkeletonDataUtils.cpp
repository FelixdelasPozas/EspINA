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
#include <functional>

// Qt
#include <QtGlobal>
#include <QMap>
#include <QSet>
#include <QStack>
#include <QColor>

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
#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkMath.h>

using namespace ESPINA;
using namespace ESPINA::Core;

//--------------------------------------------------------------------
const Core::SkeletonDefinition Core::toSkeletonDefinition(const vtkSmartPointer<vtkPolyData> skeleton)
{
  if(skeleton == nullptr || skeleton->GetNumberOfPoints() == 0 || skeleton->GetNumberOfLines() == 0) return SkeletonDefinition();

  SkeletonDefinition result;

  // edges information
  auto edgeIndexes = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeIndexes"));
  auto edgeNumbers = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeNumbers"));
  auto edgeParents = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeParents"));
  Q_ASSERT(edgeIndexes && edgeNumbers && edgeParents);
  Q_ASSERT(edgeIndexes->GetNumberOfTuples() == edgeNumbers->GetNumberOfTuples());
  Q_ASSERT(edgeIndexes->GetNumberOfTuples() == edgeParents->GetNumberOfTuples());

  for(int i = 0; i < edgeIndexes->GetNumberOfTuples(); ++i)
  {
    result.edges << std::move(SkeletonEdge{edgeIndexes->GetValue(i), edgeNumbers->GetValue(i), edgeParents->GetValue(i)});
  }

  // strokes information.
  auto strokeNames  = vtkStringArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeName"));
  auto strokeColors = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeColor"));
  auto strokeTypes  = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeType"));
  auto strokeUses   = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeUse"));
  auto numbers      = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("Numbers"));
  auto flags        = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("Flags"));
  Q_ASSERT(strokeNames && strokeColors && strokeTypes && strokeUses && numbers);
  Q_ASSERT(strokeNames->GetNumberOfValues() == strokeColors->GetNumberOfTuples());
  Q_ASSERT(strokeNames->GetNumberOfValues() == strokeTypes->GetNumberOfTuples());
  Q_ASSERT(strokeNames->GetNumberOfValues() == strokeUses->GetNumberOfTuples());
  Q_ASSERT(strokeNames->GetNumberOfValues() == numbers->GetNumberOfTuples());

  for(int i = 0; i < strokeNames->GetNumberOfValues(); ++i)
  {
    SkeletonStroke stroke;
    stroke.name       = QString::fromLocal8Bit(strokeNames->GetValue(i).c_str());
    stroke.colorHue   = strokeColors->GetValue(i);
    stroke.type       = strokeTypes->GetValue(i);
    stroke.useMeasure = (strokeUses->GetValue(i) == 0 ? true : false);

    result.count.insert(stroke, numbers->GetValue(i));
    result.strokes << std::move(stroke);
  }

  // nodes information.
  auto points = skeleton->GetPoints();
  auto lines  = skeleton->GetLines();

  for(int i = 0; i < points->GetNumberOfPoints(); ++i)
  {
    SkeletonNode *node = new SkeletonNode{points->GetPoint(i)};

    if(flags)
    {
      node->flags = static_cast<SkeletonNodeFlags>(flags->GetValue(i));
    }

    result.nodes << std::move(node);
  }

  auto cellIndexes = vtkIntArray::SafeDownCast(skeleton->GetCellData()->GetAbstractArray("LineIndexes"));
  Q_ASSERT(cellIndexes);

  // get connections and stroke values.
  lines->InitTraversal();
  for(int i = 0; i < lines->GetNumberOfCells(); ++i)
  {
    auto idList = vtkSmartPointer<vtkIdList>::New();
    lines->GetNextCell(idList);
    auto edgeIndex = cellIndexes->GetValue(i);

    if (idList->GetNumberOfIds() != 2) continue;

    vtkIdType data[2];
    data[0] = idList->GetId(0);
    data[1] = idList->GetId(1);

    result.nodes.at(data[0])->connections.insert(result.nodes.at(data[1]), edgeIndex);
    result.nodes.at(data[1])->connections.insert(result.nodes.at(data[0]), edgeIndex);
  }

  mergeSamePositionNodes(result.nodes);
  removeIsolatedNodes(result.nodes);

  return result;
}

//--------------------------------------------------------------------
const vtkSmartPointer<vtkPolyData> Core::toPolyData(const SkeletonDefinition &skeleton)
{
  // saves stroke information.
  auto strokeNames = vtkSmartPointer<vtkStringArray>::New();
  strokeNames->SetName("StrokeName");
  strokeNames->SetNumberOfComponents(1);
  strokeNames->SetNumberOfValues(skeleton.strokes.size());

  // saves stroke hue values.
  auto strokeColors = vtkSmartPointer<vtkIntArray>::New();
  strokeColors->SetName("StrokeColor");
  strokeColors->SetNumberOfComponents(1);
  strokeColors->SetNumberOfValues(skeleton.strokes.size());

  // saves stroke type.
  auto strokeTypes = vtkSmartPointer<vtkIntArray>::New();
  strokeTypes->SetName("StrokeType");
  strokeTypes->SetNumberOfComponents(1);
  strokeTypes->SetNumberOfValues(skeleton.strokes.size());

  // saves stroke use boolean.
  auto strokeUses = vtkSmartPointer<vtkIntArray>::New();
  strokeUses->SetName("StrokeUse");
  strokeUses->SetNumberOfComponents(1);
  strokeUses->SetNumberOfValues(skeleton.strokes.size());

  auto numbers = vtkSmartPointer<vtkIntArray>::New();
  numbers->SetName("Numbers");
  numbers->SetNumberOfComponents(1);
  numbers->SetNumberOfValues(skeleton.strokes.size());

  for(int i = 0; i < skeleton.strokes.size(); ++i)
  {
    auto stroke = skeleton.strokes.at(i);

    strokeNames->SetValue(i, stroke.name.toStdString().c_str());
    strokeColors->SetValue(i, stroke.colorHue);
    strokeTypes->SetValue(i, stroke.type);
    strokeUses->SetValue(i, stroke.useMeasure == true ? 0 : 1);
    numbers->SetValue(i, skeleton.count[stroke]);
  }

  // save terminal points
  auto terminal = vtkSmartPointer<vtkDoubleArray>::New();
  terminal->SetNumberOfComponents(3);
  terminal->SetName("TerminalNodes");

  // saves nodes coords
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(skeleton.nodes.size());

  // saves nodes flags
  auto flags = vtkSmartPointer<vtkIntArray>::New();
  flags->SetName("Flags");
  flags->SetNumberOfComponents(1);
  flags->SetNumberOfValues(skeleton.nodes.size());

  // save node information.
  QMap<SkeletonNode *, vtkIdType> locator;
  for(vtkIdType i = 0; i < skeleton.nodes.size(); ++i)
  {
    auto node = skeleton.nodes.at(static_cast<int>(i));

    points->SetPoint(i, node->position);
    flags->SetValue(i, static_cast<int>(node->flags));

    locator.insert(node, i);
    if(node->isTerminal()) terminal->InsertNextTuple(node->position);
  }

  QMap<vtkIdType, QList<vtkIdType>> relationsLocator;
  auto cellIndexes = vtkSmartPointer<vtkIntArray>::New();
  cellIndexes->SetName("LineIndexes");
  cellIndexes->SetNumberOfComponents(1);

  // build locator to avoid changing the nodes data.
  QSet<int> truncatedEdges;
  for(auto node: skeleton.nodes)
  {
    relationsLocator.insert(locator[node], QList<vtkIdType>());
    auto truncated = node->flags.testFlag(SkeletonNodeProperty::TRUNCATED);

    for(auto connectedNode: node->connections.keys())
    {
      if(connectedNode == node) continue;
      relationsLocator[locator[node]] << locator[connectedNode];
      if(truncated) truncatedEdges << node->connections[connectedNode];
    }
  }

  // saves nodes connections.
  auto lines  = vtkSmartPointer<vtkCellArray>::New();
  for(auto nodeId: relationsLocator.keys())
  {
    for(auto connectionId: relationsLocator[nodeId])
    {
      auto line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, nodeId);
      line->GetPointIds()->SetId(1, connectionId);
      lines->InsertNextCell(line);

      cellIndexes->InsertNextValue(locator.key(nodeId)->connections.value(locator.key(connectionId)));

      // remove duplicated lines
      relationsLocator[connectionId].removeOne(nodeId);
    }
  }

  // save edge numbers
  auto edgeNumbers = vtkSmartPointer<vtkIntArray>::New();
  edgeNumbers->SetName("EdgeNumbers");
  edgeNumbers->SetNumberOfComponents(1);
  edgeNumbers->SetNumberOfValues(skeleton.edges.size());

  // save edge indexes
  auto edgeIndexes = vtkSmartPointer<vtkIntArray>::New();
  edgeIndexes->SetName("EdgeIndexes");
  edgeIndexes->SetNumberOfComponents(1);
  edgeIndexes->SetNumberOfValues(skeleton.edges.size());

  // save if edge is truncated, this is not restored in toSkeletonDefinition, it's only used in representation pipelines
  // to avoid computing it.
  auto edgeTruncated = vtkSmartPointer<vtkIntArray>::New();
  edgeTruncated->SetName("EdgeTruncated");
  edgeTruncated->SetNumberOfComponents(1);
  edgeTruncated->SetNumberOfValues(skeleton.edges.size());

  // save edge parents
  auto edgeParents = vtkSmartPointer<vtkIntArray>::New();
  edgeParents->SetName("EdgeParents");
  edgeParents->SetNumberOfComponents(1);
  edgeParents->SetNumberOfValues(skeleton.edges.size());

  for(int i = 0; i < skeleton.edges.size(); ++i)
  {
    auto edge = skeleton.edges.at(i);

    edgeIndexes->SetValue(i, edge.strokeIndex);
    edgeNumbers->SetValue(i, edge.strokeNumber);
    edgeParents->SetValue(i, edge.parentEdge);
    edgeTruncated->SetValue(i, truncatedEdges.contains(i) ? 1 : 0);
  }

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);
  polyData->GetPointData()->AddArray(strokeNames);
  polyData->GetPointData()->AddArray(strokeColors);
  polyData->GetPointData()->AddArray(strokeTypes);
  polyData->GetPointData()->AddArray(strokeUses);
  polyData->GetPointData()->AddArray(numbers);
  polyData->GetPointData()->AddArray(terminal);
  polyData->GetPointData()->AddArray(flags);
  polyData->GetPointData()->AddArray(edgeIndexes);
  polyData->GetPointData()->AddArray(edgeNumbers);
  polyData->GetPointData()->AddArray(edgeParents);
  polyData->GetPointData()->AddArray(edgeTruncated);
  polyData->GetCellData()->AddArray(cellIndexes);

  return polyData;
}

//--------------------------------------------------------------------
const int Core::closestNode(const double position[3], const SkeletonNodes nodes)
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
const double Core::closestDistanceAndNode(const double position[3], const SkeletonNodes nodes, int& node_i, int& node_j, double worldPosition[3])
{
  node_i = node_j = VTK_INT_MAX;

  double *pos_i, *pos_j;
  double projection[3];
  double result = VTK_DOUBLE_MAX;
  int segmentNode1Index = VTK_INT_MAX;
  int segmentNode2Index = VTK_INT_MAX;

  // build temporary map to accelerate access to lines
  QMap<SkeletonNode *, unsigned int> locator;
  for(int i = 0; i < nodes.size(); ++i)
  {
    locator[nodes[i]] = i;
  }

  for (int i = 0; i < nodes.size(); i++)
  {
    pos_i = nodes[i]->position;

    auto connections = nodes[i]->connections.keys();
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
        std::memcpy(projection, pos_i, 3*sizeof(double));
        segmentNode1Index = segmentNode2Index = i;
      }
      else
      {
        if(r >= 1)
        {
          std::memcpy(projection, pos_j, 3*sizeof(double));
          segmentNode1Index = segmentNode2Index = locator[connections[j]];
        }
        else
        {
          projection[0] = pos_i[0] + r*(pos_j[0] - pos_i[0]);
          projection[1] = pos_i[1] + r*(pos_j[1] - pos_i[1]);
          projection[2] = pos_i[2] + r*(pos_j[2] - pos_i[2]);

          if(vtkMath::Distance2BetweenPoints(projection, pos_i) < vtkMath::Distance2BetweenPoints(projection, pos_j))
          {
            segmentNode1Index = i;
            segmentNode2Index = locator[connections[j]];
          }
          else
          {
            segmentNode1Index = locator[connections[j]];
            segmentNode2Index = i;
          }
        }
      }

      double pointDistance = vtkMath::Distance2BetweenPoints(projection, position);

      if(result > pointDistance)
      {
        node_i = segmentNode1Index;
        node_j = segmentNode2Index;
        std::memcpy(worldPosition, projection, 3*sizeof(double));
        result = pointDistance;
      }
    }
  }

  return std::sqrt(result);
}

//--------------------------------------------------------------------
const double Core::closestPointToSegment(const double position[3], const SkeletonNode *node_i, const SkeletonNode *node_j, double closestPoint[3])
{
  double result = -1.;

  auto pos_i = node_i->position;
  auto pos_j = node_j->position;

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
    std::memcpy(closestPoint, pos_i, 3*sizeof(double));
  }
  else
  {
    if(r >= 1)
    {
      std::memcpy(closestPoint, pos_j, 3*sizeof(double));
    }
    else
    {
      closestPoint[0] = pos_i[0] + r*(pos_j[0] - pos_i[0]);
      closestPoint[1] = pos_i[1] + r*(pos_j[1] - pos_i[1]);
      closestPoint[2] = pos_i[2] + r*(pos_j[2] - pos_i[2]);
    }
  }

  result = vtkMath::Distance2BetweenPoints(closestPoint, position);

  return std::sqrt(result);
}

//--------------------------------------------------------------------
bool Core::SkeletonNode::operator==(const Core::SkeletonNode &other) const
{
  return ((std::memcmp(position, other.position, 3*sizeof(double))) &&
          (connections == other.connections)                        &&
          (flags == other.flags));
}

//--------------------------------------------------------------------
bool Core::Path::connectsTo(const Path &path) const
{
  return path.seen.contains(end) || path.seen.contains(begin);
}

//--------------------------------------------------------------------
bool Core::Path::hasEndingPoint(const NmVector3 &point) const
{
  if(end->isTerminal() && NmVector3{end->position} == point) return true;
  if(begin->isTerminal() && NmVector3{begin->position} == point) return true;

  return false;
}

//--------------------------------------------------------------------
bool Core::Path::operator==(const Core::Path &other) const
{
  auto lessThan = [](const SkeletonNode *left, const SkeletonNode *right) { return left < right; };

  if(seen.size() != other.seen.size()) return false;

  if((begin == other.end && end == other.begin) || (end == other.end && begin == other.begin))
  {
    auto seen1 = seen;
    auto seen2 = other.seen;

    qSort(seen1.begin(), seen1.end(), lessThan);
    qSort(seen2.begin(), seen2.end(), lessThan);

    if(seen1 == seen2) return true;
  }

  return false;
}

//--------------------------------------------------------------------
bool Core::Path::operator<(const Core::Path &other) const
{
  auto lessThan = [](const SkeletonNode *left, const SkeletonNode *right) { return left < right; };

  if(this->seen.size() != other.seen.size()) return this->seen.size() < other.seen.size();

  // arbitrary, based on pointers value
  auto seen1 = seen;
  auto seen2 = other.seen;

  qSort(seen1.begin(), seen1.end(), lessThan);
  qSort(seen2.begin(), seen2.end(), lessThan);

  for(int i = 0; i < seen1.size(); ++i)
  {
    if(seen1.at(i) < seen2.at(i)) return true;
  }

  return false;
}

//--------------------------------------------------------------------
const ESPINA::Core::PathList Core::paths(const SkeletonNodes& nodes, const SkeletonEdges &edges, const SkeletonStrokes &strokes)
{
  PathList result, stack;

  if(nodes.isEmpty()) return result;

  if(nodes.size() == 1)
  {
    struct Path path;
    path.begin = path.end = nodes.first();
    path.seen << path.begin << path.begin;
    path.note = QString("Isolated node");

    result << path;
    return result;
  }

  QMap<int, QSet<SkeletonNode *>> pathNodes;
  for(auto node: nodes)
  {
    for(auto value: node->connections.values())
    {
      pathNodes[value] << node;
    }
  }

  auto searchBegin = [](Path &path, const QSet<SkeletonNode *> set, const int edge)
  {
    for(auto node: set)
    {
      auto equalEdge = [edge, node](SkeletonNode *connection){ if(edge == node->connections[connection]) return true; return false; };
      const auto nodeConnections = node->connections.keys();
      auto count = std::count_if(nodeConnections.constBegin(), nodeConnections.constEnd(), equalEdge);

      if(count == 1)
      {
        path.begin = node;
        path.end   = node;
        return;
      }
    }
  };

  std::function<void(Path &, QSet<SkeletonNode *>, const int)> buildPath = [&buildPath](Path &path, QSet<SkeletonNode *> set, const int edge)
  {
    while(path.seen.size() != set.size())
    {
      auto oldEnd = path.end;
      const auto connections = path.end->connections.keys();
      const auto keys = path.end->connections.values();

      auto count = std::count(keys.constBegin(), keys.constEnd(), edge);

      // Invalid node, use the longest route. Will be reported as malformed.
      if(count > 2)
      {
        Path one, two;

        auto *search = &one;

        for(auto &other: connections)
        {
          if(path.seen.contains(other)) continue;

          if(search->seen.size() > 0) search = &two;
          search->begin = oldEnd;
          search->end = other;
          search->seen << oldEnd;

          buildPath(*search, set, edge);
        }

        if(one.seen.size() > two.seen.size()) search = &one;
        else                                  search = &two;

        path.end = search->end;
        path.seen << search->seen;

        return;
      }

      for(const auto &connection: connections)
      {
        if((path.end->connections[connection] == edge) && !path.seen.contains(connection))
        {
          path.seen << path.end;
          path.end = connection;
          break;
        }
      }

      if(path.end == oldEnd)
      {
        path.seen << path.end;
        return;
      }
    }
  };

  for(const auto group: pathNodes.values())
  {
    auto key = pathNodes.key(group);
    Path path;

    searchBegin(path, group, key);

    if(path.begin != nullptr)
    {
      path.note = strokeName(edges.at(key), strokes, edges);
    }
    else
    {
      path.begin = (*group.begin());
      path.end   = path.begin;
      path.note  = "Loop " + strokeName(edges.at(key), strokes, edges);
    }

    path.edge   = key;
    path.stroke = edges.at(key).strokeIndex;

    buildPath(path, group, key);

    if(path.seen.size() != group.size())
    {
      path.note += " (Malformed)";
    }

    if(path.begin->isTerminal() && !path.end->isTerminal())
    {
      std::reverse(path.seen.begin(), path.seen.end());
      std::swap(path.begin, path.end);
    }

    if(path.begin->flags.testFlag(SkeletonNodeProperty::TRUNCATED) ||
       path.end->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
    {
      path.note += " (Truncated)";
    }

    result << path;
  }

  return result;
}

//--------------------------------------------------------------------
QDebug Core::operator <<(QDebug stream, const SkeletonDefinition& skeleton)
{
  stream << "Skeleton:\n- Size:" << skeleton.nodes.size() << "nodes.";

  auto strokes = skeleton.strokes;

  stream << "\nNumber of strokes:" << strokes.size();

  for(int i = 0; i < strokes.size(); ++i)
  {
    auto stroke = strokes.at(i);
    stream << "\n\t" << stroke.name << ": color" << stroke.colorHue << ", type" << (stroke.type == 0 ? "solid" : "dashed") << ", use measure" << stroke.useMeasure;
  }

  auto components = Core::connectedComponents(skeleton.nodes);

  stream << "\n- connected components:" << components.size();

  for(auto component: components)
  {
    auto pathList = Core::paths(component, skeleton.edges, skeleton.strokes);

    stream << "\n- component" << components.indexOf(component) + 1 << "paths:" << pathList.size();
    for(auto path: pathList)
    {
      stream << "\n    path" << pathList.indexOf(path) +1 << path;
    }
  }

  QStringList nodeNames;
  int terminal  = 0;
  int truncated = 0;
  for(auto node: skeleton.nodes)
  {
    if(node->isTerminal()) ++terminal;
    if(node->flags.testFlag(SkeletonNodeProperty::TRUNCATED)) ++truncated;
  }

  stream << "\n- terminal nodes number:" << terminal;
  stream << "\n- truncated nodes number:" << truncated;

  return stream;
}

//--------------------------------------------------------------------
QDebug Core::operator <<(QDebug stream, const struct Path& path)
{
  if(path.begin->connections.empty())
  {
    stream << "Path is an isolated node";
  }
  else
  {
    auto edge = path.seen.at(0)->connections[path.seen.at(1)];
    stream << "Path stroke index:" << edge << ", path size:" << path.seen.size() << "Note:" << (path.note.isEmpty() ? "None" : path.note);
  }

  return stream;
}

//--------------------------------------------------------------------
const QList<Core::SkeletonNodes> Core::connectedComponents(const SkeletonNodes& nodes)
{
  QSet<SkeletonNode *> visited;
  QSet<SkeletonNode *> visitedTotal;

  std::function<void(SkeletonNode*)> visit = [&visited, &visitedTotal, &visit] (SkeletonNode *node)
  {
    if(!visited.contains(node))
    {
      visited.insert(node);
      visitedTotal.insert(node);
      for(auto connection: node->connections.keys())
      {
        visit(connection);
      }
    }
  };

  QList<SkeletonNodes> result;
  if(nodes.isEmpty()) return result;

  while(visitedTotal.size() != nodes.size())
  {
    for(auto skeletonNode: nodes)
    {
      if(!visitedTotal.contains(skeletonNode))
      {
        visit(skeletonNode);
        break;
      }
    }

    result << visited.toList();
    visited.clear();
  }

  return result;
}

//--------------------------------------------------------------------
const QList<SkeletonNodes> Core::loops(const SkeletonNodes& skeleton)
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
    if(!visited.contains(visitor))
    {
      SkeletonNode *lastVisited = nullptr; // needed because edges are bidirectional in our graphs.
      if(!visited.isEmpty()) lastVisited = visited.last();
      visited << visitor;
      for(auto connection: visitor->connections.keys())
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

//--------------------------------------------------------------------
void Core::registerSkeletonDataOperators()
{
  qRegisterMetaTypeStreamOperators<SkeletonStroke>("SkeletonStroke");
}

//--------------------------------------------------------------------
QDataStream& operator <<(QDataStream& out, const ESPINA::Core::SkeletonStroke& stroke)
{
  out << stroke.name << stroke.colorHue << stroke.type << stroke.useMeasure;

  return out;
}

//--------------------------------------------------------------------
QDataStream& operator >>(QDataStream& in, ESPINA::Core::SkeletonStroke& stroke)
{
  in >> stroke.name;
  in >> stroke.colorHue;
  in >> stroke.type;
  in >> stroke.useMeasure;

  return in;
}

//--------------------------------------------------------------------
void Core::adjustStrokeNumbers(Core::SkeletonDefinition& skeleton)
{
  QMap<int, QSet<int>> recount;

  for(auto node: skeleton.nodes)
  {
    for(auto connection: node->connections.values())
    {
      auto edge = skeleton.edges.at(connection);

      recount[edge.strokeIndex] << edge.strokeNumber;
    }
  }

  auto substitute = [&skeleton](const int strokeIndex, const int previous, const int value)
  {
    auto equalOp = [strokeIndex, previous](const SkeletonEdge &edge) { return (edge.strokeIndex == strokeIndex && edge.strokeNumber == previous); };
    auto it = std::find_if(skeleton.edges.begin(), skeleton.edges.end(), equalOp);

    Q_ASSERT(it != skeleton.edges.end());
    (*it).strokeNumber = value;
  };

  for(auto value: recount.keys())
  {
    auto valueList = recount[value].toList();
    qSort(valueList.begin(), valueList.end());

    // are there gaps? size is const, last value is not.
    Q_ASSERT(valueList.last() >= valueList.size());
    while(valueList.size() != valueList.last())
    {
      // identify and substitute gap.
      for(int i = 1; i < valueList.last(); ++i)
      {
        if(!valueList.contains(i))
        {
          substitute(value, valueList.takeLast(), i);
          valueList.insert(i-1, i);
        }
      }
    }
  }

  for(int i = 0; i < skeleton.strokes.size(); ++i)
  {
    auto stroke = skeleton.strokes.at(i);
    skeleton.count[stroke] = recount.keys().contains(i) ? recount[i].size() : 0;
  }
}

//--------------------------------------------------------------------
const QString ESPINA::Core::strokeName(const Core::SkeletonEdge &edge, const Core::SkeletonStrokes &strokes, const Core::SkeletonEdges &edges)
{
  auto number = QString::number(edge.strokeNumber);
  auto name   = strokes.at(edge.strokeIndex).name;
  auto parent = edge.parentEdge;
  QSet<int> visited;

  while((parent != -1) && !visited.contains(parent))
  {
    visited << parent;
    const auto &otherEdge = edges.at(parent);
    number = QString("%1.%2").arg(otherEdge.strokeNumber).arg(number);
    parent = otherEdge.parentEdge;
  }

  return QString("%1 %2").arg(name).arg(number);
}

//--------------------------------------------------------------------
const double ESPINA::Core::angle(SkeletonNode* base, SkeletonNode* a, SkeletonNode* b)
{
  double vector1[3]{a->position[0]-base->position[0], a->position[1]-base->position[1], a->position[2]-base->position[2]};
  double vector2[3]{b->position[0]-base->position[0], b->position[1]-base->position[1], b->position[2]-base->position[2]};

  double cross[3];
  vtkMath::Cross(vector1, vector2, cross);
  auto angle = std::atan2(vtkMath::Norm(cross), vtkMath::Dot(vector1, vector2));
  return vtkMath::DegreesFromRadians(angle);
}

//--------------------------------------------------------------------
QList<PathHierarchyNode*> ESPINA::Core::pathHierarchy(const PathList &paths, const Core::SkeletonEdges &edges, const Core::SkeletonStrokes &strokes)
{
  QList<PathHierarchyNode *> allNodes, pending, final;
  auto checkPriorities = !edges.isEmpty() && !strokes.isEmpty();

  auto assignTo = [&pending](PathHierarchyNode *node1, PathHierarchyNode *node2)
  {
    node1->parent = node2;
    node2->children << node1;

    pending.removeOne(node1);
  };

  for(auto path: paths)
  {
    auto node = new PathHierarchyNode(path);

    if((path.begin->isTerminal() && path.end->isTerminal()) || path.note.startsWith("Loop"))
    {
      final  << node;
    }
    else
    {
      pending << node;
    }

    allNodes << node;
  }

  while(!pending.isEmpty())
  {
    const auto pendingStrokes = pending.size();
    const auto stroke = pending.first();

    for (auto otherStroke : allNodes)
    {
      if (otherStroke->path == stroke->path) continue;

      if (stroke->path.connectsTo(otherStroke->path))
      {
        if (otherStroke->path.connectsTo(stroke->path) && !otherStroke->parent && checkPriorities)
        {
          auto strokeUse = strokes.at(edges.at(stroke->path.edge).strokeIndex).useMeasure;
          auto otherStrokeUse = strokes.at(edges.at(otherStroke->path.edge).strokeIndex).useMeasure;

          if (((strokeUse != otherStrokeUse) && strokeUse == false) || otherStroke->path.note.startsWith("Shaft", Qt::CaseInsensitive))
          {
            assignTo(stroke, otherStroke);
            break;
          }
        }
        else
        {
          if(otherStroke->parent != stroke)
          {
            assignTo(stroke, otherStroke);
            break;
          }
        }
      }
    }

    if(pendingStrokes == pending.size()) // have not progressed
    {
      final << stroke;
      pending.removeOne(stroke);
    }
  }

  // depending on the order we can have some orphans.
  for(auto node: allNodes)
  {
    if(!node->parent && !final.contains(node)) final << node;
  }

  Q_ASSERT(allNodes.size() == paths.size());

  return final;
}

//--------------------------------------------------------------------
ESPINA::Core::PathHierarchyNode * ESPINA::Core::locatePathHierarchyNode(const Path &path, const QList<PathHierarchyNode *> &hierarchy)
{
  PathHierarchyNode *result = nullptr;

  for(auto node: hierarchy)
  {
    if(node->path == path) return node;

    auto child = locatePathHierarchyNode(path, node->children);

    if(child) return child;
  }

  return result;
}

//--------------------------------------------------------------------
const bool ESPINA::Core::isTruncated(const PathHierarchyNode *node)
{
  if(node->path.begin->isTerminal() && node->path.begin->flags.testFlag(SkeletonNodeFlags::enum_type::TRUNCATED))
    return true;

  if(node->path.end->isTerminal() && node->path.end->flags.testFlag(SkeletonNodeFlags::enum_type::TRUNCATED))
    return true;

  auto result = std::any_of(node->children.constBegin(), node->children.constEnd(), [](const PathHierarchyNode *child) { return isTruncated(child); });

  return result;
}

//--------------------------------------------------------------------
const double ESPINA::Core::length(const PathHierarchyNode *node)
{
  double result = 0;

  if(node->path.note.startsWith("Subspine", Qt::CaseInsensitive) || node->path.note.startsWith("Spine", Qt::CaseInsensitive))
  {
    result += node->path.length();

    for(auto child: node->children)
    {
      result += length(child);
    }
  }

  return result;
};

//--------------------------------------------------------------------
const QList<NmVector3> ESPINA::Core::connectionsInNode(const PathHierarchyNode *node, const QList<NmVector3> &connectionPoints)
{
  QList<NmVector3> points;

  auto beginPoint = NmVector3{node->path.begin->position};
  auto endPoint   = NmVector3{node->path.end->position};

  if(node->path.begin->isTerminal() && connectionPoints.contains(beginPoint))
    points << beginPoint;

  if(node->path.end->isTerminal() && connectionPoints.contains(endPoint))
    points << endPoint;

  auto operation = [&connectionPoints, &points](const PathHierarchyNode *child) { points << connectionsInNode(child, connectionPoints); };
  std::for_each(node->children.constBegin(), node->children.constEnd(), operation);

  return points;
}

//--------------------------------------------------------------------
void ESPINA::Core::cleanSkeletonStrokes(SkeletonDefinition& skeleton)
{
  SkeletonDefinition cleanSkeleton;

  for(int i = 0; i < skeleton.strokes.size(); ++i)
  {
    auto &stroke = skeleton.strokes[i];
    for(int j = 0; j < skeleton.edges.size(); ++j)
    {
      auto &edge = skeleton.edges[j];
      if(i == edge.strokeIndex)
      {
        if(!cleanSkeleton.strokes.contains(stroke))
        {
          cleanSkeleton.strokes << stroke;
        }

        edge.strokeIndex = cleanSkeleton.strokes.indexOf(stroke);
        Q_ASSERT(edge.strokeIndex != -1);
      }
    }
  }

  for(int i = 0; i < skeleton.edges.size(); ++i)
  {
    auto &edge = skeleton.edges[i];
    if(edge.parentEdge == i)
    {
      edge.parentEdge = -1;
    }

    cleanSkeleton.edges << edge;
  }

  skeleton.strokes = cleanSkeleton.strokes;
  skeleton.edges   = cleanSkeleton.edges;

  // identify identical name strokes.
  QStringList duplicated;
  for(int i = 0; i < skeleton.strokes.size(); ++i)
  {
    const auto name = skeleton.strokes[i].name;
    auto count = std::count_if(skeleton.strokes.begin(), skeleton.strokes.end(), [name](const SkeletonStroke &other) { return name == other.name; });
    if(count > 1) duplicated << name;
  }

  while(!duplicated.isEmpty())
  {
    SkeletonStrokes toRemove;
    auto name = duplicated.takeFirst();

    auto it = std::find_if(skeleton.strokes.begin(), skeleton.strokes.end(), [&name](const SkeletonStroke &stroke) { return name == stroke.name; });
    auto position = skeleton.strokes.indexOf(*it);
    auto count    = skeleton.count[*it];

    while(it != skeleton.strokes.end())
    {
      it = std::find_if(it + 1, skeleton.strokes.end(), [&name](const SkeletonStroke &stroke) { return name == stroke.name; });
      if(it != skeleton.strokes.end())
      {
        toRemove << *it;
        auto otherPosition = skeleton.strokes.indexOf(*it);

        auto replaceEdges = [position, otherPosition, count](SkeletonEdge &edge)
        {
          if(edge.strokeIndex == otherPosition)
          {
            edge.strokeIndex = position;
            edge.strokeNumber += count;
          }
          else
          {
            if(edge.strokeIndex > otherPosition)
            {
              edge.strokeIndex -= 1;
            }
          }
        };

        std::for_each(skeleton.edges.begin(), skeleton.edges.end(), replaceEdges);
        count += skeleton.count[*it];
      }
    }

    if(!toRemove.isEmpty())
    {
      skeleton.count[skeleton.strokes.at(position)] = count;
      for(auto &stroke: toRemove)
      {
        skeleton.strokes.removeAll(stroke);
        skeleton.count.remove(stroke);
      }
    }
  }
}

//--------------------------------------------------------------------
void ESPINA::Core::removeIsolatedNodes(SkeletonNodes &nodes)
{
  SkeletonNodes toRemove;

  auto removeLoops = [&toRemove](SkeletonNode *node)
  {
    if(node->connections.keys().contains(node)) node->connections.remove(node);
    if(node->connections.isEmpty()) toRemove << node;
  };
  std::for_each(nodes.begin(), nodes.end(), removeLoops);

  auto removeNode = [&nodes](SkeletonNode *node)
  {
    nodes.removeAll(node);
    delete node;
  };
  std::for_each(toRemove.begin(), toRemove.end(), removeNode);
}

//--------------------------------------------------------------------
void ESPINA::Core::mergeSamePositionNodes(SkeletonNodes &nodes)
{
  auto nodesNum = nodes.size();
  SkeletonNodes toRemove;

  auto checkEqual = [](const double *one, const double *two)
  {
    bool result = true;
    for(int i = 0; i < 3; ++i)
    {
      result &= areEqual(one[i], two[i]);
    }

    return result;
  };

  for(int i = 0; i < nodesNum; ++i)
  {
    auto node = nodes.at(i);

    for(int j = i + 1; j < nodesNum; ++j)
    {
      auto otherNode = nodes.at(j);

      if((std::memcmp(node->position, otherNode->position, 3*sizeof(double)) == 0) || checkEqual(node->position, otherNode->position))
      {
        if(!toRemove.contains(otherNode)) toRemove << otherNode;

        for(auto key: otherNode->connections.keys())
        {
          if(key == otherNode) continue;

          if(key == node)
          {
            if(node->connections.keys().contains(otherNode)) node->connections.remove(otherNode);
          }
          else
          {
            if(!node->connections.contains(key)) node->connections.insert(key, otherNode->connections[key]);
            if(!key->connections.contains(node)) key->connections.insert(node, key->connections[otherNode]);
            if(key->connections.contains(otherNode)) key->connections.remove(otherNode);
          }
        }

        otherNode->connections.clear();
        if(node->connections.isEmpty() && !toRemove.contains(node)) toRemove << node;
      }
    }
  }

  auto removeNode = [&nodes](SkeletonNode *node)
  {
    nodes.removeAll(node);
    delete node;
  };
  std::for_each(toRemove.begin(), toRemove.end(), removeNode);
}

//--------------------------------------------------------------------
const QColor ESPINA::Core::alternateStrokeColor(const SkeletonStrokes& strokes, int index)
{
  if(index < 0 || index >= strokes.size())
  {
    return QColor();
  }

  const auto &stroke = strokes.at(index);
  auto finalHue = stroke.colorHue;

  int position = 0;
  QSet<int> hueValues;

  for(int i = 0; i < strokes.size(); ++i)
  {
    if(i == index) continue;

    const auto &otherStroke = strokes.at(i);

    hueValues << otherStroke.colorHue;

    // alphabetic to keep certain order, but can be altered by introducing more strokes.
    if((otherStroke.colorHue == stroke.colorHue) && (otherStroke.name < stroke.name)) ++position;
  }

  while((position > 0) && (position < 20) && hueValues.contains(finalHue))
  {
    finalHue = (stroke.colorHue + (50*position)) % 360;

    ++position;
  }

  return QColor::fromHsv(finalHue, 255, 255);
}
