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
#include <Support/FilterHistory.h>

namespace ESPINA {

  struct Unknown_Filter_Type_Exception{};

  class SpecificFilterDelegateFactory
  {
  public:
    virtual ~SpecificFilterDelegateFactory(){}

    /** \brief Return a list of filter types for whom there is a filter delegate
     *
     */
    virtual QList<Filter::Type> availableFilterDelegates() const = 0;

    /** \brief Creates a filter delegate for \p filter
     *
     * If the type of the filter is invalid, an exception will be thrown
     *
     * \param[in] filter for which the delegate will be creted
     */
    virtual FilterDelegateSPtr createDelegate(FilterSPtr filter) throw (Unknown_Filter_Type_Exception) = 0;
  };

  using SpecificFilterDelegateFactorySPtr = std::shared_ptr<SpecificFilterDelegateFactory>;

  class FilterDelegateFactory
  {
  public:
    /** \brief Register a filter delegate factory
     * \param[in] factory filter delegate factory
     */
    void registerFilterDelegateFactory(SpecificFilterDelegateFactorySPtr factory);

    /** \brief Creates a filter delegate for \p filter
     *
     * If the type of the filter is invalid, an exception will be thrown
     *
     * \param[in] filter for which the delegate will be creted
     */
    FilterDelegateSPtr createDelegate(SegmentationAdapterPtr segmentation) throw (Unknown_Filter_Type_Exception);

    void resetDelegates();

  private:
    using Factory = QPair<FilterDelegateSPtr, Filter::Type>;

    QMap<Filter::Type, SpecificFilterDelegateFactorySPtr> m_factories;
    QMap<SegmentationAdapterPtr, Factory>                 m_instances;
  };

  using FilterDelegateFactorySPtr = std::shared_ptr<FilterDelegateFactory>;


} // namespace ESPINA

#endif // ESPINA_FILTER_DELEGATE_FACTORY_H
