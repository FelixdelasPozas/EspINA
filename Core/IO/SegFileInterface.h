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

#ifndef ESPINA_IO_SEGFILE_SEGFILE_INTERFACE_H
#define ESPINA_IO_SEGFILE_SEGFILE_INTERFACE_H

#include "Core/IO/ErrorHandler.h"
#include <Core/Analysis/Analysis.h>
#include <quazip/quazip.h>

namespace ESPINA 
{
  namespace IO
  {
    namespace SegFile
    {
      class SegFileInterface
      {
      public:
        virtual ~SegFileInterface(){}

        virtual AnalysisSPtr load(QuaZip&          zip,
                                  CoreFactorySPtr  factory,
                                  ErrorHandlerSPtr handler = ErrorHandlerSPtr()) = 0;

        virtual void save(AnalysisPtr     analysis,
                          QuaZip&         zip,
                          ErrorHandlerSPtr handler = ErrorHandlerSPtr()) = 0;

      protected:
        static void addFileToZip(const QString&    fileName,
                                 const QByteArray& content,
                                 QuaZip&           zip,
                                 ErrorHandlerSPtr  handler = ErrorHandlerSPtr());

        static QByteArray readFileFromZip(const QString&  fileName,
                                   QuaZip&         zip,
                                   ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        static QByteArray readCurrentFileFromZip(QuaZip&          zip,
                                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());
      };
    }
  }
}

#endif // ESPINA_IO_SEGFILE_SEGFILE_INTERFACE_H