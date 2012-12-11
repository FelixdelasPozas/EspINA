/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef COMPOSITIONPROXY_H
#define COMPOSITIONPROXY_H

#include <QtGui/QAbstractProxyModel>
#include <Core/EspinaTypes.h>

class ModelItem;
class EspinaModel;

/// Arrange segmentations using their COMPOSED relationship
class CompositionProxy
: public QAbstractProxyModel
{
  Q_OBJECT
public:
  explicit CompositionProxy(QObject* parent = 0);
  virtual ~CompositionProxy();

  virtual void setSourceModel(EspinaModel *sourceModel);

  virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const { return 1; }
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

  // Drag & Drop
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual Qt::DropActions supportedDropActions() const {return Qt::MoveAction;}
  virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

protected slots:
  void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
  void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
  void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);
  void sourceDataChanged(const QModelIndex &sourceTopLeft, const QModelIndex &sourceBottomRight);

protected:
  Segmentation *findSegmentation(QString tooltip) const;
  bool indices(const QModelIndex& topLeft, const QModelIndex& bottomRight, QModelIndexList& result);
  Segmentation *parentSegmentation(ModelItem *segItem) const;

private:
  EspinaModel *m_sourceModel;

  SegmentationList m_rootSegmentations;
  mutable QMap<Segmentation *, QList<ModelItem *> > m_components;
};

#endif // COMPOSITIONPROXY_H
