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

#ifndef ESPINA_ERODE_FILTER_H
#define ESPINA_ERODE_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include "MorphologicalEditionFilter.h"

namespace ESPINA
{
  /** \class ErodeFilter
   * \brief Implements morphological erode operation.
   */
  class EspinaFilters_EXPORT ErodeFilter
  : public MorphologicalEditionFilter
  {
    public:
      /** \brief EroreFilter class constructor.
       * \param[in] inputs    list of input smart pointers.
       * \param[in] type      ErodeFilter type.
       * \param[in] scheduler scheduler smart pointer.
       *
       */
      explicit ErodeFilter(InputSList          inputs,
                           const Filter::Type &type,
                           SchedulerSPtr       scheduler);

      /** \brief ErodeFilter class virtual destructor.
       *
       */
      virtual ~ErodeFilter()
      {}

    protected:
      virtual void execute()
      { execute(0); }

      virtual void execute(Output::Id id);
  };

} // namespace ESPINA


#endif // ESPINA_ERODE_FILTER_H
