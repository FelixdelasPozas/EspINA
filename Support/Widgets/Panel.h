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
#include <Support/Context.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QDockWidget>

class QPushButton;
class QUndoStack;

namespace ESPINA
{
  /** \class Panel
   * \brief EspINA main window panel base class.
   *
   */
  class EspinaSupport_EXPORT Panel
  : public QDockWidget
  , protected Support::WithContext
  {
    Q_OBJECT
  public:
    /** \brief Panel class constructor.
     * \param[in] context application context.
     * \param[in] parent raw pointer of the parent widget of this one.
     *
     */
    explicit Panel(Support::Context &context, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget());

    /** \brief Panel class constructor.
     * \param[in] title panel title.
     * \param[in] context application context.
     * \param[in] parent raw pointer of the parent widget of this one.
     *
     */
    explicit Panel(const QString& title, Support::Context &context, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget());

    /** \brief DockWidget class virtual destructor.
     *
     */
    virtual ~Panel()
    {}

    /** \brief Helper method to create a button with the specified icon and tooltip with
     * the characteristics of a dock button.
     * \param[in] icon button icon.
     * \param[in] tooltip button tooltip.
     *
     */
    static QPushButton *createDockButton(const QString &icon, const QString &tooltip);

    virtual void showEvent(QShowEvent *event);

    virtual void hideEvent(QHideEvent *event);

    /** \brief Aborts the current operation of the panel (must reset any panel tool to initial state).
     *
     */
    virtual void abortOperation() {};

  public slots:
    /** \brief Resets the dock to its initial state and frees resources.
     *
     */
    virtual void reset() {};

  signals:
    void dockShown(bool visible);
  };
}

#endif // ESPINA_DOCK_WIDGET_H
