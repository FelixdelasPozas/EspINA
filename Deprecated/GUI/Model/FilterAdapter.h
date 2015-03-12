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

#ifndef ESPINA_FILTERADAPTER_H
#define ESPINA_FILTERADAPTER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>

#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Output.h>
#include <Core/MultiTasking/Task.h>
#include <GUI/Model/FilterDelegate.h>

namespace ESPINA {

  class Representation;
  using RepresentationSPtr = std::shared_ptr<Representation>;

  class OutputAdapter;
  using OutputAdapterSPtr  = std::shared_ptr<OutputAdapter>;
  using OutputAdapterSList = QList<OutputAdapterSPtr>;

  class RepresentationFactory;
  using RepresentationFactorySPtr = std::shared_ptr<RepresentationFactory>;

  class EspinaGUI_EXPORT FilterAdapterBase
  : public QObject
  {
    Q_OBJECT
  public:
    /** \brief FilterAdapterInterface class virtual destructor.
     *
     */
    virtual ~FilterAdapterBase()
    {}

    /** \brief Returns a smart pointer to the adapted filter
     *
     */
    virtual FilterSPtr genericFilter() const = 0;

    /** \brief Returns the type of the adapted filter base class
     *
     */
    virtual Filter::Type type() const = 0;

    /** \brief Sets the filter delegate for the filter.
     *
     * Filter delegates can be used to update filter parameters or to
     * display filter information
     * \param[in] delegate filter inspector smart pointer.
     *
     */
    void setFilterDelegate(FilterDelegateSPtr delegate)
    { m_delegate = delegate; }

    /** \brief Returns the filter delegate smart pointer.
     *
     */
    FilterDelegateSPtr filterDelegate()
    { return m_delegate; }

    /** \brief Returns true if the filter has been aborted.
     *
     */
    virtual bool isAborted() const = 0;

    /** \brief Returns true if the filter has finished its execution.
     *
     */
    virtual bool hasFinished() const = 0;

    /** \brief Submits the filter for execution.
     *
     * If the filter hasn't an assigned scheduler the execution will block the main thread.
     * Otherwise the filter will execute in it's own thread.
     *
     */
    virtual void submit() = 0;

    /** \brief Updates the specified output.
     * \param[in] id output id.
     *
     */
    virtual void update(Output::Id id) = 0;

    /** \brief Updates all the outputs of the filter.
     *
     * TODO Copy adapted filter interface
     *
     */
    virtual void update() = 0;

    /** \brief Returns the specified output smart pointer.
     * \param[in] id output id.
     *
     */
    virtual OutputSPtr output(Output::Id id) = 0;

    /** \brief Returns the number of outputs of the filter.
     *
     */
    virtual unsigned int numberOfOutputs() const = 0;

  signals:
    void progress(int);
    void resumed();
    void paused();
    void finished();

  protected:
    /** \brief Returns the adapted filter smart pointer.
     *
     */
    virtual FilterSPtr adaptedFilter() const = 0;

  private:
    FilterDelegateSPtr m_delegate;

    friend class ModelFactory;
  };

  template<class T>
  class FilterAdapter
  : public FilterAdapterBase
  {
  public:
    /** \brief Returns the smart pointer of the filter.
     *
     * NOTE: Change method name to filter?
     */
    std::shared_ptr<T> get()
    { return m_filter; }

    virtual FilterSPtr genericFilter() const
    { return m_filter; }

    /** \brief Returns the type of the filter
     *
     */
    virtual Filter::Type type() const
    { return m_filter->type(); }

    /** \brief Implements FilterAdapterInterface::isAborted() const.
     *
     */
    virtual bool isAborted() const
    { return m_filter->isAborted(); }

    /** \brief Implements FilterAdapterInterface::hasFinished() const.
     *
     */
    virtual bool hasFinished() const
    { return m_filter->hasFinished(); }

    /** \brief Implements FilterAdapterInterface::submit().
     *
     */
    virtual void submit()
    { Task::submit(m_filter); }

    /** \brief Implements FilterAdapterInterface::update(id).
     *
     */
    virtual void update(Output::Id id)
    { m_filter->update(id); }

    /** \brief Implements FilterAdapterInterface::update().
     *
     */
    virtual void update()
    { m_filter->update(); }

    /** \brief Implements FilterAdapterInterface::output(id).
     *
     */
    virtual OutputSPtr output(Output::Id id)
    { return m_filter->output(id); }

    /** \brief Implements FilterAdapterInterface::numberOfOutputs().
     *
     */
    virtual unsigned int numberOfOutputs() const
    { return m_filter->numberOfOutputs(); }

  protected:
    /** \brief Implements FilterAdapterInterface::adaptedFilter() const.
     *
     */
    virtual FilterSPtr adaptedFilter() const
    { return m_filter; }

  private:
    /** \brief FilterAdapter class private constructor.
     *
     */
    FilterAdapter(std::shared_ptr<T> filter)
    : m_filter{filter}
    {
      connect(m_filter.get(), SIGNAL(progress(int)),
              this, SIGNAL(progress(int)));
      connect(m_filter.get(), SIGNAL(resumed()),
              this, SIGNAL(resumed()));
      connect(m_filter.get(), SIGNAL(paused()),
              this, SIGNAL(paused()));
      connect(m_filter.get(), SIGNAL(finished()),
              this, SIGNAL(finished()));
    }

    std::shared_ptr<T> m_filter;

    friend class ModelFactory;
    friend class ModelAdapter;
  };

  using FilterAdapterBasePtr   = FilterAdapterBase *;
  using FilterAdapterBaseSPtr  = std::shared_ptr<FilterAdapterBase>;
  using FilterAdapterBaseSList = QList<FilterAdapterBaseSPtr>;

  template<class T>
  std::shared_ptr<T> adaptedFilter(FilterAdapterBaseSPtr filter)
  {
    return std::dynamic_pointer_cast<T>(filter->genericFilter());
  }
}

#endif // ESPINA_FILTERADAPTER_H