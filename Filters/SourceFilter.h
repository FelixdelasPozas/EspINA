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

#ifndef ESPINA_SOURCE_FILTER_H
#define ESPINA_SOURCE_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT SourceFilter
  : public Filter
  {
  public:
    struct Unexpected_Filter_Exception{};

  public:
    /** \brief SourceFilter class constructor.
     * \param[in] inputs list of input smart pointers.
     * \param[in] type SourceFilter type.
     * \param[in] scheduler scheduler smart pointer.
     *
     */
    explicit SourceFilter(InputSList    inputs,
                          Filter::Type  type,
                          SchedulerSPtr scheduler)
    : Filter(inputs, type, scheduler)
    {};

    /** \brief SourceFilter class virtual destructor.
     *
     */
    virtual ~SourceFilter()
    {};

    /** \brief Implements Persistent::restoreState().
     *
     */
    virtual void restoreState(const State &state)
    {};

    /** \brief Implements Persistent::state().
     *
     */
    virtual State state() const
    { return State(); }

    /** \brief Adds an output to the filter.
     * \param[in] id id of the output.
     * \param[in] output Output object smart pointer.
     *
     */
    void addOutput(Output::Id id, OutputSPtr output);

    /** \brief Adds an output data to the filter.
     * \param[in] id id of the output.
     * \param[in] output Output object smart pointer.
     *
     *  If the no output exits, it creates a new output.
     *  If any data already exists for the output, it is replaced with the new one
     */
    void addOutputData(Output::Id id, DataSPtr data);

  protected:
    virtual Snapshot saveFilterSnapshot() const
    { return Snapshot(); }

    virtual bool needUpdate() const
    { return false; }

    virtual bool needUpdate(Output::Id oId) const;

    virtual void execute()
    { execute(0); }

    virtual void execute(Output::Id oId);

    virtual bool ignoreStorageContent() const
    { return false; }

    virtual bool areEditedRegionsInvalidated()
    { return false; }
  };

  using SourceFilterSPtr = std::shared_ptr<SourceFilter>;

} // namespace ESPINA

#endif // ESPINA_SOURCE_FILTER_H
