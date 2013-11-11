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
#include "FilterAdapter.h"

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

    FilterAdapterSPtr filter()
    { return m_filter; }
    const FilterAdapterSPtr filter() const
    { return m_filter; }

    /// Convenience method
    OutputAdapterSPtr output()
    { return m_filter->output(m_viewItem->outputId()); }

    /// Convenience method
    const OutputAdapterSPtr output() const
    { return m_filter->output(m_viewItem->outputId()); }

    DataSPtr get(Data::Type type)
    { return m_viewItem->data(type); }

    const DataSPtr get(Data::Type type) const
    { return m_viewItem->data(type); }

    RepresentationSPtr representation(Representation::Type representation) const;
    
    RepresentationSList representations() const
    { return m_representations.values(); }
    
    RepresentationTypeList representationTypes() const;
    
    void setRepresentationFactory(RepresentationFactorySPtr factory)
    { m_factory = factory; }

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
    explicit ViewItemAdapter(FilterAdapterSPtr filter, ViewItemSPtr item)
    : m_filter{filter}
    , m_viewItem{item}
    , m_isSelected{false}
    , m_isVisible{true}
    , m_outputIsModified{false}{}

  protected:
    FilterAdapterSPtr m_filter;
    ViewItemSPtr      m_viewItem;

    RepresentationFactorySPtr m_factory;
    mutable QMap<Representation::Type, RepresentationSPtr> m_representations;

    bool m_isSelected;
    bool m_isVisible;
    bool m_outputIsModified; // sticky bit
  };

  using ViewItemAdapterPtr  = ViewItemAdapter *;
  using ViewItemAdapterList = ViewItemAdapterPtr;
  using ViewItemAdapterSPtr = std::shared_ptr<ViewItemAdapter>;
} // namespace EspINA

#endif // ESPINA_VIEW_ITEM_ADAPTER_H
