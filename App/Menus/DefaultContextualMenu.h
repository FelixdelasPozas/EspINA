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

// ESPINA
#include <GUI/Widgets/ContextualMenu.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QModelIndex>

class QTreeView;
class QUndoStack;

namespace ESPINA
{
  class ViewManager;

  class DefaultContextualMenu
  : public ContextualMenu
  {
    Q_OBJECT
    public:
			/** brief DefaultContextualMenu class constructor.
			 * \param[in] selection, list of segmentation adapters of the selected items.
			 * \param[in] model, model adapter smart pointer.
			 * \param[in] viewManager, view manager smart pointer.
			 * \param[in] undoStack, QUndoStack object raw pointer.
			 * \param[in] parent, parent QWidget raw pointer.
			 *
			 */
      explicit DefaultContextualMenu(SegmentationAdapterList selection,
                                     ModelAdapterSPtr        model,
                                     ViewManagerSPtr         viewManager,
                                     QUndoStack             *undoStack,
                                     QWidget                *parent = nullptr);

      /** brief DefaultContextualMenu class destructor.
       *
       */
      ~DefaultContextualMenu();

      /** brief Overrides ContextualMenu::setSelection().
       *
       */
      virtual void setSelection(SelectionSPtr selection) override;

    private slots:
			/** brief Adds/Modifies notes to the selected segmentations.
			 *
			 */
      void addNote();

      /** brief Changes the category of the selected segmentation.
       * \param[in] index, const QModelIndex referece of the item.
       */
      void changeSegmentationsCategory(const QModelIndex &index);

      /** brief Removes the selected segmentations.
       *
       */
      void deleteSelectedSementations();

      /** brief Adds/Modifies tags to the selected segmentations.
       *
       */
      void manageTags();

      /** brief Resets the root of the model.
       *
       */
      void resetRootItem();

      /** brief Renames selected segmentations.
       *
       */
      void renameSegmentation();

    signals:
      void changeCategory(CategoryAdapterPtr);
      void deleteSegmentations();

    private:
      /** brief Creates a "note" entry for the contextual menu.
       *
       */
      void createNoteEntry();

      /** brief Creates a "change category" entry for the contextual menu.
       *
       */
      void createChangeCategoryMenu();

      /** brief Creates a "tags" entry for the contextual menu.
       *
       */
      void createTagsEntry();

      /** brief Creates a "rename" entry for the contextual menu.
       *
       */
      void createRenameEntry();

      /** brief Creates a "delete" entry for the contextual menu.
       *
       */
      void createDeleteEntry();

      /** brief Helper method that generates a title for the current segmentation selection.
       *
       */
      QString dialogTitle() const;

    private:
      ModelAdapterSPtr m_model;
      ViewManagerSPtr m_viewManager;
      QUndoStack *m_undoStack;

      QTreeView *m_classification;
      SegmentationAdapterList m_segmentations;
  };
} // namespace ESPINA

#endif // DEFAULTCONTEXTUALMENU_H
