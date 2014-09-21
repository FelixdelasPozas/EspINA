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

#ifndef ESPINA_ACTION_SELECTOR_WIDGET_H
#define ESPINA_ACTION_SELECTOR_WIDGET_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QToolButton>

class EspinaGUI_EXPORT ActionSelectorWidget
: public QToolButton
{
  Q_OBJECT
public:
  /** \brief ActionSelectorWidget class constructor.
   * \param[in] parent, raw pointer of the QWidget parent of this one.
   *
   */
  explicit ActionSelectorWidget(QWidget* parent = nullptr);

  /** \brief Shadows QWidget::addAction().
   *
   */
  void addAction(QAction *action);

  /** \brief Sets the button action to the specified action.
   * \param[in] action, QAction raw pointer.
   *
   */
  void setButtonAction(QAction *action);

  /** \brief Returns the current action of the button.
   *
   */
  QAction* getButtonAction();

public slots:
	/** \brief Unchecks the button.
	 *
	 */
  void cancelAction();

protected slots:
	/** \brief Emits the trigger signal or cancels the action depending on the parameter value.
	 * \paran[in] trigger, true to trigger the current action, false to cancel the action.
	 *
	 * If the current action is not checkable the action is always triggered.
	 *
	 */
  void triggerAction(bool trigger);

  /** \brief Changes the current action with the specified one.
   * \param[in] action, raw pointer of the new QAction.
   */
  void changeAction(QAction *action);

signals:
  /// Actions are triggered only if button is checked
  void actionTriggered(QAction *action);
  void actionCanceled();

protected:
  QMenu   *m_actions;
  QAction *m_selectedAction;
};

#endif // ESPINA_ACTION_SELECTOR_WIDGET_H
