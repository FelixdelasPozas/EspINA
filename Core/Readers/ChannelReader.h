/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_CHANNEL_READER_H
#define ESPINA_CHANNEL_READER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>
#include <Core/IO/SegFile.h>

namespace ESPINA
{
  /** \class ChannelReader
   * \brief Implements a reader for stacks raw files.
   *
   */
  class EspinaCore_EXPORT ChannelReader
  : public FilterFactory
  , public IO::AnalysisReader
  {
    public:
      static const Filter::Type VOLUMETRIC_STREAM_READER;    /** channel reader signature.     */
      static const Filter::Type ESPINA_1_3_2_CHANNEL_READER; /** channel reader old signature. */

      virtual const QString type() const
      { return "ChannelReader"; }

      virtual const FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;

      virtual const ExtensionList supportedFileExtensions() const;

      virtual AnalysisSPtr read(const QFileInfo&      file,
                                CoreFactorySPtr       factory,
                                IO::ProgressReporter *reporter = nullptr,
                                ErrorHandlerSPtr      handler  = ErrorHandlerSPtr(),
                                const IO::LoadOptions options  = IO::LoadOptions());
  };

  using ChannelReaderSPtr = std::shared_ptr<ChannelReader>;
}

#endif // ESPINA_CHANNELREADER_H
