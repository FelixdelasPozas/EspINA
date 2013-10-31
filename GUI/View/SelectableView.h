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

namespace EspINA
{

  /** \brief Interface for views displaying items whose selection state may change
   * 
   */
  class EspinaGUI_EXPORT SelectableView
  {
  public:
    using Selection = QList<ViewItemAdapterList>;

  public:
    virtual ~SelectableView(){}

    void setSelectionEnabled(bool value)
    { m_selectionEnabled = value; }

    bool selectionEnabled() const
    { return m_selectionEnabled; }

    virtual void updateRepresentations() = 0;

    virtual void updateRepresentations(ChannelAdapterList list) = 0;

    virtual void updateRepresentations(SegmentationAdapterList list) = 0;

    virtual void updateSelection() = 0;

  private:
    bool m_selectionEnabled;
  };

} // namespace EspINA

#endif // ESPINA_SELECTABLE_VIEW_H
