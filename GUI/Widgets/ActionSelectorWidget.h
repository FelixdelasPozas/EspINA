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

#include <QToolButton>

class EspinaGUI_EXPORT ActionSelectorWidget
: public QToolButton
{
  Q_OBJECT
public:
  explicit ActionSelectorWidget(QWidget* parent = 0);

  void addAction(QAction *action);
  void setButtonAction(QAction *action);
  QAction* getButtonAction();

public slots:
  void cancelAction();

protected slots:
  void triggerAction(bool trigger);
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
