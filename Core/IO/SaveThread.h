/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef CORE_IO_SAVETHREAD_H_
#define CORE_IO_SAVETHREAD_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Analysis.h>
#include <Core/IO/ErrorHandler.h>
#include <Core/IO/ProgressReporter.h>
#include <Core/Types.h>
#include <Core/MultiTasking/Task.h>

// Qt
#include <QDir>

namespace ESPINA
{
  namespace IO
  {
    namespace SegFile
    {
      /** \class SaveThread
       * \brief Saves an analysis to a file in disk using a separate thread.
       *
       */
      class EspinaCore_EXPORT SaveThread
      : public Task
      {
          Q_OBJECT
        public:
          /** \brief SaveThread class constructor.
           * \param[in] scheduler application task scheduler.
           * \param[in] analysis analysis to save.
           * \param[in] file QFileInfo object with the file info.
           * \param[in] reporter progress reporter object.
           * \param[in] handler error handler smart pointer.
           */
          SaveThread(SchedulerSPtr        scheduler,
                     AnalysisPtr          analysis,
                     const QFileInfo&     file,
                     ProgressReporterSPtr reporter = nullptr,
                     ErrorHandlerSPtr     handler = ErrorHandlerSPtr());

          /** \brief SaveThread class virtual destructor.
           *
           */
          virtual ~SaveThread()
          {};

          /** \brief Returns true if the process succeeded and false otherwise.
           *
           */
          const bool successful() const
          { return m_success; };

          /** \brief Returns the description of the error or an empty string if the process was successful.
           *
           */
          const QString errorMessage() const
          { return m_errorMessage; };

          /** \brief Returns the Qt file info structure of the file on disk.
           *
           */
          const QFileInfo filename() const
          { return m_file; };

        private:
          virtual void run();

        private:
          AnalysisPtr          m_analysis; /** analysis to save.          */
          const QFileInfo      m_file;     /** info of the file on disk.  */
          ProgressReporterSPtr m_reporter; /** progress reporter object.  */
          ErrorHandlerSPtr     m_handler;  /** application error handler. */

          bool              m_success;      /** true if the process succeeded, false otherwise. */
          QString           m_errorMessage; /** description of the error or empty if succeeded. */
      };
    } // namespace SegFile
  } // namespace IO
} // namespace ESPINA

#endif // CORE_IO_SAVETHREAD_H_
