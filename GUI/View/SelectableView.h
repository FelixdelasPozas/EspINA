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
  	/* \brief SelectableView class constructor.
  	 *
  	 */
    SelectableView()
    : m_selectionEnabled{true}
    , m_selection{new Selection()}
    {}

    /* \brief SelectableView class virtual destructor.
     *
     */
    virtual ~SelectableView()
    {}

    /* \brief Enables/disables the possibility to select items in the view.
     * \param[in] value, true to enable selection false otherwise.
     *
     */
    void setSelectionEnabled(bool value)
    { m_selectionEnabled = value; }

    /* \brief Returns true if the user can select items in the view.
     *
     */
    bool selectionEnabled() const
    { return m_selectionEnabled; }

    /* \brief Sets the selection of the view to the given one.
     * \param[in] selection, selection smart pointer.
     */
    void setSharedSelection(SelectionSPtr selection)
    { m_selection = selection; onSelectionSet(selection); }

    /* \brief Returns the view's current selection.
     *
     */
    SelectionSPtr currentSelection() const
    { return m_selection; }

    /* \brief Updates all the representations of the view.
     *
     */
    virtual void updateRepresentations() = 0;

    /* \brief Updates the representations of the channels in the given list.
     * \param[in] list, list of channel adapter raw pointers.
     *
     */
    virtual void updateRepresentations(ChannelAdapterList list) = 0;

    /* \brief Updates the representations of the segmentations in the given list.
     * \param[in] list, list of segmentation adapter raw pointers.
     *
     */
    virtual void updateRepresentations(SegmentationAdapterList list) = 0;

  protected:
    /* \brief Updates the view when the selection changes.
     * \param[in] selection, new selection.
     */
    virtual void onSelectionSet(SelectionSPtr selection) = 0;

  private:
    bool          m_selectionEnabled;
    SelectionSPtr m_selection;
  };

  using SelectableViewPtr  = SelectableView*;
  using SelectableViewSPtr = std::shared_ptr<SelectableView>;

} // namespace ESPINA

#endif // ESPINA_SELECTABLE_VIEW_H
