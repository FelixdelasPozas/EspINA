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
#include "OutputAdapter.h"

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

    void setRepresentationFactory(RepresentationFactorySPtr factory)
    { m_factory = factory; }

    virtual void update() = 0;//TODO Copy adapted filter interface

    OutputAdapterSPtr output(Output::Id id)
    { return m_outputs.at(id); }

  protected:
    virtual FilterSPtr adaptedFilter() = 0;
    virtual OutputSPtr adaptedOutput(Output::Id id) = 0;

    RepresentationFactorySPtr m_factory;
    OutputAdapterSList        m_outputs;

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

    virtual void update()
    {
      m_outputs.clear();
      m_filter->update();
      for(int i = 0; i < m_filter->numberOfOutputs(); ++i)
      {
        m_outputs << OutputAdapterSPtr{new OutputAdapter(m_filter->output(i), m_factory)};
      }
    }

  protected:
    virtual FilterSPtr adaptedFilter()
    { return m_filter; }

    virtual OutputSPtr adaptedOutput(Output::Id id)
    { return m_filter->output(id); }

  private:
    FilterAdapter(std::shared_ptr<T> filter)
    : m_filter{filter}{}

    std::shared_ptr<T> m_filter;

    friend class ModelFactory;
  };

  using FilterAdapterSPtr  = std::shared_ptr<FilterAdapterInterface>;
  using FilterAdapterSList = QList<FilterAdapterSPtr>;
}

#endif // ESPINA_FILTERADAPTER_H
