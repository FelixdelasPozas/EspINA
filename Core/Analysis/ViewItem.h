/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "Core/Analysis/NeuroItem.h"
#include "Input.h"

namespace ESPINA {

  class ViewItem
  : public QObject
  , public NeuroItem
  {
    Q_OBJECT
  public:
    explicit ViewItem(InputSPtr input);
    virtual ~ViewItem();

    InputSPtr asInput() const
    { return m_input; }

    FilterSPtr filter()
    { return m_input->filter(); }

    const FilterSPtr filter() const
    { return m_input->filter(); }

    OutputSPtr output(); //rename to input?
    const OutputSPtr output() const;

    Output::Id outputId() const
    { return m_input->output()->id(); }

    DataSPtr data(Data::Type type);
    const DataSPtr data(Data::Type type) const;

    void changeOutput(InputSPtr input);

    void changeOutput(FilterSPtr filter, Output::Id outputId);

    bool isOutputModified() const
    { return m_isOutputModified; }

    Bounds bounds() const
    { return output()->bounds(); }

  protected slots:
    void onOutputModified() 
    {
      m_isOutputModified = true;
      emit outputModified(); 
    }

  signals:
    void outputModified();

  private:
    InputSPtr m_input;
    bool      m_isOutputModified; // sticky bit
  };

  using ViewItemSPtr = std::shared_ptr<ViewItem>;
}

#endif // ESPINA_VIEWITEM_H
