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

#include "Core/Analysis/Output.h"
#include "Core/Analysis/NeuroItem.h"

namespace EspINA {

  class ViewItem
  : public QObject
  , public NeuroItem
  {
    Q_OBJECT
  public:
    explicit ViewItem(FilterSPtr filter, const Output::Id output);
    virtual ~ViewItem();

    FilterSPtr filter()
    { return m_filter; }

    const FilterSPtr filter() const
    { return m_filter; }

    OutputSPtr output();
    const OutputSPtr output() const;

    Output::Id outputId() const
    { return m_outputId; }

    DataSPtr data(Data::Type type);
    const DataSPtr data(Data::Type type) const;

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
    FilterSPtr m_filter;
    Output::Id m_outputId;
    bool       m_isOutputModified; // sticky bit
  };

  using ViewItemSPtr = std::shared_ptr<ViewItem>;
}

#endif // ESPINA_VIEWITEM_H
