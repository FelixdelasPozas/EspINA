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

#ifndef ESPINA_FILTER_REFINER_REGISTER_H
#define ESPINA_FILTER_REFINER_REGISTER_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>
#include <Support/FilterRefiner.h>

namespace ESPINA {

  struct Unknown_Filter_Type_Exception{};

  namespace Support
  {
    class EspinaSupport_EXPORT FilterRefinerRegister
    {
    public:
      /** \brief Register a filter refiner to modify filters of given \p type
       * \param[in] refiner create widgets to modify filters of given \p type
       * \param[in] type of filters
       */
      void registerFilterRefiner(const FilterRefinerSPtr refiner, const Filter::Type type);

      /** \brief Returns the filter refiner for \p segmentation filter
       * \param[in] segmentation whose filter is going to be refined
       * \param[in] context of current session
       */
      QWidget *createRefineWidget(SegmentationAdapterPtr segmentation, Context &context) throw (Unknown_Filter_Type_Exception);

      // TODO: DEPRECATED
      void unregisterFilterRefiners();

    private:
      //     using Factory = QPair<FilterRefinerSPtr, Filter::Type>;

      QMap<Filter::Type, FilterRefinerSPtr> m_register;
      //     QMap<SegmentationAdapterPtr, Factory> m_instances;
    };
  }

} // namespace ESPINA

#endif // ESPINA_FILTER_REFINER_REGISTER_H
