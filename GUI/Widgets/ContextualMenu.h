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

#ifndef ESPINA_CONTEXTUAL_MENU_H
#define ESPINA_CONTEXTUAL_MENU_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Types.h>
#include "GUI/Model/ViewItemAdapter.h"
#include <GUI/View/SelectableView.h>

// Qt
#include <QMenu>

namespace ESPINA
{
  class EspinaGUI_EXPORT ContextualMenu
  : public QMenu
  {
  public:
  	/** \brief ContextualMenu class constructor.
  	 * \param[in] parent, raw pointer of the QWidget parent of this one.
  	 *
  	 */
    explicit ContextualMenu(QWidget *parent = nullptr)
    : QMenu{parent}
		{}

		/** \brief Sets the selection of the menu.
		 *
		 */
    virtual void setSelection(GUI::View::SelectionSPtr selection) = 0;
  };

  using ContextualMenuSPtr = std::shared_ptr<ContextualMenu>;

} // namespace ESPINA

#endif // ESPINA_CONTEXTUAL_MENU_H
