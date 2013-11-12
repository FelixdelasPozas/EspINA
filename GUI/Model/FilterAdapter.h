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

#ifndef ESPINA_FILTERADAPTER_H
#define ESPINA_FILTERADAPTER_H

#include <Core/Analysis/Filter.h>

namespace EspINA {

  class FilterInspector;
  using FilterInspectorSPtr = std::shared_ptr<FilterInspector>;

  class Representation;
  using RepresentationSPtr = std::shared_ptr<Representation>;

  class OutputAdapter;
  using OutputAdapterSPtr  = std::shared_ptr<OutputAdapter>;
  using OutputAdapterSList = QList<OutputAdapterSPtr>;

  class RepresentationFactory;
  using RepresentationFactorySPtr = std::shared_ptr<RepresentationFactory>;

  class FilterAdapterInterface
  {
  public:
    virtual ~FilterAdapterInterface(){}

    void setFilterInspector(FilterInspectorSPtr inspector)
    { m_inspector = inspector; }

    FilterInspectorSPtr filterInspector()
    { return m_inspector; }
    virtual void update(Output::Id id) = 0;
    virtual void update() = 0;//TODO Copy adapted filter interface

    virtual OutputSPtr output(Output::Id id) = 0;

  protected:
    virtual FilterSPtr adaptedFilter() = 0;

  private:
    FilterInspectorSPtr m_inspector;

    friend class ModelFactory;
  };

  template<class T>
  class FilterAdapter
  : public FilterAdapterInterface
  {
  public:
    std::shared_ptr<T> get()
    { return m_filter; }

    virtual void update(Output::Id id)
    { m_filter->update(id); }

    virtual void update()
    { m_filter->update(); }

    virtual OutputSPtr output(Output::Id id)
    { return m_filter->output(id); }

  protected:
    virtual FilterSPtr adaptedFilter()
    { return m_filter; }

  private:
    FilterAdapter(std::shared_ptr<T> filter)
    : m_filter{filter}{}

    std::shared_ptr<T> m_filter;

    friend class ModelFactory;
    friend class ModelAdapter;
  };

  using FilterAdapterSPtr  = std::shared_ptr<FilterAdapterInterface>;
  using FilterAdapterSList = QList<FilterAdapterSPtr>;
}

#endif // ESPINA_FILTERADAPTER_H
