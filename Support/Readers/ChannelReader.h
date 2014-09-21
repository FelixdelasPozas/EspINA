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

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Core/Factory/AnalysisReader.h>
#include <Core/Factory/FilterFactory.h>

namespace ESPINA {

  class EspinaSupport_EXPORT ChannelReader
  : public FilterFactory
  , public IO::AnalysisReader
  {
  public:
  	/** \brief Implements IO::AnalysisReader::type().
  	 *
  	 */
    virtual QString type() const
    { return "ChannelReader"; }

    /** \brief Shadows FilterFactory::providedFilters().
     *
     */
    virtual FilterTypeList providedFilters() const;

    /** \brief Shadows FilterFactory::createFilter().
     *
     */
    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

  	/** \brief Implements IO::AnalysisReader::supportedFileExtensions().
  	 *
  	 */
    virtual ExtensionList supportedFileExtensions() const;

  	/** \brief Implements IO::AnalysisReader::read().
  	 *
  	 */
    virtual AnalysisSPtr read(const QFileInfo& file,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler = ErrorHandlerSPtr());
  };

  using ChannelReaderSPtr = std::shared_ptr<ChannelReader>;
}

#endif // ESPINA_CHANNELREADER_H
