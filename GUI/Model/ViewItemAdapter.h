/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_VIEW_ITEM_ADAPTER_H
#define ESPINA_VIEW_ITEM_ADAPTER_H

#include "GUI/Model/ItemAdapter.h"
#include "OutputAdapter.h"

#include <Core/Analysis/Data.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/ViewItem.h>

namespace EspINA {

  class FilterAdapterInterface;
  using FilterAdapterSPtr = std::shared_ptr<FilterAdapterInterface>;

  class EspinaGUI_EXPORT ViewItemAdapter
  : public ItemAdapter
  {
  public:
    virtual ~ViewItemAdapter(){}

    bool isSelected() const
    { return m_isSelected; }

    void setSelected(bool value)
    { m_isSelected = value; }

    void setVisible(bool value)
    { m_isVisible = value; }

    bool isVisible() const
    { return m_isVisible; }

    FilterAdapterSPtr filter(){}
    const FilterAdapterSPtr filter() const{}

    /// Convenience method
    OutputAdapterSPtr output(){}
    /// Convenience method
    const OutputAdapterSPtr output() const{}

    DataSPtr get(Data::Type type){}

    const DataSPtr get(Data::Type type) const{}

//     /// Convenience method to access output's representations
//     RepresentationSList representations() const
//     { return output()->graphicalRepresentations(); }


//     /// Return whether item's volume has been modified or not after its creation
//     bool outputIsModified() { return m_outputIsModified; }

//   protected slots:
//     void onOutputModified() { m_outputIsModified = true; emit outputModified(); emit modified(this);}
// 
//   signals:
//     void outputModified();
// 
  protected:
    explicit ViewItemAdapter(ViewItemSPtr item)
    : m_viewItem{item} {}

  protected:
    ViewItemSPtr m_viewItem;

    bool m_isSelected;
    bool m_isVisible;
    bool m_outputIsModified; // sticky bit
  };

  using ViewItemAdapterPtr  = ViewItemAdapter *;
  using ViewItemAdapterList = ViewItemAdapterPtr;
} // namespace EspINA

#endif // ESPINA_VIEW_ITEM_ADAPTER_H
