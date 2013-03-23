/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SEGMENTATIONCONTEXTUALMENU_H
#define SEGMENTATIONCONTEXTUALMENU_H

#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>

#include <QMenu>
#include <QModelIndex>

class QTreeView;
namespace EspINA
{
  class SegmentationContextualMenu
  : public QMenu
  {
    Q_OBJECT
  public:
    explicit SegmentationContextualMenu(SegmentationList selection,
                                        EspinaModel     *model,
                                        QUndoStack      *undoStack,
                                        ViewManager     *viewManager,
                                        QWidget         *parent = 0);
    void setSelection(SegmentationList list);

  private slots:
    void addNote();
    void changeSegmentationsTaxonomy(const QModelIndex &index);
    void deleteSelectedSementations();
    void changeFinalFlag();
    void manageTags();
    void resetRootItem();

  signals:
    void changeTaxonomy(TaxonomyElementPtr);
    void deleteSegmentations();
    void changeFinalNode(bool);

  private:
    void createAddNoteEntry();
    void createChangeTaxonomyMenu();
    void createManageTagsEntry();
    void createSetLevelOfDetailEntry();

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    QTreeView       *m_taxonomyList;
    QAction         *m_changeFinalNode;
    SegmentationList m_segmentations;
  };
} // namespace EspINA

#endif // SEGMENTATIONCONTEXTUALMENU_H
