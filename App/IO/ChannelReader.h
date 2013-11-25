/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>

namespace EspINA {

  class ChannelReader
  : public FilterFactory
  , public IO::AnalysisReader
  {
  public:
    virtual QString type() const
    { return "ChannelReader"; }

    virtual FilterTypeList providedFilters() const;

    virtual FilterSPtr createFilter(OutputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;

    virtual ExtensionList supportedFileExtensions() const;

    virtual AnalysisSPtr read(const QFileInfo& file,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler = ErrorHandlerSPtr());
  };

  using ChannelReaderSPtr = std::shared_ptr<ChannelReader>;
}

#endif // ESPINA_CHANNELREADER_H
