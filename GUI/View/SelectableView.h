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

#ifndef ESPINA_SELECTABLE_VIEW_H
#define ESPINA_SELECTABLE_VIEW_H

// ESPINA
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/View/ViewState.h>
#include "Selection.h"

namespace ESPINA
{
  /** \class SelectableView
   * \brief Interface for views displaying items whose selection state may change.
   *
   */
  class EspinaGUI_EXPORT SelectableView
  {
  public:
    /** \brief SelectableView class constructor.
     *
     */
    SelectableView(GUI::View::ViewState &viewState)
    : m_selection{viewState.selection()}
    {}

    /** \brief SelectableView class virtual destructor.
     *
     */
    virtual ~SelectableView()
    {}

    /** \brief Returns the view's current selection.
     *
     */
    GUI::View::SelectionSPtr currentSelection() const
    { return m_selection; }

  private:
    GUI::View::SelectionSPtr m_selection;
  };

  using SelectableViewPtr  = SelectableView*;
  using SelectableViewSPtr = std::shared_ptr<SelectableView>;

} // namespace ESPINA

#endif // ESPINA_SELECTABLE_VIEW_H
