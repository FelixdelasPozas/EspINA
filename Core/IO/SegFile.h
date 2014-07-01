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

#include "ErrorHandler.h"
#include <Core/Analysis/Analysis.h>
#include <Core/Factory/AnalysisReader.h>

namespace EspINA 
{
  namespace IO
  {
    namespace SegFile
    {
      struct IO_Error_Exception{};

      struct Classification_Not_Found_Exception{};

      struct File_Not_Found_Exception{};

      struct Parse_Exception{};

      AnalysisSPtr load(const QFileInfo& file,
                        CoreFactorySPtr  factory = CoreFactorySPtr(),
                        ErrorHandlerSPtr handler = ErrorHandlerSPtr());

      void save(AnalysisPtr       analysis,
                const QFileInfo&  file,
                ErrorHandlerSPtr  handler = ErrorHandlerSPtr());
    }
  }
}
#endif // ESPINA_IO_SEGFILE_H