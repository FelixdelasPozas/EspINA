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

#ifndef ESPINA_SPLIT_FILTER_H
#define ESPINA_SPLIT_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Data/VolumetricData.hxx>

// VTK
#include <vtkSmartPointer.h>

class vtkImageStencilData;

namespace ESPINA
{
  class EspinaFilters_EXPORT SplitFilter
  : public Filter
  {
    public:
      /** brief SplitFilter class constructor.
			 * \param[in] inputs, list of input smart pointers.
			 * \param[in] type, SplitFilter type.
			 * \param[in] scheduler, scheduler smart pointer.
       *
       */
      explicit SplitFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler);

      /** brief SplitFilter class virtual destructor.
       *
       */
      virtual ~SplitFilter();

      /** brief Implements Persistent::restoreState().
       *
       */
      virtual void restoreState(const State& state)
      {}

      /** brief Implements Persistent::state().
       *
       */
      virtual State state() const
      { return State(); }

      /** brief Sets the stencil used to split the input.
       * \param[in] stencil, a vtkSmartPointer<vtkImageStencilData> object.
       *
       */
      void setStencil(vtkSmartPointer<vtkImageStencilData> stencil)
      {
        m_stencil = stencil;
        m_ignoreCurrentOutputs = true;
      }

      /** brief Try to locate an snapshot of the filter in temporalStorage, returns true
       * if all volume snapshot can be recovered and false otherwise.
       *
       */
      virtual bool fetchCacheStencil() const;

    protected:
      /** brief Implements Filter::saveFilterSnapshot().
       *
       */
      virtual Snapshot saveFilterSnapshot() const;

      /** brief Implements Filter::needUpdate().
       *
       */
      virtual bool needUpdate() const;

      /** brief Implements Filter::needUpdate(oid).
       *
       */
      virtual bool needUpdate(Output::Id id) const;

      /** brief Implements Filter::execute().
       *
       */
      virtual void execute();

      /** brief Implements Filter::execute(oid).
       *
       */
      virtual void execute(Output::Id id);

      /** brief Implements Filter::ignoreStorageContents().
       *
       */
      virtual bool ignoreStorageContent() const;

      /** brief Implements Filter::invalidateEditedRegions().
       *
       */
      virtual bool invalidateEditedRegions();

      /** brief Helper method that returns the stencil file name.
       *
       */
      QString stencilFile() const
      { return prefix() + "stencil.vti"; }

    private:
      bool m_ignoreCurrentOutputs;
      mutable vtkSmartPointer<vtkImageStencilData> m_stencil;
  };

  using SplitFilterPtr  = SplitFilter *;
  using SplitFilterSPtr = std::shared_ptr<SplitFilter>;

} // namespace ESPINA

#endif // ESPINA_SPLIT_FILTER_H
