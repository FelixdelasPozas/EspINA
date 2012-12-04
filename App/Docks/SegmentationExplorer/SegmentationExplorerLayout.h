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


#ifndef SEGMENTATIONEXPLORERLAYOUT_H
#define SEGMENTATIONEXPLORERLAYOUT_H

#include "SegmentationExplorer.h"

#include <Core/Model/EspinaModel.h>

class SegmentationExplorer::Layout
{
protected:
  static const QString SEGMENTATION_MESSAGE;
  static const QString RECURSIVE_MESSAGE;
  static const QString MIXED_MESSAGE;

public:
  explicit Layout(EspinaModel *model): m_model(model) {}
  virtual ~Layout(){}

  virtual QAbstractItemModel *model()
  {return m_model; }
  virtual ModelItem *item(const QModelIndex &index) const {return indexPtr(index);}
  virtual QModelIndex index(ModelItem *item) const
  { return m_model->index(item); }
  virtual SegmentationList deletedSegmentations(QModelIndexList selection)
  { return SegmentationList(); }

protected:
  QModelIndexList indices(const QModelIndex &index, bool recursive=false);

protected:
  EspinaModel *m_model;
};

bool sortSegmentationLessThan(ModelItem *left, ModelItem *right);


#endif // SEGMENTATIONEXPLORERLAYOUT_H
