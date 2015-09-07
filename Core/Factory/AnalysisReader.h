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

#ifndef ESPINA_ANALYSIS_READER_H
#define ESPINA_ANALYSIS_READER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Analysis.h>
#include <Core/IO/ErrorHandler.h>

namespace ESPINA
{
  namespace IO
  {
    class ProgressReporter;

    class EspinaCore_EXPORT AnalysisReader
    {
    public:
      using Extensions    = QStringList;
      using Description   = QString;
      using ExtensionList = QMap<Description, Extensions>;

    public:
      /** \brief AnalysisReader class destructor.
       *
       */
      virtual ~AnalysisReader()
      {}

      /** \brief Returns the type of analysis reader.
       *
       */
      virtual QString type() const = 0;

      /** \brief Returns a list of file extensions the reader can process.
       *
       */
      virtual ExtensionList supportedFileExtensions() const = 0;

      /** \brief Reads an analysis data file.
       * \param[in] file analysis data file.
       * \param[in] factory core factory smart pointer.
       * \param[in] hander error handler smart pointer.
       *
       */
      virtual AnalysisSPtr read(const QFileInfo&  file,
                                CoreFactorySPtr   factory,
                                ProgressReporter *reporter = nullptr,
                                ErrorHandlerSPtr  handler = ErrorHandlerSPtr()) = 0;
    };
  } // namespace IO

  using AnalysisReaderPtr   = IO::AnalysisReader *;
  using AnalysisReaderList  = QList<AnalysisReaderPtr>;
  using AnalysisReaderSPtr  = std::shared_ptr<IO::AnalysisReader>;
  using AnalysisReaderSList = QList<AnalysisReaderSPtr>;

}// namespace ESPINA

#endif // ESPINA_ANALYSIS_READER_H
