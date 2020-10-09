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
const SegmentationExtension::Key SYNAPSE_TRUNCATED_SPINE     = QObject::tr("Truncated spine");
const SegmentationExtension::Key SYNAPSE_SPINE_LOCATION      = QObject::tr("Location in spine");
const SegmentationExtension::Key SYNAPSE_AXON_CONNECTION     = QObject::tr("Axon connection");

//--------------------------------------------------------------------
SynapseConnectionInformation::SynapseConnectionInformation(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
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
                     SYNAPSE_TRUNCATED_SPINE,
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
    QString truncated        = unconnected;

    const auto connections = m_extendedItem->analysis()->connections(m_extendedItem);

    for(auto &connection: connections)
    {
      if(dendriteName != unconnected && axonName != unconnected) break;

      auto other = std::dynamic_pointer_cast<Segmentation>(connection.segmentation2);
      Q_ASSERT(other);

      if(other->category()->classificationName().startsWith("Axon", Qt::CaseInsensitive))
      {
        axonName = other->alias().isEmpty() ? other->name() : other->alias();
        continue;
      }

      if(other->category()->classificationName().startsWith("Dendrite", Qt::CaseInsensitive))
      {
        dendriteName = other->alias().isEmpty() ? other->name() : other->alias();

        auto dendrite   = readLockSkeleton(other->output())->skeleton();
        auto definition = toSkeletonDefinition(dendrite);
        const auto pathList   = paths(definition.nodes, definition.edges, definition.strokes);
        const auto hierarchy  = pathHierarchy(pathList, definition.edges, definition.strokes);

        for(auto &path: pathList)
        {
          if(!path.hasEndingPoint(connection.point)) continue;

          if(path.note.startsWith("Shaft", Qt::CaseInsensitive))
          {
            dendriteLocation = tr("Shaft");
            spineName        = tr("No");
            spineLocation    = tr("No");
            branched         = tr("No");
            truncated        = tr("No");
            break;
          }

          auto pathNode = locatePathHierarchyNode(path, hierarchy);

          if(path.note.startsWith("Spine", Qt::CaseInsensitive))
          {
            dendriteLocation = tr("Spine");
            spineLocation    = tr("Spine head");
            branched         = tr("No");
            truncated        = isTruncated(pathNode) ? tr("Yes"): tr("No");
            spineName        = path.note;
            break;
          }

          if(path.note.startsWith("Subspine", Qt::CaseInsensitive))
          {
            dendriteLocation = tr("Spine");
            spineLocation    = tr("Spine head");
            branched         = path.note;
            truncated        = tr("No");

            while(pathNode && !pathNode->path.note.startsWith("Spine", Qt::CaseInsensitive)) pathNode = pathNode->parent;
            spineName = (pathNode ? pathNode->path.note : tr("Failed to identify spine"));

            if(pathNode) truncated = isTruncated(pathNode) ? tr("Yes"): tr("No");
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
              truncated        = tr("No");
              break;
            }

            dendriteLocation = tr("Spine");
            spineLocation    = path.note.contains("head", Qt::CaseInsensitive) ? tr("Spine head") : tr("Spine neck");
            branched         = tr("No");

            while(pathNode && !pathNode->path.note.startsWith("Spine", Qt::CaseInsensitive))
            {
              if(pathNode->path.note.startsWith("Subspine", Qt::CaseInsensitive))
              {
                branched = pathNode->path.note;
              }

              pathNode = pathNode->parent;
            }

            truncated = isTruncated(pathNode) ? tr("Yes"): tr("No");
            spineName = (pathNode ? pathNode->path.note : tr("Failed to identify spine"));
            break;
          }
        }

        definition.clear();
      }
    }

    // 2020-05-23: fix spine name if child is truncated
    if(truncated.startsWith("yes", Qt::CaseInsensitive) && !spineName.endsWith(" (truncated)", Qt::CaseInsensitive))
    {
      spineName += tr(" (Truncated)");
    }

    updateInfoCache(SYNAPSE_DENDRITE_CONNECTION, dendriteName);
    updateInfoCache(SYNAPSE_AXON_CONNECTION, axonName);
    updateInfoCache(SYNAPSE_DENDRITE_LOCATION, dendriteLocation);
    updateInfoCache(SYNAPSE_SPINE, spineName);
    updateInfoCache(SYNAPSE_BRANCHED_SPINE, branched);
    updateInfoCache(SYNAPSE_TRUNCATED_SPINE, truncated);
    updateInfoCache(SYNAPSE_SPINE_LOCATION, spineLocation);
  }
}
