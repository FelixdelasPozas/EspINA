/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_IMAGE_LOGIC_FILTER_H
#define ESPINA_IMAGE_LOGIC_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

namespace ESPINA
{
  class EspinaFilters_EXPORT ImageLogicFilter
  : public Filter
  {
    public:
      enum class Operation : std::int8_t
      {
        ADDITION = 1,
        SUBTRACTION = 2,
        NOSIGN = 3
      };

    public:
      /** \brief ImageLogicFilter class constructor.
			 * \param[in] inputs list of input smart pointers.
			 * \param[in] type ImageLogicFilter type.
			 * \param[in] scheduler scheduler smart pointer.
       *
       */
      explicit ImageLogicFilter(InputSList inputs, Type type, SchedulerSPtr scheduler = SchedulerSPtr());

      /** \brief ImageLogicFilter class virtual destructor.
       *
       */
      virtual ~ImageLogicFilter();

      virtual void restoreState(const State& state);

      virtual State state() const;

      /** \brief Sets the operation to be executed by the filter.
       * \param[in] op, Operation type.
       *
       */
      void setOperation(Operation op);

      /** \brief Sets the hue value for new strokes in case of skeleton addition.
       * \param[in] hue Hue value in [0, 359]
       *
       */
      void setNewSkeletonStrokesHue(const int hue)
      { m_hue = std::min(359, std::max(0,hue)); }

    protected:
      virtual Snapshot saveFilterSnapshot() const;

      virtual bool needUpdate() const;

      virtual bool needUpdate(Output::Id id) const;

      virtual void execute();

      virtual void execute(Output::Id id);

      virtual bool ignoreStorageContent() const;

    protected:
      /** \brief Performs the logical addition of the input volumetric segmentations.
       *
       */
      void volumetricAddition();

      /** \brief Performs the logical addition of the input skeleton segmentations.
       *
       */
      void skeletonAddition();

      /** \brief Performs the subtraction of all the volumetric segmentations from the first one.
       *
       */
      void volumetricSubtraction();

      /** \brief Performs the subtraction of all the skeleton segmentations from the first one.
       *
       */
      void skeletonSubtraction();

    private:
      Operation m_operation; /** operation type.                                                   */
      int       m_hue;       /** hue color of skeleton strokes in the skeleton addition operation. */
  };

  using ImageLogicFilterPtr  = ImageLogicFilter *;
  using ImageLogicFilterSPtr = std::shared_ptr<ImageLogicFilter>;

} // namespace ESPINA

#endif // ESPINA_IMAGE_LOGIC_FILTER_H
