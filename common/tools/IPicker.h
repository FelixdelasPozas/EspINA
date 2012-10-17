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

#ifndef IPICKER_H
#define IPICKER_H

#include <QObject>

#include <QCursor>
#include <QPair>
#include <QPolygonF>
#include <QSet>
#include <QVector3D>

class PickableItem;
class EspinaRenderView;

/// Interface to handle selections
/// Classes that implement this interface have to specify
/// which selection method has to be used and which type of
/// products must be selected
class IPicker
: public QObject
{
  Q_OBJECT
public:
  typedef QString Tag;
  /// Pickable ModelItems identifiers
  static  const Tag SAMPLE;
  static  const Tag CHANNEL;
  static  const Tag SEGMENTATION;

  typedef QSet<Tag>              PickableItems;
  typedef QPolygonF               DisplayRegion;
  typedef QList<DisplayRegion>    DisplayRegionList;
  typedef QList<QVector3D>        WorldRegion;
  typedef QPair<WorldRegion,
                PickableItem *> PickedItem;
  typedef QList<PickedItem>       PickList;

public:
  explicit IPicker()
  : m_multiSelection(false)
  , m_cursor(Qt::CrossCursor)
  {}
  virtual ~IPicker(){};

  //TODO: 2012-10-17 Hacer abstracto
  virtual bool filterEvent(QEvent *e, EspinaRenderView *view=NULL);

  virtual QCursor cursor() const {return m_cursor;}
  virtual void setCursor(QCursor cursor);

  void setSelection(PickList msel);
  virtual void abortPick();

  /// The types of products which are requested for selection
  void setPickable(const Tag type, bool pick=true);
  /// Whether multiple products can be selected or not
  bool multiSelection() {return m_multiSelection;}
  void setMultiSelection(bool value) {m_multiSelection = value;}

signals:
  void itemsPicked(IPicker::PickList);
  void selectionAborted();

protected:
  PickableItems m_filters;
  bool          m_multiSelection;
  QCursor       m_cursor;
};

#endif // IPICKER_H
