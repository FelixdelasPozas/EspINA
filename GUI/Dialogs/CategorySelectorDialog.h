/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_CATEGORY_SELECTOR_DIALOG_H
#define ESPINA_CATEGORY_SELECTOR_DIALOG_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/CategoryAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include "GUI/ui_CategorySelectorDialog.h"

// Qt
#include <QDialog>

namespace ESPINA {

  class EspinaGUI_EXPORT CategorySelectorDialog
  : public QDialog
  , private Ui::CategorySelectorDialog
  {
		public:
			/* \brief CategorySelectorDialog class constructor.
			 * \param[in] model, model adapter smart pointer.
			 * \param[in] parent, raw pointer of the parent of this dialog.
			 * \param[in] f, window flags.
			 */
			explicit CategorySelectorDialog(ModelAdapterSPtr model,
																			QWidget*         parent = nullptr,
																			Qt::WindowFlags  f = 0);

			/* \brief Returns the list of category adapter smart pointers.
			 *
			 */
			CategoryAdapterSList categories();

		private:
			ModelAdapterSPtr m_model;
  };
}

#endif // ESPINA_CATEGORY_SELECTOR_DIALOG_H
