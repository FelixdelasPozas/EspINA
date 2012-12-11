/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef SEGMENTATIONCONTEXTUALMENU_H
#define SEGMENTATIONCONTEXTUALMENU_H

#include <Core/EspinaTypes.h>

#include <QMenu>
#include <QModelIndex>

class EspinaModel;
class TaxonomyElement;


class SegmentationContextualMenu
: public QMenu
{
  Q_OBJECT
public:
  explicit SegmentationContextualMenu(EspinaModel *model, SegmentationList selection, QWidget* parent = 0);
  void setSelection(SegmentationList list);

private slots:
  void changeTaxonomyClicked(const QModelIndex &index);
  void deleteSementationsClicked();
  void changeFinalFlag();

signals:
  void changeTaxonomy(TaxonomyElement *);
  void deleteSegmentations();
  void changeFinalNode(bool);

private:
  QAction *m_changeFinalNode;
  SegmentationList m_segmentations;
};

#endif // SEGMENTATIONCONTEXTUALMENU_H