/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IPICKER_H
#define IPICKER_H

#include <QObject>

#include <QCursor>
#include <QPair>
#include <QPolygonF>
#include <QSet>
#include <QVector3D>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>

#include <Core/EspinaTypes.h>

namespace EspINA
{
  class EspinaRenderView;

  /// Interface to handle selections
  /// Classes that implement this interface have to specify
  /// which selection method has to be used and which type of
  /// products must be selected
  class ISelector
  : public QObject
  {
    Q_OBJECT
  public:
    typedef QString Tag;
    /// Pickable ModelItems identifiers
    static  const Tag SAMPLE;
    static  const Tag CHANNEL;
    static  const Tag SEGMENTATION;

    typedef QSet<Tag>                  PickableItems;
    typedef QPolygonF                  DisplayRegion;
    typedef QList<DisplayRegion>       DisplayRegionList;
    typedef vtkSmartPointer<vtkPoints> WorldRegion;
    typedef QPair<WorldRegion,
    PickableItemPtr>     PickedItem;
    typedef QList<PickedItem>          PickList;

  public:
    explicit ISelector()
    : m_multiSelection(false)
    , m_cursor(Qt::CrossCursor)
    {}
    virtual ~ISelector(){};

    virtual bool filterEvent(QEvent *e, EspinaRenderView *view=NULL) = 0;

    virtual QCursor cursor() const {return m_cursor;}
    virtual void setCursor(QCursor cursor);

    /// The types of products which are requested for selection
    void setPickable(const Tag type, bool pick=true);
    /// Whether multiple products can be selected or not
    bool multiSelection() {return m_multiSelection;}
    void setMultiSelection(bool value) {m_multiSelection = value;}

  signals:
    void itemsPicked(ISelector::PickList);

  protected:
    PickableItems m_filters;
    bool          m_multiSelection;
    QCursor       m_cursor;
  };

  typedef QSharedPointer<ISelector> IPickerSPtr;

} // namespace EspINA

#endif // IPICKER_H
