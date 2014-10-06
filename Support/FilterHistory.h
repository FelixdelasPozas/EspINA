/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_FILTER_HISTORY_H
#define ESPINA_FILTER_HISTORY_H

#include <Support/ViewManager.h>

class QUndoStack;
class QWidget;

namespace ESPINA {

  class FilterHistory
  {
  public:
    virtual ~FilterHistory() {}

    /** \brief Return a widget to display or modify filter parameters
     *
     */
    virtual QWidget *createWidget(ViewManagerSPtr viewManager,
                                  QUndoStack      *undoStack) = 0;
  };

  using FilterDelegateSPtr = std::shared_ptr<FilterHistory>;

} // namespace ESPINA

#endif // ESPINA_FILTER_HISTORY_H
