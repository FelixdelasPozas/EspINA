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

#ifndef ESPINA_CATEGORY_SELECTOR_H
#define ESPINA_CATEGORY_SELECTOR_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Widgets/QComboTreeView.h>

// Qt
#include <QLabel>
#include <QSpinBox>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {

      class EspinaGUI_EXPORT CategorySelector
      : public QComboTreeView
      {
        Q_OBJECT
      public:
        /** \brief CategorySelector class constructor.
         * \param[in] model model adapter smart pointer.
         * \param[in] parent of the widget
         *
         */
        explicit CategorySelector(ModelAdapterSPtr model,
                                  QWidget         *parent = nullptr);

        /** \brief Selects the given category in the widget.
         * \param[in] category smart pointer of the category adapter to select.
         *
         */
        void selectCategory(CategoryAdapterSPtr category);

        /** \brief Returns the categoty selected in the widget.
         *
         */
        CategoryAdapterSPtr selectedCategory();

      signals:
        void categoryChanged(CategoryAdapterSPtr);

      private slots:
        /** \brief Signals the change of category in the widget.
         * \param[in] index, model index of the new category.
         *
         */
        void categorySelected(const QModelIndex& index);

        /** \brief Invalidates the state of all the objects in the selector.
         *
         */
        void invalidateState();

        /** \brief Resets the model of all the objects in the selector.
         *
         */
        void resetRootItem();

      private:
        ModelAdapterSPtr    m_model;
        CategoryAdapterSPtr m_selectedCategory;
      };

    }
  }
} // namespace ESPINA

#endif // ESPINA_CATEGORY_SELECTOR_H
