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

#ifndef ESPINA_DOCK_WIDGET_H
#define ESPINA_DOCK_WIDGET_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QDockWidget>

class QUndoStack;

namespace ESPINA
{
  class EspinaSupport_EXPORT DockWidget
  : public QDockWidget
  {
    Q_OBJECT
  public:
    /** \brief DockWidget class constructor.
     * \param[in] parent, raw pointer of the parent widget of this one.
     *
     */
    explicit DockWidget(QWidget *parent = nullptr)
    : QDockWidget(parent)
    {}

    /** \brief DockWidget class virtual destructor.
     *
     */
    virtual ~DockWidget()
    {}

  public slots:
    /** \brief Resets the dock to its initial state and frees resources.
     *
     */
    virtual void reset() = 0;
  };
}

#endif // ESPINA_DOCK_WIDGET_H