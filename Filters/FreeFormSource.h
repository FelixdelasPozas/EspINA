/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef FREEFORMSOURCE_H
#define FREEFORMSOURCE_H

// #include "EspinaFilters_Export.h" añadir EspinaFilters_EXPORT a la clase luego

#include "BasicSegmentationFilter.h"
#include <Core/Utils/BinaryMask.h>

#include <QVector3D>

namespace EspINA
{
  class EspinaFilters_EXPORT FreeFormSource
  : public Filter
  {
    public:
      explicit FreeFormSource(OutputSList inputs,
                              Filter::Type     type,
                              SchedulerSPtr    scheduler);
      virtual ~FreeFormSource();

      virtual void restoreState(const State &state);
      virtual State state() const;

      void setMask(BinaryMaskSPtr<unsigned char> mask)
      { m_mask = mask; }

    protected:
      virtual Snapshot saveFilterSnapshot() const;

      virtual bool needUpdate() const
      { return needUpdate(0); }

      virtual bool needUpdate(Output::Id oId) const;

      virtual void execute()
      { execute(0); }

      virtual void execute(Output::Id oId);

      virtual bool ignoreStorageContent() const
      { return false; }

      virtual bool invalidateEditedRegions();

    private:
      BinaryMaskSPtr<unsigned char> m_mask;
  };

} // namespace EspINA


#endif // FREEFORMSOURCE_H
