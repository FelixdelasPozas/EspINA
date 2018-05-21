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
#include <Extensions/SkeletonInformation/AxonInformation.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type EspinaExtensions_EXPORT AxonSkeletonInformation::TYPE = "AxonInformation";

const SegmentationExtension::Key AXON_SHAFT_LENGTH          = QObject::tr("Shaft length (Nm)");
const SegmentationExtension::Key AXON_SYNAPSES_NUM          = QObject::tr("Num of synapses");
const SegmentationExtension::Key AXON_SYNAPSES_PASSANT      = QObject::tr("Num of synapses en passant");
const SegmentationExtension::Key AXON_SYNAPSES_TERMINALS    = QObject::tr("Num of synapses on axon terminals");
const SegmentationExtension::Key AXON_SYNAPSES_LD           = QObject::tr("Linear density of synapses");
const SegmentationExtension::Key AXON_SYNAPSES_ON_SPINE_NUM = QObject::tr("Num of synapses on dendritic spines");
const SegmentationExtension::Key AXON_SYNAPSES_ON_SHAFT_NUM = QObject::tr("Num of synapses on dendritic shafts");
const SegmentationExtension::Key AXON_SYNAPSES_RATIO        = QObject::tr("Ratio of synapses on spines and shafts");

//--------------------------------------------------------------------
AxonSkeletonInformation::AxonSkeletonInformation(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
{
}

//--------------------------------------------------------------------
AxonSkeletonInformation::~AxonSkeletonInformation()
{
}

//--------------------------------------------------------------------
State AxonSkeletonInformation::state() const
{
  return State();
}

//--------------------------------------------------------------------
Snapshot AxonSkeletonInformation::snapshot() const
{
  return Snapshot();
}

//--------------------------------------------------------------------
SegmentationExtension::InformationKeyList AxonSkeletonInformation::availableInformation() const
{
  InformationKeyList keys;

  for (auto value : {AXON_SHAFT_LENGTH,
                     AXON_SYNAPSES_NUM,
                     AXON_SYNAPSES_PASSANT,
                     AXON_SYNAPSES_TERMINALS,
                     AXON_SYNAPSES_LD,
                     AXON_SYNAPSES_ON_SPINE_NUM,
                     AXON_SYNAPSES_ON_SHAFT_NUM,
                     AXON_SYNAPSES_RATIO})
  {
    keys << createKey(value);
  }

  keys << m_keys; // add custom keys.

  return keys;
}

//--------------------------------------------------------------------
QVariant AxonSkeletonInformation::cacheFail(const InformationKey& key) const
{
  QVariant info;

  if(m_extendedItem && hasSkeletonData(m_extendedItem->output()) && m_extendedItem->category()->classificationName().startsWith("Axon"))
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
void AxonSkeletonInformation::updateInformation() const
{
  QWriteLocker lock(&m_mutex);

  Q_ASSERT(hasSkeletonData(m_extendedItem->output()));

  if(m_extendedItem && readLockSkeleton(m_extendedItem->output())->isValid())
  {
    auto axon       = readLockSkeleton(m_extendedItem->output())->skeleton();
    auto definition = toSkeletonDefinition(axon);
    auto nodes      = definition.nodes;
    auto edges      = definition.edges;
    auto strokes    = definition.strokes;
    auto pathList   = paths(nodes, edges, strokes);

    double shaftLength = 0;
    unsigned int passantNum = 0;
    for(auto path: pathList)
    {
      if(path.note.startsWith("Shaft", Qt::CaseInsensitive))
      {
        shaftLength += path.length();
        continue;
      }

      if(path.note.startsWith("Synapse en passant", Qt::CaseInsensitive))
      {
        ++passantNum;
      }
    }

    auto connections = m_extendedItem->analysis()->connections(m_extendedItem);
    auto synapsesNum = connections.size();

    updateInfoCache(AXON_SHAFT_LENGTH, shaftLength);
    updateInfoCache(AXON_SYNAPSES_NUM, synapsesNum);
    updateInfoCache(AXON_SYNAPSES_PASSANT, passantNum);
    updateInfoCache(AXON_SYNAPSES_TERMINALS, synapsesNum-passantNum);
    updateInfoCache(AXON_SYNAPSES_LD, static_cast<double>(synapsesNum)/shaftLength);

    // NOTE: axon <-> synapse <-> dendrite if the synapse is fully connected.
    unsigned int spineConnections = 0;
    for(auto connection: connections)
    {
      auto synapseItem = connection.segmentation2;
      auto synapseConnections = m_extendedItem->analysis()->connections(synapseItem);
      if(synapseConnections.size() == 1) continue; // means only connected to axon and not yet to a dendrite.

      PersistentSPtr dendriteSeg = nullptr;
      PersistentPtr extendedPtr = dynamic_cast<PersistentPtr>(m_extendedItem);

      NmVector3 point;
      for(auto sConnection: synapseConnections)
      {
        if(sConnection.segmentation2.get() == extendedPtr) continue;
        dendriteSeg = sConnection.segmentation2;
        point = sConnection.point;
        break;
      }

      Q_ASSERT(dendriteSeg);
      auto segmentation = std::dynamic_pointer_cast<Segmentation>(dendriteSeg);
      Q_ASSERT(segmentation && hasSkeletonData(segmentation->output()));

      auto dendrite    = readLockSkeleton(segmentation->output())->skeleton();
      auto dDefinition = toSkeletonDefinition(dendrite);
      auto dPathList   = paths(dDefinition.nodes, dDefinition.edges, dDefinition.strokes);

      for(auto dPath: dPathList)
      {
        if(dPath.hasEndingPoint(point))
        {
          if(dPath.note.startsWith("Synapse on spine", Qt::CaseInsensitive) ||
             dPath.note.startsWith("Spine",            Qt::CaseInsensitive) ||
             dPath.note.startsWith("Subspine",         Qt::CaseInsensitive))
          {
            ++spineConnections;
          }
          break;
        }
      }
    }

    auto shaftConnections = connections.size() - spineConnections;
    updateInfoCache(AXON_SYNAPSES_ON_SPINE_NUM, spineConnections);
    updateInfoCache(AXON_SYNAPSES_ON_SHAFT_NUM, shaftConnections);

    auto ratio = (shaftConnections == 0 ? 0 : static_cast<double>(spineConnections)/shaftConnections);
    updateInfoCache(AXON_SYNAPSES_RATIO, ratio);
  }
}
