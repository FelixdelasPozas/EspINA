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
#include <Extensions/SkeletonInformation/SynapseInformation.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type EspinaExtensions_EXPORT SynapseConnectionInformation::TYPE = "SynapseConnections";

const SegmentationExtension::Key SYNAPSE_DENDRITE_CONNECTION = QObject::tr("Dendrite connection");
const SegmentationExtension::Key SYNAPSE_DENDRITE_LOCATION   = QObject::tr("Shaft/Spine");
const SegmentationExtension::Key SYNAPSE_SPINE               = QObject::tr("Spine name");
const SegmentationExtension::Key SYNAPSE_BRANCHED_SPINE      = QObject::tr("Branched spine");
const SegmentationExtension::Key SYNAPSE_SPINE_LOCATION      = QObject::tr("Location in spine");
const SegmentationExtension::Key SYNAPSE_AXON_CONNECTION     = QObject::tr("Axon connection");

//--------------------------------------------------------------------
SynapseConnectionInformation::SynapseConnectionInformation(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
{
}

//--------------------------------------------------------------------
SynapseConnectionInformation::~SynapseConnectionInformation()
{
}

//--------------------------------------------------------------------
State SynapseConnectionInformation::state() const
{
  return State();
}

//--------------------------------------------------------------------
Snapshot SynapseConnectionInformation::snapshot() const
{
  return Snapshot();
}

//--------------------------------------------------------------------
const SegmentationExtension::InformationKeyList SynapseConnectionInformation::availableInformation() const
{
  InformationKeyList keys;

  for (auto value : {SYNAPSE_DENDRITE_CONNECTION,
                     SYNAPSE_DENDRITE_LOCATION,
                     SYNAPSE_SPINE,
                     SYNAPSE_BRANCHED_SPINE,
                     SYNAPSE_SPINE_LOCATION,
                     SYNAPSE_AXON_CONNECTION})
  {
    keys << createKey(value);
  }

  keys << m_keys; // add custom keys.

  return keys;
}

//--------------------------------------------------------------------
QVariant SynapseConnectionInformation::cacheFail(const InformationKey& key) const
{
  QVariant info;

  if(m_extendedItem && m_extendedItem->category()->classificationName().startsWith("Synapse"))
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
void SynapseConnectionInformation::updateInformation() const
{
  QWriteLocker lock(&m_mutex);
  const QString unconnected = tr("Unconnected");

  if(m_extendedItem)
  {
    QString dendriteName     = unconnected;
    QString axonName         = unconnected;
    QString dendriteLocation = unconnected;
    QString spineLocation    = unconnected;
    QString spineName        = unconnected;
    QString branched         = unconnected;

    auto connections = m_extendedItem->analysis()->connections(m_extendedItem);

    for(auto connection: connections)
    {
      auto other = std::dynamic_pointer_cast<Segmentation>(connection.segmentation2);
      Q_ASSERT(other);

      if(other->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive))
      {
        dendriteName = other->alias().isEmpty() ? other->name() : other->alias();

        auto dendrite   = readLockSkeleton(other->output())->skeleton();
        auto definition = toSkeletonDefinition(dendrite);
        auto pathList   = paths(definition.nodes, definition.edges, definition.strokes);
        auto hierarchy  = pathHierarchy(pathList, definition.edges, definition.strokes);

        std::function<Core::PathHierarchyNode*(const Core::Path &, const QList<Core::PathHierarchyNode *>)> locateHNode = [&locateHNode](const Core::Path &path, const QList<Core::PathHierarchyNode *> nodes)
        {
          PathHierarchyNode *result = nullptr;
          for(const auto &node: nodes)
          {
            if(node->path == path) return node;

            auto child = locateHNode(path, node->children);

            if(child) return child;
          }

          return result;
        };

        for(auto &path: pathList)
        {
          if(path.hasEndingPoint(connection.point))
          {
            if(path.note.startsWith("Shaft", Qt::CaseInsensitive))
            {
              dendriteLocation = tr("Shaft");
              spineName        = tr("No");
              spineLocation    = tr("No");
              branched         = tr("No");
              break;
            }

            if(path.note.startsWith("Spine", Qt::CaseInsensitive))
            {
              dendriteLocation = tr("Spine");
              spineName        = path.note;
              spineLocation    = tr("Spine head");
              branched         = tr("No");
              break;
            }

            if(path.note.startsWith("Subspine", Qt::CaseInsensitive))
            {
              dendriteLocation = tr("Spine");
              spineLocation    = tr("Spine head");
              branched         = path.note;

              auto pathNode = locateHNode(path, hierarchy);
              Q_ASSERT(pathNode);

              while(pathNode && !pathNode->path.note.startsWith("Spine")) pathNode = pathNode->parent;
              if(pathNode) spineName = pathNode->path.note;
              break;
            }

            if(path.note.startsWith("Synapse", Qt::CaseInsensitive))
            {
              if(path.note.contains("shaft", Qt::CaseInsensitive))
              {
                dendriteLocation = tr("Shaft");
                spineLocation    = tr("No");
                branched         = tr("No");
                spineName        = tr("No");
                break;
              }

              dendriteLocation = tr("Spine");
              spineLocation    = path.note.contains("head", Qt::CaseInsensitive) ? tr("Spine head") : tr("Spine neck");

              auto pathNode = locateHNode(path, hierarchy);
              while(pathNode && !pathNode->path.note.startsWith("Spine", Qt::CaseInsensitive))
              {
                if(pathNode->path.note.startsWith("Subspine", Qt::CaseInsensitive))
                {
                  branched = pathNode->path.note;
                }

                pathNode = pathNode->parent;
              }

              branched  = (branched == unconnected ? tr("No") : tr("yes"));
              spineName = (pathNode ? pathNode->path.note : tr("Failed to identify"));
              break;
            }
          }
        }

        definition.clear();
      }

      if(other->category()->classificationName().startsWith("Axon", Qt::CaseInsensitive))
      {
        axonName = other->alias().isEmpty() ? other->name() : other->alias();
      }
    }

    updateInfoCache(SYNAPSE_DENDRITE_CONNECTION, dendriteName);
    updateInfoCache(SYNAPSE_AXON_CONNECTION, axonName);
    updateInfoCache(SYNAPSE_DENDRITE_LOCATION, dendriteLocation);
    updateInfoCache(SYNAPSE_SPINE, spineName);
    updateInfoCache(SYNAPSE_BRANCHED_SPINE, branched);
    updateInfoCache(SYNAPSE_SPINE_LOCATION, spineLocation);
  }
}
