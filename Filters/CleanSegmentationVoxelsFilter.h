/*
 File: CleanSegmentationVoxelsFilter.h
 Created on: 25/07/2019
 Author: Felix de las Pozas Alvarez

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

#ifndef FILTERS_CLEANSEGMENTATIONVOXELSFILTER_H_
#define FILTERS_CLEANSEGMENTATIONVOXELSFILTER_H_

// ESPINA
#include "Filters/EspinaFilters_Export.h"
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Filter.h>

namespace ESPINA
{
  /** \class CleanSegmentationVoxelsFilter
   * \brief Implements a filter to remove isolated groups of voxels from a volumetric segmentation.
   *
   */
  class EspinaFilters_EXPORT CleanSegmentationVoxelsFilter
  : public Filter
  {
    public:
      /** \brief CleanSegmentationVoxelsFilter class constructor.
       * \param[in] inputs list of input smart pointers.
       * \param[in] type SliceInterpolationFilter type.
       * \param[in] scheduler scheduler smart pointer.
       *
       */
      explicit CleanSegmentationVoxelsFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler);

      /** \brief CleanSegmentationVoxelsFilter class virtual destructor.
       *
       */
      virtual ~CleanSegmentationVoxelsFilter()
      {}

      virtual const QStringList errors() const;

      virtual void restoreState(const State& state)
      {}

      virtual State state() const
      {return State();}

      /** \brief Returns the number of deleted voxels. Only valid when the filter has finished.
       *
       */
      unsigned long long removedVoxelsNumber() const
      { return m_removedVoxelsNum; }

    protected:
      virtual Snapshot saveFilterSnapshot() const
      { return Snapshot(); }

      virtual bool needUpdate() const;

      virtual bool ignoreStorageContent() const
      { return false; }

      virtual void execute();

    private:
      QStringList        m_errors;           /** Error message or empty if filter ran successfully. */
      unsigned long long m_removedVoxelsNum; /** number of removed voxels.                          */
  };

  using CleanSegmentationVoxelsFilterSPtr = std::shared_ptr<CleanSegmentationVoxelsFilter>;

} // namespace ESPINA

#endif // FILTERS_CLEANSEGMENTATIONVOXELSFILTER_H_
