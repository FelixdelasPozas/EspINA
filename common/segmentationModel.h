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


#ifndef SEGMENTATIONMODEL_H
#define SEGMENTATIONMODEL_H

#include <QModelIndex>

// Forward declarations
class TaxonomyNode;

//! An Abstract Model to represent Segementations
class SegmentationModel : public QAbstractItemModel
{
  Q_OBJECT
public:
    SegmentationModel();
    virtual ~SegmentationModel();
    
    //! Implement QAbstractItemModel Interface
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    
    //! Segmentation Model methods
    void setTaxonomy(TaxonomyNode *taxonomy) {m_tax = taxonomy;}
private:
  TaxonomyNode *m_tax;
};

#endif // SEGMENTATIONMODEL_H
