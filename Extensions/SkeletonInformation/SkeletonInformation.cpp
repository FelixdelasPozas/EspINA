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
#include <Extensions/SkeletonInformation/SkeletonInformation.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QTimer>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type EspinaExtensions_EXPORT SkeletonInformation::TYPE = "SkeletonInformation";

const SegmentationExtension::Key SKELETON_LENGTH           = "Total Length (Nm)";
const SegmentationExtension::Key SKELETON_COMPONENTS       = "Connected Components";
const SegmentationExtension::Key SKELETON_PATHS            = "Number Of Segments";
const SegmentationExtension::Key SKELETON_LOOPS            = "Cycles";
const SegmentationExtension::Key SKELETON_CONNECTIONS      = "Number Of Connections";

//--------------------------------------------------------------------
SkeletonInformation::SkeletonInformation(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
{
}

//--------------------------------------------------------------------
SkeletonInformation::~SkeletonInformation()
{
}

//--------------------------------------------------------------------
State SkeletonInformation::state() const
{
  return State();
}

//--------------------------------------------------------------------
Snapshot SkeletonInformation::snapshot() const
{
  return Snapshot();
}

//--------------------------------------------------------------------
SegmentationExtension::InformationKeyList SkeletonInformation::availableInformation() const
{
  InformationKeyList keys;

  for (auto value : {SKELETON_LENGTH, SKELETON_COMPONENTS, SKELETON_PATHS, SKELETON_LOOPS, SKELETON_CONNECTIONS})
  {
    keys << createKey(value);
  }

  keys << m_keys; // add custom keys.

  return keys;
}

//--------------------------------------------------------------------
QVariant SkeletonInformation::cacheFail(const InformationKey& key) const
{
  QVariant info;

  if(hasSkeletonData(m_extendedItem->output()))
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
void SkeletonInformation::onExtendedItemSet(Segmentation* item)
{
  updateKeys();
}

//--------------------------------------------------------------------
void SkeletonInformation::updateInformation() const
{
  QWriteLocker lock(&m_mutex);

  Q_ASSERT(hasSkeletonData(m_extendedItem->output()));

  if(m_extendedItem && readLockSkeleton(m_extendedItem->output())->isValid())
  {
    auto skeleton   = readLockSkeleton(m_extendedItem->output())->skeleton();
    auto nodes      = toNodes(skeleton);
    auto components = connectedComponents(nodes);
    PathList pathList;
    QList<SkeletonNodes> loopList;

    auto follow = [](SkeletonNode *node, SkeletonNodes visited)
    {
      while(node->connections.size() == 2 && !visited.contains(node))
      {
        for(auto connection: node->connections)
        {
          if(connection == visited.last()) continue;

          visited << node;
          node = connection;
        }
      }
      return node->id;
    };

    auto angle = [](SkeletonNode *base, SkeletonNode *a, SkeletonNode *b)
    {
      double vector1[3]{a->position[0]-base->position[0], a->position[1]-base->position[1], a->position[2]-base->position[2]};
      double vector2[3]{b->position[0]-base->position[0], b->position[1]-base->position[1], b->position[2]-base->position[2]};

      return vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(vector1, vector2));
    };

    double totalLength = 0;
    for(auto component: components)
    {
      auto index = components.indexOf(component) + 1;
      auto componentPaths = paths(component);

      double length = 0;
      for(auto path: componentPaths)
      {
        length += path.length();
        totalLength += path.length();

        auto name = (path.begin->id == path.end->id ? path.begin->id + " (Loop)" : path.begin->id + "<->" + path.end->id);
        updateInfoCache(tr("Segment %1 Length (Nm)").arg(name), path.length());

        if(path.begin->connections.size() != 1)
        {
          QStringList angleNames;
          for(auto connection: path.begin->connections)
          {
            if(connection == path.seen.at(1) || connection == path.seen.at(path.seen.length()-2)) continue;

            auto endName = follow(connection, SkeletonNodes{path.begin});
            auto name    = tr("Angle %1^%2^%3 (Degrees)").arg(path.end->id).arg(path.begin->id).arg(endName);
            if(angleNames.contains(name)) continue;
            angleNames << name;

            updateInfoCache(name, tr("%1").arg(angle(path.begin, path.seen.at(1), connection)));
          }
        }
      }
      pathList << componentPaths;

      updateInfoCache(tr("Component %1 Length (Nm)").arg(index), length);
      auto comploops = loops(component);
      if(comploops.size() != 0)
      {
        updateInfoCache(tr("Component %1 Loops").arg(index), comploops.size());
        loopList << comploops;
      }
    }

    updateInfoCache(SKELETON_LENGTH,     totalLength);
    updateInfoCache(SKELETON_COMPONENTS, components.size());
    updateInfoCache(SKELETON_PATHS,      pathList.size());
    updateInfoCache(SKELETON_LOOPS,      loopList.size());
    updateInfoCache(SKELETON_CONNECTIONS, 0); // TODO
  }
}

//--------------------------------------------------------------------
void SkeletonInformation::invalidateImplementation()
{
  // need to defer the keys update so it doesn't interfere with representation updates.
  QTimer::singleShot(0, this, SLOT(updateKeys()));
}

//--------------------------------------------------------------------
void SkeletonInformation::updateKeys()
{
  m_keys.clear();

  auto skeleton   = readLockSkeleton(m_extendedItem->output())->skeleton();
  auto nodes      = toNodes(skeleton);
  auto components = connectedComponents(nodes);
  PathList pathList;
  QList<SkeletonNodes> loopList;

  auto follow = [](SkeletonNode *node, SkeletonNodes visited)
  {
    while(node->connections.size() == 2 && !visited.contains(node))
    {
      for(auto connection: node->connections)
      {
        if(connection == visited.last()) continue;

        visited << node;
        node = connection;
      }
    }

    return node->id;
  };

  InformationKeyList connectedLength, angles, pathLength, loopsKeys;

  QStringList angleNames;
  for(auto component: components)
  {
    loopList.clear();
    pathList.clear();

    auto index = components.indexOf(component) + 1;
    auto componentPaths = paths(component);

    connectedLength << createKey(tr("Component %1 Length (Nm)").arg(index));
    loopList = loops(component);
    if(loopList.size() != 0) loopsKeys << createKey(tr("Component %1 Loops").arg(index));

    pathList = paths(component);
    for(auto path: pathList)
    {
      auto name = (path.begin->id == path.end->id ? path.begin->id + " (Loop)" : path.begin->id + "<->" + path.end->id);
      pathLength << createKey(tr("Segment %1 Length (Nm)").arg(name));

      if(path.begin->connections.size() != 1)
      {
        for(auto connection: path.begin->connections)
        {
          if(connection == path.seen.at(1) || connection == path.seen.at(path.seen.length()-2)) continue;

          auto endName = follow(connection, SkeletonNodes{path.begin});
          auto name    = tr("Angle %1^%2^%3 (Degrees)").arg(path.end->id).arg(path.begin->id).arg(endName);
          if(angleNames.contains(name)) continue;
          angleNames << name;

          angles << createKey(name);
        }
      }
    }
  }

  // put a little order in the keys.
  for(auto group: {connectedLength, loopsKeys, pathLength, angles})
  {
    qSort(group.begin(), group.end());

    for(auto key: group)
    {
      m_keys << key;
    }
  }
}
