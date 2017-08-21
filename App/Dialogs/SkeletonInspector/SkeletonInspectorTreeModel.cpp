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
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/CategoryAdapter.h>
#include <Dialogs/SkeletonInspector/SkeletonInspectorTreeModel.h>

// VTK
#include <vtkActor.h>

using namespace ESPINA;

//--------------------------------------------------------------------
SkeletonInspectorTreeModel::SkeletonInspectorTreeModel(const SegmentationAdapterSPtr   segmentation,
                                                       const SegmentationAdapterList  &segmentations,
                                                       const QList<struct StrokeInfo> &strokes,
                                                       QObject                        *parent)
: QAbstractItemModel{parent}
, m_segmentation    {segmentation}
, m_connections     (segmentations)
, m_strokes         (strokes)
{
}

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
            return tr("Connections");
          }
        }
        else
        {
          if(index.parent().row() == 0)
          {
            auto stroke = m_strokes.at(index.row());
            return tr("%1 (%2 nm)").arg(stroke.name).arg(stroke.length);
          }
          else
          {
            return m_connections.at(index.row())->data(role);
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
          if(index.parent().row() == 0)
          {
            QPixmap icon(3, 16);
            icon.fill(QColor::fromHsv(m_strokes.at(index.row()).hue, 255, 255));

            return icon;
          }
          else
          {
            return m_connections.at(index.row())->data(role);
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
          if(index.parent().row() == 0)
          {
            auto stroke = m_strokes.at(index.row());
            return tr("%1 (%2 nm)").arg(stroke.name).arg(stroke.length);
          }
          else
          {
            return m_connections.at(index.row())->data(role);
          }
        }
      }
      break;
    case Qt::CheckStateRole:
      {
        if(!index.parent().isValid())
        {
          int count = 0;
          if(index.row() == 0)
          {
            for(auto stroke: m_strokes)
            {
              if(stroke.actor->GetVisibility()) ++count;
            }

            if(count == m_strokes.size()) return Qt::Checked;
          }
          else
          {
            for(auto segmentation: m_connections)
            {
              if(segmentation->isVisible()) ++count;
            }

            if(count == m_connections.size()) return Qt::Checked;
          }

          if(count == 0) return Qt::Unchecked;
          return Qt::PartiallyChecked;
        }
        else
        {
          if(index.parent().row() == 0)
          {
            return (m_strokes.at(index.row()).actor->GetVisibility() ? Qt::Checked : Qt::Unchecked);
          }
          else
          {
            return m_connections.at(index.row())->data(role);
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
  if(!index.isValid() || (index.parent().isValid() && index.parent().internalPointer() != nullptr))
  {
    return Qt::ItemFlags();
  }

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
    return createIndex(row, column, nullptr);
  }
  else
  {
    if(parent.internalPointer() != nullptr) return QModelIndex();

    if(parent.row() == 0)
    {
      auto ptr = const_cast<struct StrokeInfo *>(&m_strokes.at(row));
      return createIndex(row, column, reinterpret_cast<void *>(ptr));
    }
    else
    {
      return createIndex(row, column, m_connections.at(row));
    }
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
QModelIndex SkeletonInspectorTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid() || index.internalPointer() == nullptr) return QModelIndex();

  if(reinterpret_cast<struct StrokeInfo*>(index.internalPointer()) == &m_strokes.at(index.row()))
  {
    return createIndex(0,0,nullptr);
  }
  else
  {
    return createIndex(1,0,nullptr);
  }

  return QModelIndex();
}

//--------------------------------------------------------------------
int SkeletonInspectorTreeModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    if(parent.internalPointer() != nullptr) return 0;

    switch(parent.row())
    {
      case 0:
        return m_strokes.size();
        break;
      case 1:
        return m_connections.size();
        break;
      default:
        break;
    }
  }

  return 2; // root->rowCount();
}

//--------------------------------------------------------------------
int SkeletonInspectorTreeModel::columnCount(const QModelIndex& parent) const
{
  if(parent.internalPointer() != nullptr) return 0;

  return 1;
}

//--------------------------------------------------------------------
bool SkeletonInspectorTreeModel::setData(const QModelIndex& modelIndex, const QVariant& value, int role)
{
  if(!modelIndex.isValid() || role != Qt::CheckStateRole) return false;

  auto state = value.toBool();

  ViewItemAdapterList toInvalidate;

  if(!modelIndex.parent().isValid())
  {
    switch(modelIndex.row())
    {
      case 0:
        for(auto stroke: m_strokes)
        {
          stroke.actor->SetVisibility(state == true);
          auto newIndex = index(m_strokes.indexOf(stroke),0, modelIndex);
          emit dataChanged(newIndex, newIndex);
        }
        toInvalidate << m_segmentation.get();
        break;
      default:
        for(auto connection: m_connections)
        {
          connection->setData(value, role);
          auto newIndex = index(m_connections.indexOf(connection),0, modelIndex);
          emit dataChanged(newIndex, newIndex);
          toInvalidate << connection;
        }
        break;
    }
    emit dataChanged(modelIndex, modelIndex);
  }
  else
  {
    switch(modelIndex.parent().row())
    {
      case 0:
        m_strokes.at(modelIndex.row()).actor->SetVisibility(state == true);
        toInvalidate << m_segmentation.get();
        break;
      default:
        m_connections.at(modelIndex.row())->setData(value, role);
        toInvalidate << m_connections.at(modelIndex.row());
        break;
    }
    emit dataChanged(modelIndex, modelIndex);
    emit dataChanged(modelIndex.parent(), modelIndex.parent());
  }

  emit invalidate(toInvalidate);

  return true;
}
