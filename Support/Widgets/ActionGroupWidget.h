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

#ifndef ESPINA_SUPPORT_ACTION_GROUP_WIDGET_H
#define ESPINA_SUPPORT_ACTION_GROUP_WIDGET_H

#include "Support/EspinaSupport_Export.h"

// Qt
#include <QWidgetAction>

namespace ESPINA
{

  class EspinaSupport_EXPORT ActionGroupWidget
  : public QWidgetAction
  {
  public:
  	/* \brief ActionGroupWidget class constructor.
  	 * \param[in] parent, raw pointer of the QObject parent of this one.
  	 *
  	 */
    explicit ActionGroupWidget(QObject* parent);

    /* \brief Overrides QWidgetAction::createWidget().
     *
     */
    virtual QWidget* createWidget(QWidget* parent) override;

    /* \brief Adds a QAction to the widget.
     * \param[in] action, action to add.
     *
     */
    void add(QAction* action);

    /* \brief Removes a QAction from the widget.
     * \param[in] action, action to remove.
     *
     */
    void remove(QAction* action);

  private:
    /* \brief Helper method to create and add a button to the widget.
     *
     */
    void addActionButton(QAction* action);

  private:
    QList<QAction *> m_actions;
    QWidget         *m_widget;
  };
}

#endif // ESPINA_SUPPORT_ACTION_GROUP_WIDGET_H
