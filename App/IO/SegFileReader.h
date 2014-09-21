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

#ifndef ESPINA_SEGFILEREADER_H
#define ESPINA_SEGFILEREADER_H

// ESPINA
#include <Core/Factory/AnalysisReader.h>

namespace ESPINA {

  class SegFileReader
  : public IO::AnalysisReader
  {
  public:
  	/** brief Overrides IO::AnalisysReader::type().
  	 *
  	 */
    virtual QString type() const override
    { return "SegFileReader"; }

    /** brief Overrides IO::AnalisysReader::supportedFileExtensions().
     *
     */
    virtual ExtensionList supportedFileExtensions() const override;

    /** brief Overrides IO::AnalisysReader::read().
     *
     */
    virtual AnalysisSPtr read(const QFileInfo& file,
                              CoreFactorySPtr  factory,
                              ErrorHandlerSPtr handler = ErrorHandlerSPtr()) override;
  };

}

#endif // ESPINA_SEGFILEREADER_H
