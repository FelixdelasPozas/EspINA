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

// ESPINA
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Data/VolumetricData.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT ImageLogicFilter
  : public Filter
  {
    public:
      enum class Operation : std::int8_t
      { ADDITION = 1,
        SUBTRACTION = 2,
        NOSIGN = 3
      };

    public:
      /* \brief ImageLogicFilter class constructor.
       *
       */
      explicit ImageLogicFilter(InputSList inputs, Type type, SchedulerSPtr scheduler);

      /* \brief ImageLogicFilter class virtual destructor.
       *
       */
      virtual ~ImageLogicFilter();

      /* \brief Implements Persistent::restoreState().
       *
       */
      virtual void restoreState(const State& state);


      /* \brief Implements Persistent::state().
       *
       */
      virtual State state() const;

      /* \brief Sets the operation to be executed by the filter.
       *
       */
      void setOperation(Operation op);

    protected:
      /* \brief Implements Filter::saveFilterSnapshot().
       *
       */
      virtual Snapshot saveFilterSnapshot() const;

      /* \brief Implements Filter::needUpdate().
       *
       */
      virtual bool needUpdate() const;

      /* \brief Implements Filter::needUpdate(oid).
       *
       */
      virtual bool needUpdate(Output::Id id) const;

      /* \brief Implements Filter::execute().
       *
       */
      virtual void execute();

      /* \brief Implements Filter::execute(oid).
       *
       */
      virtual void execute(Output::Id id);

      /* \brief Implements Filter::ignoreStorageContents().
       *
       */
      virtual bool ignoreStorageContent() const;

      /* \brief Implements Filter::invalidateEditedRegions().
       *
       */
      virtual bool invalidateEditedRegions();

    protected:
      /* \brief Performs the logical addition of the input segmentations.
       *
       */
      void addition();

      /* \brief Performs the substraction off all the segmentations from the
       * first one.
       *
       */
      void subtraction();

    private:
      Operation m_operation;
  };

  using ImageLogicFilterPtr  = ImageLogicFilter *;
  using ImageLogicFilterSPtr = std::shared_ptr<ImageLogicFilter>;

} // namespace ESPINA

#endif // ESPINA_IMAGE_LOGIC_FILTER_H
