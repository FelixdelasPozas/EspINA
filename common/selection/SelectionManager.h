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
#include <QCursor>

class TaxonomyNode;
class Channel;
class EspinaWidget;
class QPoint;
class SelectableView;

/// Singleton instance to coordinate selections through different
/// components such as views and plugins
class SelectionManager
: public QObject
{
  Q_OBJECT
public:
  ~SelectionManager(){}

  /// Returns a SelectionManager singleton
  static SelectionManager *instance();

  // Active Elements: These are specified by the user to be used when
  // one element of the proper type is required
  void setActiveChannel(Channel *channel)
  { m_activeChannel=channel; }
  Channel *activeChannel()
  { return m_activeChannel; }
  void setActiveTaxonomy(TaxonomyNode *taxonomy)
  { m_activeTaxonomy = taxonomy; }
  TaxonomyNode *activeTaxonomy()
  { return m_activeTaxonomy; }

  // Delegates calls on active SelectionHandler
  void onMouseDown(const QPoint &pos, SelectableView *view) const;
  void onMouseMove(const QPoint &pos, SelectableView *view) const;
  void onMouseUp  (const QPoint &pos, SelectableView *view) const;
  bool filterEvent(QEvent *e, SelectableView *view=NULL) const;

  void setSelection(SelectionHandler::MultiSelection sel);
  SelectionHandler::MultiSelection selection() const
  { return m_selection; }
  const Nm *selectionCenter() const
  { return m_selectionCenter; }
  void setVOI(EspinaWidget *voi);
  EspinaWidget *voi() const {return m_voi;}

  QCursor cursor() const;

public slots:
  /// Register @sh as active Selection Handler
  void setSelectionHandler(SelectionHandler *sh);
  /// Unregister @sh as active Selection Handler
  void unsetSelectionHandler(SelectionHandler *sh);

signals:
  void activeChannelChanged(Channel *);
  void activeTaxonomyChanged(TaxonomyNode *);
  void selectionChanged(SelectionHandler::MultiSelection);

private:
  explicit SelectionManager();
  void computeSelectionCenter();

private:
  static SelectionManager *m_singleton;

  SelectionHandler *m_handler;
  EspinaWidget     *m_voi;

  Channel      *m_activeChannel;
  TaxonomyNode *m_activeTaxonomy;
  SelectionHandler::MultiSelection m_selection;
  Nm m_selectionCenter[3];
};

#endif // SELECTIONMANAGER_H
