/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_FILTER_DELEGATE_FACTORY_H
#define ESPINA_FILTER_DELEGATE_FACTORY_H

#include <Core/Analysis/Filter.h>
#include <GUI/Model/FilterAdapter.h>

namespace ESPINA {

  class FilterDelegateFactory
  {
  public:
    struct Unknown_Filter_Type_Exception{};

  public:
    virtual ~FilterDelegateFactory(){}

    /** \brief Return a list of filter types for whom there is a filter delegate
     *
     */
    virtual QList<Filter::Type> availableFilterDelegates() const = 0;

    /** \brief Set a filter delegate to \p filter
     *
     * If the type of the filter is invalid, an exception will be thrown
     *
     * \param[in] filter in which the delegate will be set
     */
    virtual void setDelegate(FilterAdapterBaseSPtr filter) throw (Unknown_Filter_Type_Exception) = 0;
  };

  using FilterDelegateFactorySPtr = std::shared_ptr<FilterDelegateFactory>;

} // namespace ESPINA

#endif // ESPINA_FILTER_DELEGATE_FACTORY_H
