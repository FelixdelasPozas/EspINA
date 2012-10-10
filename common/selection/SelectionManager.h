/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <QObject>

#include "SelectionHandler.h"
#include <EspinaTypes.h>
#include <QCursor>

class EspinaRenderView;
class TaxonomyElement;
class Channel;
class EspinaWidget;
class QPoint;
class EspinaRenderView;

/// Singleton instance to coordinate selections through different
/// components such as views and plugins
class SelectionManager
: public QObject
{
  Q_OBJECT
public:
  typedef QList<PickableItem *> Selection;
  ~SelectionManager(){}

  /// Returns a SelectionManager singleton
  //static SelectionManager *instance();

  // Delegates calls on active SelectionHandler
  void onMouseDown(const QPoint &pos, EspinaRenderView *view) const;
  void onMouseMove(const QPoint &pos, EspinaRenderView *view) const;
  void onMouseUp  (const QPoint &pos, EspinaRenderView *view) const;
  bool filterEvent(QEvent *e, EspinaRenderView *view=NULL) const;

  void setSelection(Selection selection);
  Selection selection() const
  { return m_selection; }
  const Nm *selectionCenter() const
  { return m_selectionCenter; }
  void setVOI(EspinaWidget *voi);
  EspinaWidget *voi() const {return m_voi;}

  QCursor cursor() const;

public slots:
  /// Register @sh as active Selection Handler
  void setSelectionHandler(IPicker *sh);
  /// Unregister @sh as active Selection Handler
  void unsetSelectionHandler(IPicker *sh);

signals:
  void activeChannelChanged(Channel *);
  void activeTaxonomyChanged(TaxonomyElement *);
  void selectionChanged(SelectionManager::Selection);

private:
  explicit SelectionManager();
  void computeSelectionCenter();

private:
  static SelectionManager *m_singleton;

  IPicker *m_handler;
  EspinaWidget     *m_voi;

  Selection     m_selection;
  Nm            m_selectionCenter[3];
};

#endif // SELECTIONMANAGER_H
