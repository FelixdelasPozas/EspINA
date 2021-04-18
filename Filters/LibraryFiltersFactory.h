/*

 Copyright (C) 2020 Felix de las Pozas Alvarez <felix.delaspozas@ctb.upm.es>

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

#ifndef FILTERS_LIBRARYFILTERSFACTORY_H_
#define FILTERS_LIBRARYFILTERSFACTORY_H_

#include "Filters/EspinaFilters_Export.h"

// Project
#include <Core/Analysis/DataFactory.h>
#include <Core/Factory/FilterFactory.h>

namespace ESPINA
{
  /** \brief Helper class that can create all the filter's types in the library.
   *
   */
  class EspinaFilters_EXPORT LibraryFiltersFactory
  : public FilterFactory
  {
    public:
      virtual FilterSPtr createFilter(InputSList          inputs,
                                      const Filter::Type& filter,
                                      SchedulerSPtr       scheduler) const;

      virtual const FilterTypeList providedFilters() const;

    private:
      mutable DataFactorySPtr m_mCubesData = nullptr; /** marching cubes data factory. */
      mutable DataFactorySPtr m_rawData    = nullptr; /** raw data factory.            */
  };
} // namespace ESPINA

#endif // FILTERS_LIBRARYFILTERSFACTORY_H_
