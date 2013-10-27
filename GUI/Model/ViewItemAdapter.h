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

#ifndef ESPINA_VIEWITEM_H
#define ESPINA_VIEWITEM_H

#include "GUI/Model/ItemAdapter.h"

#include <Core/Analysis/Data.h>

namespace EspINA {

  class EspinaGUI_Export ViewItemAdapter
  : public ItemAdapter
  {
  public:
    virtual ~ViewItemAdapter();

    bool isSelected() const
    {return m_isSelected;}

    void setSelected(bool value)
    {m_isSelected = value;}

    FilterAdapterSPtr filter();
    const FilterSPtr filter() const;

    virtual const FilterOutputId outputId() const = 0;

    /// Convenience method
    OutputSPtr output();
    /// Convenience method
    const OutputSPtr output() const;

    DataSPtr get(Data::Type type);

    const DataSPtr get(Data::Type type) const;

    /// Convenience method to access output's representations
    GraphicalRepresentationSList representations() const
    { return output()->graphicalRepresentations(); }


    /// Return whether item's volume has been modified or not after its creation
    bool outputIsModified() { return m_outputIsModified; }

  protected slots:
    void onOutputModified() { m_outputIsModified = true; emit outputModified(); emit modified(this);}

  signals:
    void outputModified();

  protected:
    explicit ViewItemAdapter(ViewItemSPtr item);

  protected:
    ViewItemSPtr m_viewItem;

    bool m_isSelected;
    bool m_outputIsModified; // sticky bit
  };
} // namespace EspINA

#endif // ESPINA_VIEWITEM_H
