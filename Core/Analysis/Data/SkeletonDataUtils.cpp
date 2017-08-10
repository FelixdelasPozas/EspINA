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
#include <QSet>

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

using namespace ESPINA;
using namespace ESPINA::Core;

//--------------------------------------------------------------------
Core::SkeletonDefinition Core::toSkeletonDefinition(const vtkSmartPointer<vtkPolyData> skeleton)
{
  if(skeleton == nullptr || skeleton->GetNumberOfPoints() == 0 || skeleton->GetNumberOfLines() == 0) return SkeletonDefinition();

  SkeletonDefinition result;

  // edges information
  auto edgeIndexes = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeIndexes"));
  auto edgeNumbers = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("EdgeNumbers"));
  Q_ASSERT(edgeIndexes && edgeNumbers && (edgeIndexes->GetNumberOfTuples() == edgeNumbers->GetNumberOfTuples()));

  for(int i = 0; i < edgeIndexes->GetNumberOfTuples(); ++i)
  {
    SkeletonEdge edge;
    edge.strokeIndex = edgeIndexes->GetValue(i);
    edge.strokeNumber = edgeNumbers->GetValue(i);

    result.edges << edge;
  }

  // strokes information.
  auto strokeNames  = vtkStringArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeName"));
  auto strokeColors = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeColor"));
  auto strokeTypes  = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeType"));
  auto strokeUses   = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("StrokeUse"));
  auto numbers      = vtkIntArray::SafeDownCast(skeleton->GetPointData()->GetAbstractArray("Numbers"));
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
    result.strokes << stroke;
  }

  // nodes information.
  auto points = skeleton->GetPoints();
  auto lines  = skeleton->GetLines();

  for(int i = 0; i < points->GetNumberOfPoints(); ++i)
  {
    SkeletonNode *node = new SkeletonNode{points->GetPoint(i)};

    result.nodes << node;
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

  return result;
}

//--------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Core::toPolyData(const SkeletonDefinition &skeleton)
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

  for(int i = 0; i < skeleton.edges.size(); ++i)
  {
    auto edge = skeleton.edges.at(i);

    edgeIndexes->SetValue(i, edge.strokeIndex);
    edgeNumbers->SetValue(i, edge.strokeNumber);
  }

  // save terminal points
  auto terminal = vtkSmartPointer<vtkDoubleArray>::New();
  terminal->SetNumberOfComponents(3);
  terminal->SetName("TerminalNodes");

  // saves nodes coords
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(skeleton.nodes.size());

  // save node information.
  QMap<SkeletonNode *, vtkIdType> locator;
  for(vtkIdType i = 0; i < skeleton.nodes.size(); ++i)
  {
    auto node = skeleton.nodes.at(static_cast<int>(i));
    points->SetPoint(i, node->position);

    locator.insert(node, i);
    if(node->connections.size() == 1) terminal->InsertNextTuple(node->position);
  }

  QMap<vtkIdType, QList<vtkIdType>> relationsLocator;
  auto cellIndexes = vtkSmartPointer<vtkIntArray>::New();
  cellIndexes->SetName("LineIndexes");
  cellIndexes->SetNumberOfComponents(1);

  // build locator to avoid changing the nodes data.
  for(auto node: skeleton.nodes)
  {
    relationsLocator.insert(locator[node], QList<vtkIdType>());

    for(auto connectedNode: node->connections.keys())
    {
      if(connectedNode == node) continue;
      relationsLocator[locator[node]] << locator[connectedNode];
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

  auto polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);
  polyData->GetPointData()->AddArray(strokeNames);
  polyData->GetPointData()->AddArray(strokeColors);
  polyData->GetPointData()->AddArray(strokeTypes);
  polyData->GetPointData()->AddArray(strokeUses);
  polyData->GetPointData()->AddArray(numbers);
  polyData->GetPointData()->AddArray(terminal);
  polyData->GetPointData()->AddArray(edgeIndexes);
  polyData->GetPointData()->AddArray(edgeNumbers);
  polyData->GetCellData()->AddArray(cellIndexes);

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
bool Core::SkeletonNode::operator==(const Core::SkeletonNode &other) const
{
  return ((::memcmp(position, other.position, 3*sizeof(double))) &&
          (connections == other.connections));
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
ESPINA::Core::PathList Core::paths(const SkeletonNodes& nodes, const SkeletonEdges &edges, const SkeletonStrokes &strokes)
{
  PathList result, stack;

  // remove loops to itself
  for(auto node: nodes)
  {
    if(node->connections.contains(node))
    {
      node->connections.remove(node);
    }
  }

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
      auto count = 0;
      for(auto connection: node->connections.keys())
      {
        if(edge == node->connections[connection]) ++count;
      }

      if(count == 1)
      {
        path.begin = node;
        path.end   = node;
        return;
      }
    }
  };

  auto buildPath = [](Path &path, QSet<SkeletonNode *> set, const int edge)
  {
    while(path.seen.size() != set.size())
    {
      auto oldEnd = path.end;

      for(auto connection: path.end->connections.keys())
      {
        if(path.end->connections[connection] == edge && (path.seen.isEmpty() || path.seen.last() != connection))
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

  for(auto group: pathNodes.values())
  {
    auto key = pathNodes.key(group);
    Path path;

    searchBegin(path, group, key);

    if(path.begin != nullptr)
    {
      path.note  = strokeName(edges.at(key), strokes);
    }
    else
    {
      path.begin = (*group.begin());
      path.end   = path.begin;
      path.note  = "Loop " + strokeName(edges.at(key), strokes);
    }

    buildPath(path, group, key);

    if(path.seen.size() != group.size())
    {
      path.note += "(Malformed)";
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
    auto pathList = Core::paths(skeleton.nodes, skeleton.edges, skeleton.strokes);

    stream << "\n- component" << components.indexOf(component) + 1 << "paths:" << pathList.size();
    for(auto path: pathList)
    {
      stream << "\n    path" << pathList.indexOf(path) +1 << path;
    }
  }

  QStringList nodeNames;
  int count = 0;
  for(auto node: skeleton.nodes)
  {
    if(node->connections.size() == 1)
    {
      ++count;
    }
  }

  stream << "\n- relevant nodes number:" << count;

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
      for(auto connection: visitor->connections.keys())
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

  auto maximum = [](QSet<int> set)
  {
    int result = -1;
    for(auto element: set)
    {
      if(result < element) result = element;
    }

    return result;
  };

  auto firstMissing = [maximum](QSet<int> set)
  {
    for(int i = 1; i <= maximum(set); ++i)
    {
      if(!set.contains(i)) return i;
    }

    return -1;
  };

  auto nextAfterMissing = [maximum](QSet<int> set, const int element)
  {
    if(element == -1) return element;

    for(int i = element + 1; i <= maximum(set); ++i)
    {
      if(set.contains(i)) return i;
    }

    return -1;
  };

  for(auto index: recount.keys())
  {
    auto missing = firstMissing(recount[index]);
    while(missing != -1)
    {
      const auto element = nextAfterMissing(recount[index], missing);

      for(auto &edge: skeleton.edges)
      {
        if(edge.strokeIndex == index && edge.strokeNumber == element)
        {
          edge.strokeNumber = missing;
          break;
        }
      }

      recount[index] << missing;
      recount[index].remove(element);

      missing = firstMissing(recount[index]);
    }
  }

  for(int i = 0; i < skeleton.strokes.size(); ++i)
  {
    auto stroke = skeleton.strokes.at(i);
    skeleton.count[stroke] = recount.keys().contains(i) ? recount[i].size() : 0;
  }
}

//--------------------------------------------------------------------
const QString ESPINA::Core::strokeName(const Core::SkeletonEdge& edge, const Core::SkeletonStrokes& strokes)
{
  return QString("%1 %2").arg(strokes.at(edge.strokeIndex).name).arg(edge.strokeNumber);
}