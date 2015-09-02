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

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/IO/ErrorHandler.h"
#include <Core/Analysis/Analysis.h>

// QuaZip
#include <quazip/quazip.h>

namespace ESPINA
{
  namespace IO
  {
    class ProgressReporter;

    namespace SegFile
    {
      class EspinaCore_EXPORT SegFileInterface
      {
      public:
        /** \brief SegFileInterface class destructor.
         *
         */
        virtual ~SegFileInterface()
        {}

        /** \brief Loads an analysis from a QuaZip file.
         * \param[in] zip QuaZip handler.
         * \param[in] factory factory smart pointer to create objects.
         * \param[in] handler error handler. smart pointer.
         *
         */
        virtual AnalysisSPtr load(QuaZip           &zip,
                                  CoreFactorySPtr   factory,
                                  ProgressReporter *reporter = nullptr,
                                  ErrorHandlerSPtr  handler = ErrorHandlerSPtr()) = 0;

        /** \brief Saves an analysis to a QuaZip file.
         * \param[in] analysis analysis to save.
         * \param[in] zip QuaZip handler.
         * \param[in] handler error handler smart pointer.
         *
         */
        virtual void save(AnalysisPtr     analysis,
                          QuaZip&         zip,
                          ProgressReporter *reporter = nullptr,
                          ErrorHandlerSPtr handler = ErrorHandlerSPtr()) = 0;

      protected:
        /** \brief Adds a file to a QuaZip file.
         * \param[in] fileName file name.
         * \param[in] content content of the file.
         * \param[in] zip QuaZip handler.
         * \param[in] handler error handler smart pointer.
         *
         */
        static void addFileToZip(const QString&    fileName,
                                 const QByteArray& content,
                                 QuaZip&           zip,
                                 ErrorHandlerSPtr  handler = ErrorHandlerSPtr());

        /** \brief Reads a file from a QuaZip file.
         * \param[in] fileName file name to read.
         * \param[in] zip QuaZip handler.
         * \param[in] handler error handler smart pointer.
         *
         */
        static QByteArray readFileFromZip(const QString&  fileName,
                                          QuaZip&         zip,
                                          ErrorHandlerSPtr handler = ErrorHandlerSPtr());

        /** \brief Reads current file from a QuaZip file.
         * \param[in] zip QuaZip handler.
         * \param[in] handler error handler smart pointer.
         *
         */
        static QByteArray readCurrentFileFromZip(QuaZip&          zip,
                                                 ErrorHandlerSPtr handler = ErrorHandlerSPtr());
      };
    }
  }
}

#endif // ESPINA_IO_SEGFILE_SEGFILE_INTERFACE_H
