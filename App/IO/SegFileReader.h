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

namespace ESPINA
{
  /** \class SegFileReader
   * \brief Reader for Espina SEG files.
   *
   */
  class SegFileReader
  : public IO::AnalysisReader
  {
  public:
    virtual const QString type() const override
    { return "SegFileReader"; }

    virtual const ExtensionList supportedFileExtensions() const override;

    virtual AnalysisSPtr read(const QFileInfo&      file,
                              CoreFactorySPtr       factory,
                              IO::ProgressReporter *reporter = nullptr,
                              ErrorHandlerSPtr      handler  = ErrorHandlerSPtr(),
                              IO::LoadOptions       options  = IO::LoadOptions()) override;
  };

}

#endif // ESPINA_SEGFILEREADER_H
