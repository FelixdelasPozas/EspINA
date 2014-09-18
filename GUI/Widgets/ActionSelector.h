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

#ifndef ESPINA_ACTION_SELECTOR_H
#define ESPINA_ACTION_SELECTOR_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QWidgetAction>

// C++
#include <memory>

class QMenu;

class ActionSelectorWidget;

class EspinaGUI_EXPORT ActionSelector
: public QWidgetAction
{
  Q_OBJECT
public:
  /* \brief ActionSelector class constructor.
   * \param[in] parent, raw pointer of the QObject parent of this one.
   *
   */
  explicit ActionSelector(QObject *parent = nullptr);

  /* \brief Overrides QWidgetAction::createWidget().
   *
   */
  virtual QWidget* createWidget(QWidget *parent);

  /* \brief Adds an action to the widget.
   *
   */
  void addAction(QAction *action);

  /* \brief Sets the default action for the widget's button.
   * \param[in] action, QAction raw pointer.
   *
   */
  void setDefaultAction(QAction *action);

  /* \brief Returns the current action of the button.
   *
   */
  QAction* getCurrentAction();

  /* \brief Returns the current action string.
   *
   */
  QString getCurrentActionAsQString();

  /* \brief Emits the cancellation signal.
   *
   */
  void cancel()
  {emit cancelAction();}

  /* \brief Shadows QAction::checked().
   *
   */
  bool isChecked();

  /* \brief Shadows QAction::setChecked().
   *
   */
  void setChecked(bool value);

  /* \brief Shadows QAction::setIcon().
   *
   */
  void setIcon(const QIcon &);

  /* \brief Shadows QAction::setEnabled().
   *
   */
  void setEnabled(bool);

  /* \brief Shadows QAction::isEnabled().
   *
   */
  bool isEnabled() const;

protected slots:
	/* \brief Triggers the given action.
	 * \param[in] action, raw pointer of the QAction to trigger.
	 *
	 */
  void actionTriggered(QAction *action);

  /* \brief Resets the UI when the action has been cancelled.
   *
   */
  void onActionCanceled();

  /* \brief Updates internal data when the destroy signal for the button has been emmited.
   *
   */
  void destroySignalEmmited();

signals:
  void cancelAction();
  void actionCanceled();
  void triggered(QAction *);

private:
  QList<QAction *>      m_actions;
  int                   m_defaultAction;
  ActionSelectorWidget *m_button;
  bool                  m_enabled;
  bool                  m_checked;
};

using ActionSelectorSPtr = std::shared_ptr<ActionSelector>;

#endif // ESPINA_ACTION_SELECTOR_H
