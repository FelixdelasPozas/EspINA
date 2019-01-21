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
#include <Core/Utils/AnalysisUtils.h>
#include <Extensions/SkeletonInformation/DendriteInformation.h>

// Qt
#include <QObject>
#include <QFileInfo>
#include <QByteArray>

#include <numeric>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type EspinaExtensions_EXPORT DendriteSkeletonInformation::TYPE = "DendriteInformation";
const QString DendriteSkeletonInformation::SPINE_SNAPSHOT_FILE = "SpineInformation.csv";

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
State DendriteSkeletonInformation::state() const
{
  return State();
}

//--------------------------------------------------------------------
Snapshot DendriteSkeletonInformation::snapshot() const
{
  Snapshot result;

  if(!m_spines.isEmpty())
  {
    QByteArray buffer;

    const QChar SEPARATOR{','};
    const QChar NEWLINE  {'\n'};

    for(auto spine: m_spines)
    {
      buffer.append(spine.name);
      buffer.append(SEPARATOR);
      buffer.append(spine.parentName);
      buffer.append(SEPARATOR);
      buffer.append(spine.complete ? "true":"false");
      buffer.append(SEPARATOR);
      buffer.append(spine.branched ? "true":"false");
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.length));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numSynapses));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numAsymmetric));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numAsymmetricHead));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numAsymmetricNeck));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numSymmetric));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numSymmetricHead));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numSymmetricNeck));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numAxons));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numAxonsExcitatory));
      buffer.append(SEPARATOR);
      buffer.append(QString::number(spine.numAxonsInhibitory));
      buffer.append(NEWLINE);
    }

    result << SnapshotData(snapshotName(SPINE_SNAPSHOT_FILE), buffer);
  }

  return result;
}

//--------------------------------------------------------------------
const SegmentationExtension::InformationKeyList DendriteSkeletonInformation::availableInformation() const
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
void DendriteSkeletonInformation::updateSpineInformation(const SkeletonDefinition               &definition,
                                                         const Core::PathList                   &paths,
                                                         const QList<Core::PathHierarchyNode *> &hierarchy,
                                                         const Core::Connections                &connections) const
{
  m_spines.clear();

  int edgeStrokeIndex = -1;
  for(int i = 0; i < definition.strokes.size(); ++i)
  {
    if(definition.strokes.at(i).name == "Spine")
    {
      edgeStrokeIndex = i;
      break;
    }
  }

  if(edgeStrokeIndex == -1) return; // No spine stroke, nothing to compute.

  // Returns true if any of the given node has branches. Must be called with the children of a spine node.
  std::function<bool(QList<PathHierarchyNode *>)> hasBranches = [&hasBranches](QList<PathHierarchyNode *> nodes)
  {
    for(auto node: nodes)
    {
      auto name = node->path.note;
      if(name.startsWith("Spine", Qt::CaseInsensitive) || name.startsWith("Subspine", Qt::CaseInsensitive) || hasBranches(node->children))
      {
        return true;
      }
    }

    return false;
  };

  // Returns the connections number and the type in the given parameters of the connections of the given node.
  std::function<const Connections(const PathHierarchyNode *, int&, int&, int&)> connectionsNumber = [&connectionsNumber, &connections](const PathHierarchyNode *node, int &number, int &asymNumber, int &symNumber)
  {
    Connections result;
    auto beginPoint = NmVector3{node->path.begin->position};
    auto endPoint   = NmVector3{node->path.end->position};

    for(auto connection: connections)
    {
      if((node->path.begin->isTerminal() && beginPoint == connection.point) || (node->path.end->isTerminal() && endPoint == connection.point))
      {
        ++number;
        auto seg = std::dynamic_pointer_cast<Segmentation>(connection.segmentation2);
        if(seg->category()->name().startsWith("Asy", Qt::CaseInsensitive)) ++asymNumber;
        else                                                               ++symNumber;

        result << connection;
      }
    }

    for(const auto child: node->children)
    {
      result << connectionsNumber(child, number, asymNumber, symNumber);
    }

    return result;
  };

  // Classifies the connections in the passed parameters for the given node.
  std::function<void(const PathHierarchyNode *, const Connections, int&, int&)> classifyConnections = [&classifyConnections] (const PathHierarchyNode *node, const Connections &points, int &numANeck, int &numSNeck)
  {
    for(auto connection: points)
    {
      for(auto child: node->children)
      {
        if(child->path.hasEndingPoint(connection.point))
        {
          if(child->path.note.startsWith("Synapse on spine neck", Qt::CaseInsensitive))
          {
            auto seg = std::dynamic_pointer_cast<Segmentation>(connection.segmentation2);
            if(seg->category()->name().startsWith("Asy", Qt::CaseInsensitive)) ++numANeck;
            else                                                               ++numSNeck;
          }
          continue;
        }
      }
    }
  };


  QList<NmVector3> pointList;
  for(auto connection: connections)
  {
    pointList << connection.point;
  }

  for(auto &path: paths)
  {
    if(!path.note.startsWith("Spine")) continue;
    if(path.stroke != edgeStrokeIndex) continue;

    auto pathNode = locatePathHierarchyNode(path, hierarchy);
    Q_ASSERT(pathNode);

    SpineInformation info;
    info.name = path.note;
    info.parentName = m_extendedItem->alias().isEmpty() ? m_extendedItem->name() : m_extendedItem->alias();
    info.complete = !isTruncated(pathNode);
    info.branched = hasBranches(pathNode->children);
    info.length  = length(pathNode);
    auto points = connectionsInNode(pathNode, pointList);

    auto connected = connectionsNumber(pathNode, info.numSynapses, info.numAsymmetric, info.numSymmetric);
    Q_ASSERT((connected.size() == info.numSynapses) && (info.numSynapses == info.numAsymmetric + info.numSymmetric));

    classifyConnections(pathNode, connected, info.numAsymmetricNeck, info.numSymmetricNeck);
    info.numAsymmetricHead = info.numAsymmetric - info.numAsymmetricNeck;
    info.numSymmetricHead  = info.numSymmetric - info.numSymmetricNeck;

    for(auto connection: connected)
    {
      auto seg = std::dynamic_pointer_cast<Segmentation>(connection.segmentation2);
      auto axon = axonOf(seg.get());
      if(axon)
      {
        ++info.numAxons;
        if(seg->category()->name().startsWith("Asy", Qt::CaseInsensitive)) ++info.numAxonsExcitatory;
        else                                                               ++info.numAxonsInhibitory;
      }
    }

    m_spines << info;
  }

  auto lessThan = [](const struct SpineInformation &rhs, const struct SpineInformation &lhs) { return rhs.name < lhs.name; };
  qSort(m_spines.begin(), m_spines.end(), lessThan);
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

    updateSpineInformation(definition, pathList, hierarchy, connections);

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
        SkeletonNode *next = nullptr;

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
          else
          {
            Q_ASSERT(!next);
            next = connection;
          }
        }

        if(next)
        {
          path.seen.push_back(next);
        }

        if(size == path.seen.size() && !finished) return 0.0; // we added nothing to the path so it's an ending.
      }

      return path.length();
    };

    double shaftLength = 0;
    double completeSpinesLength = 0;
    unsigned int spinesNum = m_spines.size();
    unsigned int truncated = 0;
    unsigned int branched = 0;
    unsigned int none = 0;
    unsigned int mono = 0;
    unsigned int multi = 0;
    unsigned int connectionsOnSpine = 0;
    QList<double> spineDistances;

    for(auto spine: m_spines)
    {
      if(!spine.complete)
      {
        ++truncated;
      }
      else
      {
        completeSpinesLength += spine.length;
      }

      if(spine.branched) ++branched;

      switch(spine.numSynapses)
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

      connectionsOnSpine += spine.numSynapses;
    }

    for(auto path: pathList)
    {
      if(path.note.startsWith("Shaft", Qt::CaseInsensitive))
      {
        shaftLength += path.length();
        continue;
      }

      if(path.note.startsWith("Spine", Qt::CaseInsensitive))
      {
        auto distanceToNext = std::numeric_limits<double>::max();
        auto node = locatePathHierarchyNode(path, hierarchy);
        if(node->parent && node->parent->path.note.startsWith("Shaft", Qt::CaseInsensitive))
        {
          auto connectionNode = node->parent->path.seen.contains(node->path.begin) ? node->path.begin : node->path.end;
          auto shaftEdge      = node->parent->path.edge;

          for(auto otherNode: connectionNode->connections.keys())
          {
            auto otherEdge = connectionNode->connections[otherNode];
            if(otherEdge != shaftEdge && otherEdge != node->path.edge)
            {
              auto sEdge   = edges.at(connectionNode->connections[otherNode]);
              auto sStroke = strokes.at(sEdge.strokeIndex);
              if(sStroke.name.startsWith("Spine", Qt::CaseInsensitive))
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

    auto markAsInvalid = [this](const QString &key) { updateInfoCache(key, tr("Failed to compute")); };

    if(shaftLength != 0)
    {
      updateInfoCache(DENDRITE_SPINES_LD, static_cast<double>(spinesNum)/shaftLength);
    }
    else
    {
      markAsInvalid(DENDRITE_SPINES_LD);
    }
    updateInfoCache(DENDRITE_SPINES_LENGTH, completeSpinesLength);

    if (spinesNum - truncated != 0)
    {
      updateInfoCache(DENDRITE_SPINES_LENGTH_MEAN, completeSpinesLength / (spinesNum - truncated));
    }
    else
    {
      markAsInvalid(DENDRITE_SPINES_LENGTH_MEAN);
    }

    auto toDouble = [](unsigned int number) { return static_cast<double>(number); };

    auto totalConnections = connections.size();
    updateInfoCache(DENDRITE_SYNAPSES_NUM, totalConnections);
    updateInfoCache(DENDRITE_SYNAPSES_ON_SPINES, connectionsOnSpine);
    updateInfoCache(DENDRITE_SYNAPSES_ON_SHAFT, totalConnections - connectionsOnSpine);

    if(connectionsOnSpine != 0)
    {
      updateInfoCache(DENDRITE_SYNAPSES_LOCATION_RATIO, toDouble(totalConnections-connectionsOnSpine)/connectionsOnSpine);
    }
    else
    {
      markAsInvalid(DENDRITE_SYNAPSES_LOCATION_RATIO);
    }

    if(shaftLength != 0)
    {
      updateInfoCache(DENDRITE_SYNAPSES_LD, toDouble(totalConnections)/shaftLength);
    }
    else
    {
      markAsInvalid(DENDRITE_SYNAPSES_LD);
    }

    if(shaftLength != 0)
    {
      updateInfoCache(DENDRITE_SYNAPSES_SHAFT_LD, toDouble(totalConnections-connectionsOnSpine)/shaftLength);
    }
    else
    {
      markAsInvalid(DENDRITE_SYNAPSES_SHAFT_LD);
    }

    if(shaftLength != 0)
    {
      updateInfoCache(DENDRITE_SYNAPSES_SPINE_LD, toDouble(connectionsOnSpine)/shaftLength);
    }
    else
    {
      markAsInvalid(DENDRITE_SYNAPSES_SPINE_LD);
    }

    if(spinesNum-truncated != 0)
    {
      updateInfoCache(DENDRITE_SYNAPSES_PER_SPINE_MEAN, toDouble(connectionsOnSpine)/toDouble(spinesNum-truncated));
    }
    else
    {
      markAsInvalid(DENDRITE_SYNAPSES_PER_SPINE_MEAN);
    }

    if(!spineDistances.isEmpty())
    {
      double meanNearest = std::accumulate(spineDistances.begin(), spineDistances.end(), 0.0)/spineDistances.size();
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

    if(symmetric != 0)
    {
      updateInfoCache(DENDRITE_SYNAPSES_RATIO, toDouble(totalConnections-symmetric)/symmetric);
    }
    else
    {
      markAsInvalid(DENDRITE_SYNAPSES_RATIO);
    }

    definition.clear();
  }
}

//---------------------------------------------------------------------
const QList<struct DendriteSkeletonInformation::SpineInformation> DendriteSkeletonInformation::spinesInformation() const
{
  if(m_extendedItem && !isReady(createKey(DENDRITE_SHAFT_LENGTH)))
  {
    updateInformation();
  }

  return m_spines;
}

//--------------------------------------------------------------------
void DendriteSkeletonInformation::invalidateImplementation()
{
  m_spines.clear();
}

//--------------------------------------------------------------------
void DendriteSkeletonInformation::onExtendedItemSet(Segmentation* item)
{
  if(item && item->storage()->exists(snapshotName(SPINE_SNAPSHOT_FILE)))
  {
    auto buffer = item->storage()->snapshot(snapshotName(SPINE_SNAPSHOT_FILE));
    const QChar SEPARATOR{','};
    const QChar NEWLINE  {'\n'};

    for(auto spineInfoLine: QString{buffer}.split(NEWLINE))
    {
      auto parts = spineInfoLine.split(SEPARATOR);
      Q_ASSERT(parts.size() == 15);

      struct SpineInformation spineInfo;
      spineInfo.name               = parts.at(0);
      spineInfo.parentName         = parts.at(1);
      spineInfo.complete           = (parts.at(2) == "true");
      spineInfo.branched           = (parts.at(3) == "true");
      spineInfo.length             = parts.at(4).toDouble();
      spineInfo.numSynapses        = parts.at(5).toInt();
      spineInfo.numAsymmetric      = parts.at(6).toInt();
      spineInfo.numAsymmetricHead  = parts.at(7).toInt();
      spineInfo.numAsymmetricNeck  = parts.at(8).toInt();
      spineInfo.numSymmetric       = parts.at(9).toInt();
      spineInfo.numSymmetricHead   = parts.at(10).toInt();
      spineInfo.numSymmetricNeck   = parts.at(11).toInt();
      spineInfo.numAxons           = parts.at(12).toInt();
      spineInfo.numAxonsInhibitory = parts.at(13).toInt();
      spineInfo.numAxonsExcitatory = parts.at(14).toInt();

      m_spines << spineInfo;
    }
  }
}
