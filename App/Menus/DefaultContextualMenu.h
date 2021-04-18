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
#include <Support/Context.h>

// Qt
#include <QModelIndex>

class QUndoStack;

namespace ESPINA
{
  class ViewManager;

  /** \class DefaultContextualMenu
   *  \brief Default context menu for segmentation explorer layouts.
   *
   */
  class DefaultContextualMenu
  : public ContextualMenu
  , private Support::WithContext
  {
    Q_OBJECT
    public:
      /** \brief DefaultContextualMenu class constructor.
       * \param[in] context
       *
       */
      explicit DefaultContextualMenu(SegmentationAdapterList selection,
                                     Support::Context       &context,
                                     QWidget                *parent = nullptr);

      /** \brief DefaultContextualMenu class destructor.
       *
       */
      ~DefaultContextualMenu()
      {};

      virtual void setSelection(GUI::View::SelectionSPtr selection) override
      {};

    private slots:
      /** \brief Adds/Modifies notes to the selected segmentations.
       *
       */
      void addNote();

      /** \brief Export selected segmentations to external format
       *
       */
      void exportSelectedSegmentations();

      /** \brief Removes the selected segmentations.
       *
       */
      void deleteSelectedSementations();

      /** \brief Adds/Modifies tags to the selected segmentations.
       *
       */
      void manageTags();

      /** \brief Renames selected segmentations.
       *
       */
      void renameSegmentation();

      /** \brief Renames a group of segmentations with the user given prefix.
       *
       */
      void renameSegmentationGroup();

      /** \brief Changes the color engine assigned to the selected segmentations.
       *
       */
      void changeSegmentationsColorEngine();

      /** \brief Exports the current segmentation to OBJ format in a file on disk.
       *
       */
      void exportSegmentationToOBJ();

      /** \brief Helper method to execute code on current selection. Shouldn't be used in production.
       *
       */
      void doFixes();

    signals:
      void renamedSegmentations();
      void deleteSegmentations();

    private:
      /** \brief Creates a "note" entry for the contextual menu.
       *
       */
      void createNoteEntry();

      /** \brief Creates a "tags" entry for the contextual menu.
       *
       */
      void createTagsEntry();

      /** \brief Creates a "rename" entry for the contextual menu.
       *
       */
      void createRenameEntry();

      /** \brief Creates a "rename group" entry for the contextual menu.
       *
       */
      void createGroupRenameEntry();

      /** \brief Export segmentations to external format
       *
       */
      void createExportEntry();

      /** \brief Creates a "delete" entry for the contextual menu.
       *
       */
      void createDeleteEntry();

      /** \brief Created "Custom coloring" entry for the contextual menu.
       *
       */
      void createColorEntry();

      /** \brief Helper method to add an entry to the menu to apply fixes to selected items. Not to be used in production,
       *  just to debug and fix.
       *
       */
      void createFixesEntry();

    private:
      SegmentationAdapterList m_segmentations;  /** selected segmentation list. */
  };
} // namespace ESPINA

#endif // DEFAULTCONTEXTUALMENU_H
