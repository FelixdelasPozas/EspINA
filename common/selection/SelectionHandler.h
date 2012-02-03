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

#ifndef SELECTIONHANDLER_H
#define SELECTIONHANDLER_H

#include <QObject>

#include <processing/pqData.h>

#include <QStringList>
#include <QPolygon>
#include <QVector3D>
#include <QPair>
#include <QCursor>


class SelectableView;

/// Interface to handle selections
/// Classes that implement this interface have to specify
/// which selection method has to be used and which type of
/// products must be selected
class SelectionHandler
: public QObject
{
  Q_OBJECT
public:
  /// List of taxonomy ids which can be selected
  /// WARNING: Special EspINA_Sample taxonomy is used to refer to the sample itself
  static  const QString            EspINA_Channel;
  typedef QStringList              SelectionFilters;
  typedef QList<QPolygon>          ViewRegions;
  typedef QList<QVector3D>         VtkRegion;
  typedef QList<VtkRegion>         VtkRegions;
  typedef QPair<VtkRegion, pqData> Selelection;
  typedef QList<Selelection>       MultiSelection;

public:
  explicit SelectionHandler(SelectionHandler *succesor=NULL)
  : m_multiSelection(false)
  , m_succesor(succesor)
  , m_cursor(Qt::CrossCursor)
  {}
  virtual ~SelectionHandler(){};

  virtual bool filterEvent(QEvent *e, SelectableView *view=NULL);

  virtual QCursor cursor() {return m_cursor;}
  virtual void setCursor(QCursor cursor) {m_cursor = cursor;}


  void setSelection(SelectionHandler::MultiSelection msel);
  void abortSelection();

  /// The types of products which are requested for selection
  void setSelectable(QString type, bool sel=true);
  /// Whether multiple products can be selected or not
  bool multiSelection() {return m_multiSelection;}
  void setMultiSelection(bool value) {m_multiSelection = value;}

signals:
  void selectionChanged(SelectionHandler::MultiSelection);
  void selectionAborted();

protected:
  SelectionFilters  m_filters;
  bool              m_multiSelection;
  SelectionHandler *m_succesor;
  QCursor	    m_cursor;
};

#endif // SELECTIONHANDLER_H
