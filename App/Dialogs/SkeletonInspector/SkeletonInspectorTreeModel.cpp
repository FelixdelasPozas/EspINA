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
#include <Core/Utils/ListUtils.hxx>
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/CategoryAdapter.h>
#include <Dialogs/SkeletonInspector/SkeletonInspectorTreeModel.h>

// VTK
#include <vtkActor.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
//--------------------------------------------------------------------
SkeletonInspectorTreeModel::SkeletonInspectorTreeModel(const SegmentationAdapterSPtr segmentation,
                                                       ModelAdapterSPtr              model,
                                                       QList<struct StrokeInfo>     &strokes,
                                                       QObject                      *parent)
: QAbstractItemModel {parent}
, m_StrokesTree      {nullptr}
, m_ConnectTree      {nullptr}
, m_segmentation     {segmentation}
, m_model            {model}
, m_strokes          (strokes)
, m_useRandomColoring{false}
, m_connectionLevel  {0}
{
  connectSignals();

  computeStrokesTree();

  computeConnectionDistances(2);
}

//--------------------------------------------------------------------
SkeletonInspectorTreeModel::~SkeletonInspectorTreeModel()
{
  if(m_StrokesTree) delete m_StrokesTree;
  if(m_ConnectTree) delete m_ConnectTree;
};

//--------------------------------------------------------------------
QVariant SkeletonInspectorTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) return QVariant();

  switch(role)
  {
    case Qt::DisplayRole:
      {
        if(!index.parent().isValid())
        {
          if(index.row() == 0)
          {
            double length = 0;
            for(auto stroke: m_strokes) if(stroke.used) length += stroke.length;
            return tr("%1 (%2 nm)").arg(m_segmentation->data().toString()).arg(length);
          }
          else
          {
            return tr("Connections at distance %1").arg(m_connectionLevel);
          }
        }
        else
        {
          auto node = dataNode(index);
          if(node->type == TreeNode::Type::SEGMENTATION)
          {
            auto segmentation = node->connection();
            return segmentation->data().toString();
          }
          else
          {
            auto stroke = node->stroke();
            return stroke->name;
          }
        }
      }
      break;
    case Qt::DecorationRole:
      {
        if(!index.parent().isValid())
        {
          if(index.row() == 0)
          {
            return m_segmentation->data(role);
          }
          else
          {
            QPixmap icon(3, 16);
            icon.fill(Qt::black);

            return icon;
          }
        }
        else
        {
          auto node = dataNode(index);
          if(node->type == TreeNode::Type::SEGMENTATION)
          {
            auto segmentation = node->connection();
            return segmentation->data(role);
          }
          else
          {
            auto stroke = node->stroke();
            QPixmap icon(3, 16);
            auto hue = m_useRandomColoring ? stroke->randomHue : stroke->hue;
            icon.fill(QColor::fromHsv(hue, 255, 255));

            return icon;
          }
        }
      }
      break;
    case Qt::ToolTipRole:
      {
        if(!index.parent().isValid())
        {
          if(index.row() == 0)
          {
            return m_segmentation->data(role);
          }
          else
          {
            auto name = m_segmentation->data(Qt::DisplayRole).toString();
            return tr("%1's Connections").arg(name);
          }
        }
        else
        {
          auto node = dataNode(index);
          if(node->type == TreeNode::Type::SEGMENTATION)
          {
            auto segmentation = node->connection();
            return segmentation->data(role);
          }
          else
          {
            auto stroke = node->stroke();
            return tr("%1 (%2 nm)").arg(stroke->name).arg(stroke->length);
          }
        }
      }
      break;
    case Qt::CheckStateRole:
      {
        if(!index.parent().isValid())
        {
          int count = 0;
          switch(index.row())
          {
            case 0:
              for(auto stroke: m_strokes) if(stroke.actors.first()->GetVisibility()) ++count;
              if(count == 0) return Qt::Unchecked;
              break;
            case 1:
            default:
              return (getVisibility(m_ConnectTree) ? Qt::Checked : Qt::Unchecked);
              break;
          }

          return Qt::Checked;
        }
        else
        {
          auto node = dataNode(index);
          if(node->type == TreeNode::Type::SEGMENTATION)
          {
            auto segmentation = node->connection();
            return segmentation->data(role);
          }
          else
          {
            auto stroke = node->stroke();
            if(stroke->actors.first()->GetVisibility()) return Qt::Checked;
            return Qt::Unchecked;
          }
        }
      }
      break;
    default:
      break;
  }

  return QVariant();
}

//--------------------------------------------------------------------
Qt::ItemFlags SkeletonInspectorTreeModel::flags(const QModelIndex& index) const
{
  if(!index.isValid()) return Qt::ItemFlags();

  return (Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
}

//--------------------------------------------------------------------
QVariant SkeletonInspectorTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant();
}

//--------------------------------------------------------------------
QModelIndex SkeletonInspectorTreeModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent)) return QModelIndex();

  if(!parent.isValid())
  {
    switch(row)
    {
      case 0:
        return createIndex(0, 0, m_StrokesTree);
        break;
      default:
        return createIndex(1, 0, m_ConnectTree);
        break;
    }
  }
  else
  {
    auto node = dataNode(parent);
    return createIndex(row, column, node->children.at(row));
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
QModelIndex SkeletonInspectorTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) return QModelIndex();

  auto node = dataNode(index);
  if(!node->parent)
  {
    Q_ASSERT(node->type == TreeNode::Type::ROOT);
    return QModelIndex();
  }

  if(node->parent->type == TreeNode::Type::ROOT)
  {
    if(node->type == TreeNode::Type::STROKE)
    {
      return createIndex(0,0, m_StrokesTree);
    }
    else
    {
      return createIndex(1,0, m_ConnectTree);
    }
  }

  auto grandParent = node->parent->parent;
  if(grandParent)
  {
    return createIndex(grandParent->children.indexOf(node->parent), 0, node->parent);
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
int SkeletonInspectorTreeModel::rowCount(const QModelIndex& parent) const
{
  if(parent.isValid())
  {
    auto node = dataNode(parent);
    return node->children.size();
  }

  return 2; // root->rowCount();
}

//--------------------------------------------------------------------
int SkeletonInspectorTreeModel::columnCount(const QModelIndex& parent) const
{
  if(parent.isValid())
  {
    auto node = dataNode(parent);
    return (node->children.isEmpty() ? 0 : 1);
  }

  return 1; // root->columnCount();
}

//--------------------------------------------------------------------
bool SkeletonInspectorTreeModel::setData(const QModelIndex& modelIndex, const QVariant& value, int role)
{
  if(!modelIndex.isValid() || role != Qt::CheckStateRole) return false;

  auto state = value.toBool();

  SegmentationAdapterList modified;

  if(!modelIndex.parent().isValid())
  {
    emit layoutAboutToBeChanged();
    switch(modelIndex.row())
    {
      case 0:
        modified << setVisibility(m_StrokesTree, state);
        break;
      default:
        modified << setVisibility(m_ConnectTree, state);
        break;
    }
    emit layoutChanged();
  }
  else
  {
    auto node = dataNode(modelIndex);

    if(node->type == TreeNode::Type::SEGMENTATION)
    {
      auto segmentation = node->connection();
      segmentation->setData(value, role);
      modified << segmentation;

      auto connectionsIndex = createIndex(1,0,m_ConnectTree);
      emit dataChanged(connectionsIndex, connectionsIndex);
    }
    else
    {
      auto stroke = node->stroke();
      for(auto actor: stroke->actors) actor->SetVisibility(state);
      modified << m_segmentation.get();
    }
    emit dataChanged(modelIndex, modelIndex);
  }

  ViewItemAdapterList toInvalidate;
  for(auto segmentation: modified)
  {
    if(toInvalidate.contains(segmentation)) continue;

    toInvalidate << segmentation;
  }

  emit invalidate(toInvalidate);

  return true;
}

//--------------------------------------------------------------------
void SkeletonInspectorTreeModel::computeConnectionDistances(int distance)
{
  emit beginResetModel();

  if(m_ConnectTree)
  {
    delete m_ConnectTree;
  }

  m_connectionLevel = distance;

  SegmentationAdapterList shown;
  shown << m_segmentation.get();

  std::function<void(TreeNode*,int)> fillNode = [this, &fillNode, &shown](TreeNode *node, const unsigned int distance)
  {
    if(distance == 0) return;

    auto segmentation = node->connection();
    auto segSPtr = this->m_model->smartPointer(segmentation);
    auto connectSegs = connections(segmentation);

    for(auto connection: connectSegs)
    {
      auto newNode = new TreeNode();
      newNode->type = TreeNode::Type::SEGMENTATION;
      newNode->data = connection.get();
      newNode->parent = node;
      if(!shown.contains(connection.get()))
      {
        shown << connection.get();
        fillNode(newNode, distance - 1);
      }

      node->children << newNode;
    }
  };

  m_ConnectTree = new TreeNode();
  m_ConnectTree->type = TreeNode::Type::ROOT;
  m_ConnectTree->data = m_segmentation.get();
  fillNode(m_ConnectTree, distance);

  shown.removeOne(m_segmentation.get());
  emit segmentationsShown(shown);

  reset();

  emit endResetModel();
}

//--------------------------------------------------------------------
void SkeletonInspectorTreeModel::computeStrokesTree()
{
  m_StrokesTree = new TreeNode();
  m_StrokesTree->type = TreeNode::Type::ROOT;

  auto definition = toSkeletonDefinition(readLockSkeleton(m_segmentation->output())->skeleton());

  PathList pathList;
  for(auto stroke: m_strokes) pathList << stroke.path;
  auto hierarchy  = pathHierarchy(pathList, definition.edges, definition.strokes);

  int i = 0;
  std::function<TreeNode *(PathHierarchyNode *)> fillNode = [&fillNode, this, &i](PathHierarchyNode *node)
  {
    auto tNode = new TreeNode();
    tNode->type = TreeNode::Type::STROKE;

    for(auto &stroke: this->m_strokes)
    {
      if(stroke.path == node->path) tNode->data = &stroke;
    }

    for(auto child: node->children)
    {
      auto otherTNode = fillNode(child);
      otherTNode->parent = tNode;
      tNode->children << otherTNode;
    }

    return tNode;
  };

  for(auto node: hierarchy)
  {
    auto otherNode = fillNode(node);
    otherNode->parent = m_StrokesTree;
    m_StrokesTree->children << otherNode;
  }
}

//--------------------------------------------------------------------
SegmentationAdapterSList SkeletonInspectorTreeModel::connections(const SegmentationAdapterPtr segmentation) const
{
  SegmentationAdapterSList result;

  if(segmentation)
  {
    auto segSPtr = m_model->smartPointer(segmentation);
    for(auto connection: m_model->connections(segSPtr))
    {
      result << ((connection.item1 == segSPtr) ? connection.item2 : connection.item1);
    }
  }

  return result;
}

//--------------------------------------------------------------------
const SegmentationAdapterList SkeletonInspectorTreeModel::setVisibility(TreeNode *node, bool visible)
{
  QSet<SegmentationAdapterPtr> result;

  switch(node->type)
  {
    case TreeNode::Type::SEGMENTATION:
      {
        auto seg = node->connection();
        seg->setData(visible ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        result << seg;
      }
      break;
    case TreeNode::Type::STROKE:
      {
        auto stroke = node->stroke();
        for(auto actor: stroke->actors) actor->SetVisibility(visible);
        result << m_segmentation.get();
      }
      break;
    default:
    case TreeNode::Type::ROOT:
      break;
  }

  for(auto child: node->children)
  {
    result += setVisibility(child, visible).toSet();
  }

  return result.toList();
}

//--------------------------------------------------------------------
const bool SkeletonInspectorTreeModel::getVisibility(TreeNode *node) const
{
  bool result = true;

  switch(node->type)
  {
    case TreeNode::Type::SEGMENTATION:
      {
        auto segmentation = node->connection();
        if(segmentation != m_segmentation.get()) result &= segmentation->isVisible();
      }
      break;
    case TreeNode::Type::STROKE:
      result &= node->stroke()->actors.first()->GetVisibility();
      break;
    case TreeNode::Type::ROOT:
      break;
  }

  if(!result) return result;

  for(auto child: node->children)
  {
    result &= getVisibility(child);
    if(!result) return result;
  }

  return result;
}

//--------------------------------------------------------------------
void SkeletonInspectorTreeModel::setRandomTreeColoring(const bool enabled)
{
  if(m_useRandomColoring != enabled)
  {
    emit layoutAboutToBeChanged();
    m_useRandomColoring = enabled;
    emit layoutChanged();
  }
}

//--------------------------------------------------------------------
void SkeletonInspectorTreeModel::onSelectionChanged(SegmentationAdapterList segmentations)
{
  ViewItemAdapterList toInvalidate;

  auto segSelected = segmentations.contains(m_segmentation.get());
  for(auto stroke: m_strokes)
  {
    stroke.selected = segSelected;
  }

  toInvalidate << m_segmentation.get();

  emit invalidate(toInvalidate);
}

//--------------------------------------------------------------------
void SkeletonInspectorTreeModel::connectSignals()
{
}
