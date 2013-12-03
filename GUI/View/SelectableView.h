/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_SELECTABLE_VIEW_H
#define ESPINA_SELECTABLE_VIEW_H

#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include "Selection.h"

namespace EspINA
{

  /** \brief Interface for views displaying items whose selection state may change
   * 
   */
  class EspinaGUI_EXPORT SelectableView
  {
  public:
    SelectableView() 
    : m_selectionEnabled(true)
    , m_selection(new Selection()) {}

    virtual ~SelectableView(){}

    void setSelectionEnabled(bool value)
    { m_selectionEnabled = value; }

    bool selectionEnabled() const
    { return m_selectionEnabled; }

    void setSharedSelection(SelectionSPtr selection)
    { m_selection = selection; onSelectionSet(selection); }

    SelectionSPtr currentSelection() const
    { return m_selection; }

    virtual void updateRepresentations() = 0;

    virtual void updateRepresentations(ChannelAdapterList list) = 0;

    virtual void updateRepresentations(SegmentationAdapterList list) = 0;

  protected:
    virtual void onSelectionSet(SelectionSPtr selection) = 0;

  private:
    bool          m_selectionEnabled;
    SelectionSPtr m_selection;
  };

  using SelectableViewPtr  = SelectableView*;
  using SelectableViewSPtr = std::shared_ptr<SelectableView>;

} // namespace EspINA

#endif // ESPINA_SELECTABLE_VIEW_H
