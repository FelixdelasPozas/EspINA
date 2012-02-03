/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#ifndef SEGMENTACTION_H
#define SEGMENTACTION_H

#include <QWidgetAction>

class QMenu;
class SelectionHandler;


class SegmentAction
: public QWidgetAction
{
  Q_OBJECT
public:
  explicit SegmentAction(QObject* parent);

  virtual QWidget* createWidget(QWidget* parent);

  void addSelector(QAction *action);
  void cancel() {emit cancelAction();}

protected slots:
  void actionTriggered(QAction *action);
  void onActionCanceled();

signals:
  void cancelAction();
  void actionCanceled();
  void triggered(QAction *);

private:
  SelectionHandler *m_selector;
  QList<QAction *>  m_actions;
};

#endif // SEGMENTACTION_H
