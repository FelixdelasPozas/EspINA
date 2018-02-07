/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_INFORMATION_SELECTOR_H
#define ESPINA_INFORMATION_SELECTOR_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Analysis/Segmentation.h>
#include <GUI/ModelFactory.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QDialog>
#include <QStringListModel>

class QTreeWidgetItem;
namespace ESPINA
{
  namespace GUI
  {
    /** \class InformationSelector
     * \brief Dialog to select a group of information tags from the available group of tags.
     *
     */
    class EspinaGUI_EXPORT InformationSelector
    : public QDialog
    {
        Q_OBJECT

      public:
        using GroupedInfo = QMap<QString, QStringList>;

      public:
        /** \brief InformationSelector class constructor.
         * \param[in] availableGroups map of available title-tags groups.
         * \param[in] selection map of checked title-tags groups.
         * \param[in] title title of the dialog.
         * \param[in] exclusive true to make the item selecion exclusive and false otherwise.
         * \param[in] parent raw pointer of the QWidget parent of this one.
         *
         */
        explicit InformationSelector(const GroupedInfo &availableGroups,
                                     GroupedInfo       &selection,
                                     const QString     &title,
                                     const bool         exclusive,
                                     QWidget           *parent = GUI::DefaultDialogs::defaultParentWidget());

        /** \brief InformationSelector class virtual destructor.
         *
         */
        virtual ~InformationSelector();

      protected slots:
        void accept();

      private slots:
        /** \brief Updates the state of the tree widget.
         * \param[in] item tree item.
         * \param[in] column column of the item.
         *
         */
        void onItemClicked(QTreeWidgetItem *item, int column);

      private:
        /** \brief Updates the state of the tree widget.
         * \param[in] item tree item.
         * \param[in] column column of the item.
         * \param[in] updateParent true to update the parent state, false otherwise.
         *
         */
        void updateCheckState(QTreeWidgetItem *item, int column, bool updateParent = true);

        /** \brief De-selects all the items.
         *
         */
        void unselectItems();

      private:
        class UI;

      private:
        UI          *m_gui;       /** chessire cat GUI implementation.                              */
        bool         m_exclusive; /** true if the selection is exclusive (only one item selection). */
        GroupedInfo &m_selection; /** selected tag group.                                           */
    };

    InformationSelector::GroupedInfo EspinaGUI_EXPORT availableInformation(ModelFactorySPtr factory);

    InformationSelector::GroupedInfo EspinaGUI_EXPORT availableInformation(SegmentationAdapterList segmentations, ModelFactorySPtr factory);
  }
}

#endif // ESPINA_INFORMATION_SELECTOR_H
