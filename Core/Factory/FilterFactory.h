/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

#ifndef ESPINA_FILTER_FACTORY_H
#define ESPINA_FILTER_FACTORY_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Analysis/Filter.h"

namespace ESPINA
{
  using FilterTypeList = QList<Filter::Type>;

  class EspinaCore_EXPORT FilterFactory
  {
  public:
    struct Unknown_Filter_Exception{};
    struct Filter_Not_Provided_Exception{};

  public:
    /** brief FilterFactory class destructor.
     *
     */
    virtual ~FilterFactory()
    {}

    /** brief Creates a filter of the given type with the given inputs and scheduler.
     * \param[in] inputs, list of input object smart pointers.
     * \param[in] type, filter type.
     * \param[in] sheduler, scheduler object smart pointer.
     *
     */
    virtual FilterSPtr createFilter(InputSList          inputs,
                                    const Filter::Type& filter,
                                    SchedulerSPtr       scheduler) const throw (Unknown_Filter_Exception) = 0;

    /** brief Returns a list types of filter this factory can create.
     *
     */
    virtual FilterTypeList providedFilters() const = 0;
  };

  using FilterFactoryPtr  = FilterFactory *;
  using FilterFactorySPtr = std::shared_ptr<FilterFactory>;
  using FilterFactorySList = QList<FilterFactorySPtr>;

}// namespace ESPINA

#endif // ESPINA_FILTER_FACTORY_H
