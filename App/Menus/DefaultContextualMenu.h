/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef DEFAULTCONTEXTUALMENU_H
#define DEFAULTCONTEXTUALMENU_H

#include <QModelIndex>
#include <GUI/Widgets/ContextualMenu.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

class QTreeView;
class QUndoStack;

namespace EspINA
{
  class ViewManager;

  class DefaultContextualMenu
  : public ContextualMenu
  {
    Q_OBJECT
    public:
      explicit DefaultContextualMenu(SegmentationAdapterList selection,
                                     ModelAdapterSPtr        model,
                                     ViewManagerSPtr         viewManager,
                                     QUndoStack             *undoStack,
                                     QWidget                *parent = 0);
      ~DefaultContextualMenu();

      virtual void setSelection(SelectionSPtr selection);

    private slots:
      void addNote();
      void changeSegmentationsCategory(const QModelIndex &index);
      void deleteSelectedSementations();
      void manageTags();
      void resetRootItem();
      void renameSegmentation();
      void displayVisualizationSettings();

    signals:
      void changeCategory(CategoryAdapterPtr);
      void deleteSegmentations();

    private:
      void createNoteEntry();
      void createChangeCategoryMenu();
      void createTagsEntry();
      void createSetLevelOfDetailEntry();
      void createRenameEntry();
      void createVisualizationEntry();
      void createDeleteEntry();

      QString dialogTitle() const;

    private:
      ModelAdapterSPtr m_model;
      ViewManagerSPtr m_viewManager;
      QUndoStack *m_undoStack;

      QTreeView *m_classification;
      SegmentationAdapterList m_segmentations;
  };
} // namespace EspINA

#endif // DEFAULTCONTEXTUALMENU_H
