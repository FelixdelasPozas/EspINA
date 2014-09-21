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

#ifndef ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
#define ESPINA_MORPHOLOGICAL_EDITION_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include "Core/Analysis/Filter.h"

// ITK
#include <itkImageToImageFilter.h>
#include <itkCommand.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT MorphologicalEditionFilter
  : public Filter
  {
    public:
  		/** \brief MorphologicalEditionFilter class virtual destructor.
  		 *
  		 */
      virtual ~MorphologicalEditionFilter();

      /** \brief Implements Persistent::restoreState().
       *
       */
      virtual void restoreState(const State& state);

      /** \brief Implements Persistent::state().
       *
       */
      virtual State state() const;

      /** \brief Returns the radius of the morphological operation.
       *
       */
      unsigned int radius() const
      { return m_radius; }

      /** \brief Sets the radius and the flag to ignore the storage content.
       * \param[in] radius, radius of the morphological operation.
       * \param[in] ignoreUpdate, true to use the storage content false otherwise.
       *
       */
      void setRadius(int radius, bool ignoreUpdate = false)
      {
        m_radius = radius;
        m_ignoreStorageContent = !ignoreUpdate;
      }

      /** \brief Returs true if the output is empty.
       *
       * Morphological operations like erode can destroy the segmentation.
       *
       */
      bool isOutputEmpty()
      { return m_isOutputEmpty; }

    protected:
      /** \brief MorphologicalEditionFilter class constructor.
			 * \param[in] inputs, list of input smart pointers.
			 * \param[in] type, type of the morphological operation.
			 * \param[in] scheduler, scheduler smart pointer.
			 *
       */
      explicit MorphologicalEditionFilter(InputSList    inputs,
                                          Filter::Type  type,
                                          SchedulerSPtr scheduler);

      /** \brief Implements Filter::saveFilterSnapshot().
       *
       */
      virtual Snapshot saveFilterSnapshot() const;

      /** \brief Implements Filter::needUpdate().
       *
       */
      virtual bool needUpdate() const;

      /** \brief Implements Filter::needUpdate(id).
       *
       */
      virtual bool needUpdate(Output::Id id) const;

      /** \brief Implements Filter::ignoreStorageContent().
       *
       */
      virtual bool ignoreStorageContent() const
      { return m_ignoreStorageContent; }

      /** \brief Implements Filter::invalidateEditedRegions().
       *
       */
      virtual bool invalidateEditedRegions();

      /** \brief Checks if the output is empty after execution
       * and creates the output if it's not.
       *
       */
      void finishExecution(itkVolumeType::Pointer output);

    protected:
      bool m_ignoreStorageContent;

      int  m_radius;
      bool m_isOutputEmpty;
  };

  using MorphologicalEditionFilterPtr  = MorphologicalEditionFilter *;
  using MorphologicalEditionFilterSPtr = std::shared_ptr<MorphologicalEditionFilter>;
} // namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
