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

#ifndef ESPINA_SELECTOR_H
#define ESPINA_SELECTOR_H

#include "EspinaGUI_Export.h"

#include <QObject>

#include <QCursor>
#include <QPair>
#include <QPolygonF>
#include <QSet>
#include <QVector3D>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>

#include <Core/EspinaTypes.h>
#include <Core/Utils/BinaryMask.h>
#include <GUI/Model/ViewItemAdapter.h>

namespace EspINA
{
  class RenderView;

  /** \brief Interface to handle view events in order to select view items
   *
   */
  class EspinaGUI_EXPORT Selector
  : public QObject
  {
    Q_OBJECT
  public:
    using SelectionTag      = QString;
    using SelectionMask     = BinaryMask<unsigned char>;
    using SelectionMaskSPtr = std::shared_ptr<BinaryMask<unsigned char>>;

    static const SelectionTag SAMPLE;
    static const SelectionTag CHANNEL;
    static const SelectionTag SEGMENTATION;

    // DEPRECATED
    typedef QPolygonF                  DisplayRegion;
    typedef QList<DisplayRegion>       DisplayRegionList;
    typedef vtkSmartPointer<vtkPoints> WorldRegion;
    typedef QPair<WorldRegion, ViewItemAdapterPtr>  SelectedItem;
    typedef QList<SelectedItem>        SelectionList;

    using SelectionFlags = QSet<SelectionTag>;
    using SelectionItem  = QPair<SelectionMask, ViewItemAdapterPtr>;
    using Selection      = QList<SelectionItem>;

  public:
    explicit Selector()
    : m_multiSelection(false)
    , m_cursor(Qt::CrossCursor)
    {}

    virtual ~Selector(){};

    virtual bool filterEvent(QEvent *e, RenderView *view=nullptr) = 0;

    virtual QCursor cursor() const
    {return m_cursor;}

    virtual void setCursor(const QCursor& cursor);

    void setSelectionTag(const SelectionTag tag, bool selectable=true);

    void setMultiSelection(bool enabled)
    {m_multiSelection = enabled;}

    bool multiSelection()
    {return m_multiSelection;}

  signals:
    void itemsSelected(Selector::SelectionList);

    void itemsSelected(Selector::Selection);

  protected:
    SelectionFlags m_flags;
    bool           m_multiSelection;
    QCursor        m_cursor;
  };

  using SelectorSPtr = std::shared_ptr<Selector>;

} // namespace EspINA

#endif // ESPINA_SELECTOR_H
