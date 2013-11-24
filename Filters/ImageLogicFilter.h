/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
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

#include "EspinaFilters_Export.h"

#include "BasicSegmentationFilter.h"
#include <GUI/Model/ModelAdapter.h>

// #include <itkConstantPadImageFilter.h>
// #include <itkOrImageFilter.h>
//   typedef itk::ConstantPadImageFilter<EspinaVolume, EspinaVolume> PadFilterType;
//   typedef itk::OrImageFilter<EspinaVolume, EspinaVolume, EspinaVolume> OrFilterType;

namespace EspINA
{
  class EspinaFilters_EXPORT ImageLogicFilter
  : public BasicSegmentationFilter
  {
    public:
      enum class Operation
      {
        ADDITION, SUBTRACTION, NOSIGN
      };

    public:
      virtual ~ImageLogicFilter();

      virtual Snapshot snapshot() const;

      virtual void unload();



    protected:
      explicit ImageLogicFilter(OutputSList inputs, Type type, SchedulerSPtr scheduler);

      virtual Snapshot saveFilterSnapshot() const;

      virtual bool needUpdate() const;

      virtual bool needUpdate(Output::Id oId) const;

      virtual bool fetchOutputData(Output::Id oId);



      virtual void run();

      virtual void execute();

      virtual void execute(Output::Id oId);

      virtual bool ignoreStorageContent() const
      { return false; }

      virtual bool invalidateEditedRegions();

    protected:
      void addition();
      void subtraction();

    private:
  };

} // namespace EspINA

#endif // ESPINA_IMAGE_LOGIC_FILTER_H
