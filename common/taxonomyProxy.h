/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#include <QSortFilterProxyModel>

// Forward declaration
class EspINA;
class TaxonomyNode;
class Segmentation;

#ifndef TAXONOMYPROXY_H
#define TAXONOMYPROXY_H
class TaxonomyProxy : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  TaxonomyProxy(QObject *parent=0);
  virtual ~TaxonomyProxy();
  
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;
  
protected:
  void updateSegmentations() const;
  
private:
  mutable QMap<const TaxonomyNode *, QList<Segmentation *> > m_taxonomySegs;
};

#endif // TAXONOMYPROXY_H
