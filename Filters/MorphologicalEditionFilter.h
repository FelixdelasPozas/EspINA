/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

 This program is free software: you can redistribute it and/or modify
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

// #include "EspinaFilters_Export.h"

#include "BasicSegmentationFilter.h"

namespace EspINA
{
  template<class T>
  class EspinaFilters_EXPORT MorphologicalEditionFilter
  : public Filter
  {
    public:
      virtual ~MorphologicalEditionFilter();

      unsigned int radius() const
      { return m_radius; }

      void setRadius(int radius, bool ignoreUpdate = false)
      {
        m_radius = radius;
        m_ignoreCurrentOutputs = !ignoreUpdate;
      }

      bool isOutputEmpty()
      { return m_isOutputEmpty; }

    protected:
      explicit MorphologicalEditionFilter(OutputSList inputs, Type type, SchedulerSPtr scheduler);

      bool ignoreCurrentOutputs() const
      { return m_ignoreCurrentOutputs; }

      bool needUpdate(Output::Id oId) const;

    protected:
      bool m_ignoreCurrentOutputs;
      bool m_isOutputEmpty;

      int m_radius;
  };

} // namespace EspINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
