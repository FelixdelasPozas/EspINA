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

#ifndef ESPINA_IO_SEGFILE_H
#define ESPINA_IO_SEGFILE_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "ErrorHandler.h"
#include <Core/Analysis/Analysis.h>
#include <Core/Factory/AnalysisReader.h>

namespace ESPINA
{
  namespace IO
  {
    namespace SegFile
    {
      struct IO_Error_Exception{};

      struct Classification_Not_Found_Exception{};

      struct File_Not_Found_Exception{};

      struct Parse_Exception{};

      /** \brief Loads an analysis from a file in disk.
       * \param[in] file, QFileInfo object with the file info.
       * \param[in] factory, factory smart pointer.
       * \param[in] handler, error handler smart pointer.
       *
       */
      AnalysisSPtr EspinaCore_EXPORT load(const QFileInfo& file,
                                          CoreFactorySPtr  factory = CoreFactorySPtr(),
                                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());

      /** \brief Saves an analysis to a file in disk.
       * \param[in] analysis, analysis to save.
       * \param[in] file, QFileInfo object with the file info.
       * \param[in] handler, error handler smart pointer.
       *
       */
      void EspinaCore_EXPORT save(AnalysisPtr       analysis,
                                  const QFileInfo&  file,
                                  ErrorHandlerSPtr  handler = ErrorHandlerSPtr());
    }
  }
}
#endif // ESPINA_IO_SEGFILE_H
