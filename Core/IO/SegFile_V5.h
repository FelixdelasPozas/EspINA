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

#ifndef ESPINA_SEGFILE_V5_H
#define ESPINA_SEGFILE_V5_H

#include "Core/IO/SegFileInterface.h"
#include <Core/Utils/TemporalStorage.h>
#include <Core/Analysis/Output.h>
#include <Core/Analysis/FetchBehaviour.h>

namespace ESPINA {

  namespace IO {

    namespace SegFile {

      class SegFile_V5
      : public SegFileInterface
      {
        class Loader;

      public:
        static const QString FORMAT_INFO_FILE;

      public:
        SegFile_V5();

        virtual AnalysisSPtr load(QuaZip&          zip,
                                  CoreFactorySPtr  factory = CoreFactorySPtr(),
                                  ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        virtual void save(AnalysisPtr      analysis, 
                          QuaZip&          zip,
                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());
      };
    }
  }
}

#endif // ESPINA_SEGFILE_V5_H
