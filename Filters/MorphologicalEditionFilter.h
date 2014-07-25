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

#include "Core/Analysis/Filter.h"
#include <itkImageToImageFilter.h>
#include <itkCommand.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT MorphologicalEditionFilter
  : public Filter
  {
    public:
      virtual ~MorphologicalEditionFilter();

      virtual void restoreState(const State& state);

      virtual State state() const;

      unsigned int radius() const
      { return m_radius; }

      void setRadius(int radius, bool ignoreUpdate = false)
      {
        m_radius = radius;
        m_ignoreStorageContent = !ignoreUpdate;
      }

      bool isOutputEmpty()
      { return m_isOutputEmpty; }

    protected:
      explicit MorphologicalEditionFilter(InputSList    inputs,
                                          Filter::Type  type,
                                          SchedulerSPtr scheduler);

      virtual Snapshot saveFilterSnapshot() const;

      virtual bool needUpdate() const;

      virtual bool needUpdate(Output::Id id) const;

      virtual bool ignoreStorageContent() const
      { return m_ignoreStorageContent; }

      virtual bool invalidateEditedRegions();

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
