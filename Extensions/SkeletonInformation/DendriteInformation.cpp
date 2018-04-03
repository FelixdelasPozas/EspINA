/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Analysis.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Extensions/SkeletonInformation/DendriteInformation.h>

// Qt
#include <QObject>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type EspinaExtensions_EXPORT DendriteSkeletonInformation::TYPE = "DendriteInformation";

const SegmentationExtension::Key DENDRITE_SHAFT_LENGTH             = QObject::tr("Shaft Length (Nm)");
const SegmentationExtension::Key DENDRITE_CONTACTED_AXONS_NUM      = QObject::tr("Num of axons contacted");
const SegmentationExtension::Key DENDRITE_EXCITATORY_AXONS_NUM     = QObject::tr("Num of excitatory axons contacted");
const SegmentationExtension::Key DENDRITE_INHIBITORY_AXONS_NUM     = QObject::tr("Num of inhibitory axons contacted");
const SegmentationExtension::Key DENDRITE_SYNAPSES_RATIO           = QObject::tr("Excitatory/inhibitory synapses ratio");
const SegmentationExtension::Key DENDRITE_TOTAL_SPINES_NUM         = QObject::tr("Num of spines");
const SegmentationExtension::Key DENDRITE_COMPLETE_SPINES_NUM      = QObject::tr("Num of complete spines");
const SegmentationExtension::Key DENDRITE_TRUNCATED_SPINES_NUM     = QObject::tr("Num of truncated spines");
const SegmentationExtension::Key DENDRITE_MONOSYNAPTIC_SPINES      = QObject::tr("Num of monosynaptic spines");
const SegmentationExtension::Key DENDRITE_NONSYNAPTIC_SPINES       = QObject::tr("Num of non-synaptic spines");
const SegmentationExtension::Key DENDRITE_MULTISYNAPTIC_SPINES     = QObject::tr("Num of multisynaptic spines");
const SegmentationExtension::Key DENDRITE_BRANCHES_SPINES          = QObject::tr("Num of branched spines");
const SegmentationExtension::Key DENDRITE_SPINES_LD                = QObject::tr("Linear density of spines");
const SegmentationExtension::Key DENDRITE_SPINES_LENGTH            = QObject::tr("Total length of complete spines (Nm)");
const SegmentationExtension::Key DENDRITE_SPINES_LENGTH_MEAN       = QObject::tr("Mean spine length (Nm)");
const SegmentationExtension::Key DENDRITE_NEAREAST_NEIGHTBOUR_MEAN = QObject::tr("Mean nearest neighbour distance of spines (Nm)");
const SegmentationExtension::Key DENDRITE_SYNAPSES_NUM             = QObject::tr("Num of synapses");
const SegmentationExtension::Key DENDRITE_SYNAPSES_ON_SPINES       = QObject::tr("Num of synapses on spines");
const SegmentationExtension::Key DENDRITE_SYNAPSES_ON_SHAFT        = QObject::tr("Num of synapses on shaft");
const SegmentationExtension::Key DENDRITE_SYNAPSES_LOCATION_RATIO  = QObject::tr("Ratio of synapses on spines and shafts");
const SegmentationExtension::Key DENDRITE_SYNAPSES_LD              = QObject::tr("Linear density of synapses");
const SegmentationExtension::Key DENDRITE_SYNAPSES_SHAFT_LD        = QObject::tr("Linear density of synapses on shaft");
const SegmentationExtension::Key DENDRITE_SYNAPSES_SPINE_LD        = QObject::tr("Linear density of synapses on spines");
const SegmentationExtension::Key DENDRITE_SYNAPSES_PER_SPINE_MEAN  = QObject::tr("Mean Num of Synapses per spine");

//--------------------------------------------------------------------
DendriteSkeletonInformation::DendriteSkeletonInformation(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
{
}

//--------------------------------------------------------------------
DendriteSkeletonInformation::~DendriteSkeletonInformation()
{
}

//--------------------------------------------------------------------
State DendriteSkeletonInformation::state() const
{
  return State();
}

//--------------------------------------------------------------------
Snapshot DendriteSkeletonInformation::snapshot() const
{
  return Snapshot();
}

//--------------------------------------------------------------------
SegmentationExtension::InformationKeyList DendriteSkeletonInformation::availableInformation() const
{
  InformationKeyList keys;

  for (auto value : {DENDRITE_SHAFT_LENGTH,
                     DENDRITE_CONTACTED_AXONS_NUM,
                     DENDRITE_EXCITATORY_AXONS_NUM,
                     DENDRITE_INHIBITORY_AXONS_NUM,
                     DENDRITE_SYNAPSES_RATIO,
                     DENDRITE_TOTAL_SPINES_NUM,
                     DENDRITE_COMPLETE_SPINES_NUM,
                     DENDRITE_TRUNCATED_SPINES_NUM,
                     DENDRITE_MONOSYNAPTIC_SPINES,
                     DENDRITE_NONSYNAPTIC_SPINES,
                     DENDRITE_MULTISYNAPTIC_SPINES,
                     DENDRITE_BRANCHES_SPINES,
                     DENDRITE_SPINES_LD,
                     DENDRITE_SPINES_LENGTH,
                     DENDRITE_SPINES_LENGTH_MEAN,
                     DENDRITE_NEAREAST_NEIGHTBOUR_MEAN,
                     DENDRITE_SYNAPSES_NUM,
                     DENDRITE_SYNAPSES_ON_SPINES,
                     DENDRITE_SYNAPSES_ON_SHAFT,
                     DENDRITE_SYNAPSES_LOCATION_RATIO,
                     DENDRITE_SYNAPSES_LD,
                     DENDRITE_SYNAPSES_SHAFT_LD,
                     DENDRITE_SYNAPSES_SPINE_LD,
                     DENDRITE_SYNAPSES_PER_SPINE_MEAN})
  {
    keys << createKey(value);
  }

  keys << m_keys; // add custom keys.

  return keys;
}

//--------------------------------------------------------------------
QVariant DendriteSkeletonInformation::cacheFail(const InformationKey& key) const
{
  QVariant info;

  if(m_extendedItem && hasSkeletonData(m_extendedItem->output()) && m_extendedItem->category()->classificationName().startsWith("Dendrite"))
  {
    updateInformation();

    if (availableInformation().contains(key))
    {
      info = information(key);
    }
  }

  return info;
}

//--------------------------------------------------------------------
void DendriteSkeletonInformation::updateInformation() const
{
  QWriteLocker lock(&m_mutex);

  Q_ASSERT(hasSkeletonData(m_extendedItem->output()));

  if(m_extendedItem && readLockSkeleton(m_extendedItem->output())->isValid())
  {
    auto skeleton    = readLockSkeleton(m_extendedItem->output())->skeleton();
    auto definition  = toSkeletonDefinition(skeleton);
    auto nodes       = definition.nodes;
    auto edges       = definition.edges;
    auto strokes     = definition.strokes;
    auto pathList    = paths(nodes, edges, strokes);
    auto hierarchy   = pathHierarchy(pathList, edges, strokes);
    auto connections = m_extendedItem->analysis()->connections(m_extendedItem);

    QList<NmVector3> conPoints;
    for(auto connection: connections) conPoints << connection.point;

    // finds the path node in the hierarchy of the given path in the given node list or any child.
    std::function<Core::PathHierarchyNode*(const Core::Path &, const QList<Core::PathHierarchyNode *>)> locateHNode = [&locateHNode](const Core::Path &path, const QList<Core::PathHierarchyNode *> nodes)
    {
      PathHierarchyNode *result = nullptr;

      for(auto node: nodes)
      {
        if(node->path == path) return node;

        auto child = locateHNode(path, node->children);

        if(child) return child;
      }

      return result;
    };

    // returns true if the given node or any children is truncated.
    std::function<bool(const PathHierarchyNode *)> isTruncated = [&isTruncated](const PathHierarchyNode *node)
    {
      if(node->path.begin->isTerminal() && node->path.begin->flags.testFlag(SkeletonNodeFlags::enum_type::TRUNCATED))
        return true;

      if(node->path.end->isTerminal() && node->path.end->flags.testFlag(SkeletonNodeFlags::enum_type::TRUNCATED))
        return true;

      for(auto child: node->children)
      {
        if(isTruncated(child)) return true;
      }

      return false;
    };

    // returns the total length of the given node and children, to be called from spine paths.
    std::function<double(const PathHierarchyNode *)> childLength = [&childLength](const PathHierarchyNode *node)
    {
      double result = 0;
      if(node->path.note.startsWith("Subspine", Qt::CaseInsensitive))
      {
        result += node->path.length();
      }

      for(auto child: node->children)
      {
        result += childLength(child);
      }

      return result;
    };

    // returns the number of connections of the given node and children.
    std::function<int(const PathHierarchyNode *)> connectionsNum = [&connectionsNum, &conPoints](const PathHierarchyNode *node)
    {
      int result = 0;

      if(node->path.begin->isTerminal() && conPoints.contains(NmVector3{node->path.begin->position}))
        ++result;

      if(node->path.end->isTerminal() && conPoints.contains(NmVector3{node->path.end->position}))
        ++result;

      for(auto child: node->children)
      {
        result += connectionsNum(child);
      }

      return result;
    };

    // follows the path to the end in the given direction, needs 2 nodes in seen list.
    auto followPath = [&edges, &strokes](Core::Path &path)
    {
      Q_ASSERT(path.seen.size() == 2);

      auto edgeIndex = path.seen.first()->connections[path.seen.last()];
      bool finished = false;
      while(!finished)
      {
        auto node = path.seen.last();
        auto size = path.seen.size();

        for(auto connection: node->connections.keys())
        {
          if(path.seen.contains(connection)) continue;
          auto otherEdgeIndex = node->connections[connection];

          if(otherEdgeIndex != edgeIndex)
          {
            auto stroke = strokes.at(edges.at(otherEdgeIndex).strokeIndex);
            if(stroke.name.startsWith("Spine", Qt::CaseInsensitive))
            {
              finished = true;
            }
          }

          path.seen.push_back(connection);
          break;
        }

        if(size == path.seen.size() && !finished) return 0.0; // we added nothing to the path so it's an ending.
      }

      return path.length();
    };

    double shaftLength = 0;
    double completeSpinesLength = 0;
    unsigned int spinesNum = 0;
    unsigned int truncated = 0;
    unsigned int branched = 0;
    unsigned int none = 0;
    unsigned int mono = 0;
    unsigned int multi = 0;
    unsigned int connectionsOnSpine = 0;
    QList<double> spineDistances;

    for(auto path: pathList)
    {
      if(path.note.startsWith("Shaft", Qt::CaseInsensitive))
      {
        shaftLength += path.length();
        continue;
      }

      if(path.note.startsWith("Spine", Qt::CaseInsensitive))
      {
        ++spinesNum;

        auto node = locateHNode(path, hierarchy);
        auto spineLength = childLength(node);

        if(isTruncated(node))
        {
          ++truncated;
        }
        else
        {
          completeSpinesLength += path.length() + spineLength;
        }

        if(spineLength != 0)
        {
          ++branched;
        }

        auto numberOfConnections = connectionsNum(node);
        switch(numberOfConnections)
        {
          case 0:
            ++none;
            break;
          case 1:
            ++mono;
            break;
          default:
            ++multi;
            break;
        }

        connectionsOnSpine += numberOfConnections;

        auto distanceToNext = std::numeric_limits<double>::max();
        if(node->parent && node->parent->path.note.startsWith("Shaft", Qt::CaseInsensitive))
        {
          auto connectionNode = node->parent->path.seen.contains(node->path.begin) ? node->path.begin : node->path.end;
          auto shaftEdge      = node->parent->path.edge;

          for(auto otherNode: connectionNode->connections.keys())
          {
            auto otherEdge = connectionNode->connections[otherNode];
            if(otherEdge != shaftEdge && otherEdge != node->path.edge)
            {
              auto otherEdge = edges.at(connectionNode->connections[otherNode]);
              auto otherStroke = strokes.at(otherEdge.strokeIndex);
              if(otherStroke.name.startsWith("Spine", Qt::CaseInsensitive))
              {
                distanceToNext = 0; // there is another spine at the very beginning.
                break;
              }
            }
            else
            {
              if(otherEdge == shaftEdge)
              {
                Core::Path neighbourPath;
                neighbourPath.seen << connectionNode << otherNode;
                auto distance = followPath(neighbourPath);
                if(distance > 0) distanceToNext = std::min(distance, distanceToNext);
              }
            }
          }
        }

        if(distanceToNext != std::numeric_limits<double>::max())
        {
          spineDistances << distanceToNext;
        }

        continue;
      }
    }

    updateInfoCache(DENDRITE_SHAFT_LENGTH, shaftLength);
    updateInfoCache(DENDRITE_TOTAL_SPINES_NUM, spinesNum);
    updateInfoCache(DENDRITE_COMPLETE_SPINES_NUM, spinesNum - truncated);
    updateInfoCache(DENDRITE_TRUNCATED_SPINES_NUM, truncated);

    updateInfoCache(DENDRITE_MONOSYNAPTIC_SPINES, mono);
    updateInfoCache(DENDRITE_MULTISYNAPTIC_SPINES, multi);
    updateInfoCache(DENDRITE_NONSYNAPTIC_SPINES, none);
    updateInfoCache(DENDRITE_BRANCHES_SPINES, branched);

    updateInfoCache(DENDRITE_SPINES_LD, static_cast<double>(spinesNum)/shaftLength);
    updateInfoCache(DENDRITE_SPINES_LENGTH, completeSpinesLength);
    updateInfoCache(DENDRITE_SPINES_LENGTH_MEAN, completeSpinesLength/(spinesNum-truncated));

    auto toDouble = [](unsigned int number) { return static_cast<double>(number); };

    auto totalConnections = connections.size();
    updateInfoCache(DENDRITE_SYNAPSES_NUM, totalConnections);
    updateInfoCache(DENDRITE_SYNAPSES_ON_SPINES, connectionsOnSpine);
    updateInfoCache(DENDRITE_SYNAPSES_ON_SHAFT, totalConnections - connectionsOnSpine);
    updateInfoCache(DENDRITE_SYNAPSES_LOCATION_RATIO, toDouble(totalConnections-connectionsOnSpine)/connectionsOnSpine);
    updateInfoCache(DENDRITE_SYNAPSES_LD, toDouble(totalConnections)/shaftLength);
    updateInfoCache(DENDRITE_SYNAPSES_SHAFT_LD, toDouble(totalConnections-connectionsOnSpine)/shaftLength);
    updateInfoCache(DENDRITE_SYNAPSES_SPINE_LD, toDouble(connectionsOnSpine)/shaftLength);
    updateInfoCache(DENDRITE_SYNAPSES_PER_SPINE_MEAN, toDouble(connectionsOnSpine)/toDouble(spinesNum-truncated));

    if(!spineDistances.isEmpty())
    {
      double meanNearest = std::accumulate(spineDistances.begin(), spineDistances.end(), 0)/spineDistances.size();
      updateInfoCache(DENDRITE_NEAREAST_NEIGHTBOUR_MEAN, meanNearest);
    }
    else
    {
      updateInfoCache(DENDRITE_NEAREAST_NEIGHTBOUR_MEAN, tr("No neighbours"));
    }

    unsigned int symmetric = 0;
    for(auto connection: connections)
    {
      auto synapseItem = connection.segmentation2;
      auto synapseSeg  = std::dynamic_pointer_cast<Segmentation>(synapseItem);
      Q_ASSERT(synapseSeg);
      if(synapseSeg->category()->classificationName().startsWith("Synapse/Symmetric", Qt::CaseInsensitive))
      {
        ++symmetric;
      }
    }

    updateInfoCache(DENDRITE_CONTACTED_AXONS_NUM, totalConnections);
    updateInfoCache(DENDRITE_EXCITATORY_AXONS_NUM, totalConnections - symmetric);
    updateInfoCache(DENDRITE_INHIBITORY_AXONS_NUM, symmetric);
    updateInfoCache(DENDRITE_SYNAPSES_RATIO, toDouble(totalConnections-symmetric)/symmetric);
  }
}


