/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/BinaryMask.hxx>
#include <GUI/Model/NeuroItemAdapter.h>
#include <GUI/View/EventHandler.h>

// Qt
#include <QObject>
#include <QCursor>
#include <QPair>
#include <QPolygonF>
#include <QSet>
#include <QVector3D>

namespace ESPINA
{
  class RenderView;

  /** \brief Interface to handle view events in order to select view items
   *
   */
  class EspinaGUI_EXPORT Selector
  : public EventHandler
  {
  Q_OBJECT
  public:
    using SelectionTag      = QString;
    using SelectionMask     = BinaryMaskSPtr<unsigned char>;
    using SelectionMaskSPtr = std::shared_ptr<BinaryMask<unsigned char>>;

    static const SelectionTag SAMPLE;
    static const SelectionTag CHANNEL;
    static const SelectionTag SEGMENTATION;

    using SelectionFlags = QSet<SelectionTag>;
    using SelectionItem  = QPair<SelectionMask, NeuroItemAdapterPtr>;
    using Selection      = QList<SelectionItem>;

  public:
    /** brief Selector class constructor.
     *
     */
    explicit Selector()
    : m_enabled{true}
    , m_multiSelection{false}
    {}

    /** brief Selector class virtual destructor.
     *
     */
    virtual ~Selector()
    {};

    /** brief Enables/Disables the specified selection tag.
     * \param[in] tag, tag to modify.
     * \param[in] selectable, true to select false otherwise.
     *
     */
    void setSelectionTag(const SelectionTag tag, bool selectable=true);

    /** brief Enables/Disables multiple item selection.
     * \param[in] enabled, true to enable multiple selection false otherwise.
     *
     */
    void setMultiSelection(bool enabled)
    { m_multiSelection = enabled; }

    /** brief Returns true if the multi-selection flag is true, false otherwise.
     *
     */
    bool multiSelection()
    { return m_multiSelection; }

    /** brief Enables/Disables the selector.
     * \param[in] value, true to enable, false otherwise.
     *
     */
    void setEnabled(bool value)
    { m_enabled = value; }

    /** brief Returns true if the selector is enabled, false otherwise.
     *
     */
    bool isEnabled() const
    {return m_enabled; }

  signals:
    void itemsSelected(Selector::Selection);
    void startUsingSelector();
    void stopUsingSelector();

  protected:
    bool           m_enabled;
    SelectionFlags m_flags;
    bool           m_multiSelection;
  };

  using SelectorSPtr = std::shared_ptr<Selector>;
} // namespace ESPINA

#endif // ESPINA_SELECTOR_H
