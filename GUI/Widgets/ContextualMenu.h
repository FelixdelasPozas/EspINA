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


#ifndef ESPINA_CONTEXTUAL_MENU_H
#define ESPINA_CONTEXTUAL_MENU_H

#include "EspinaGUI_Export.h"

#include <Core/EspinaTypes.h>
#include "GUI/Model/ViewItemAdapter.h"
#include <GUI/View/SelectableView.h>

#include <QMenu>

namespace EspINA
{
  class EspinaGUI_EXPORT ContextualMenu
  : public QMenu
  {
  public:
    explicit ContextualMenu(QWidget *parent = 0)
    : QMenu(parent) {}

    virtual void setSelection(SelectionSPtr selection) = 0;
  };

  using ContextualMenuSPtr = std::shared_ptr<ContextualMenu>;

} // namespace EspINA

#endif // ESPINA_CONTEXTUAL_MENU_H
