/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_IO_PROGRESS_REPORTER_H
#define ESPINA_IO_PROGRESS_REPORTER_H

#include <Core/EspinaCore_Export.h>

// Qt
#include <QObject>

// C++
#include <memory>

namespace ESPINA
{
  namespace IO
  {
    /** \class ProgressReporter
     * \brief Helper base class to report a progress from an external process.
     *
     */
    class EspinaCore_EXPORT ProgressReporter
    : public QObject
    {
        Q_OBJECT
      public:
        /** \brief ProgressReporter class virtual destructor.
         *
         */
        virtual ~ProgressReporter()
        {};

        /** \brief Sets the progress value to report.
         *
         */
        virtual void setProgress(unsigned int progress) = 0;

      signals:
        void progress(int progress);
    };

    using ProgressReporterPtr  = ProgressReporter *;
    using ProgressReporterSPtr = std::shared_ptr<ProgressReporter>;
  }
} // namespace ESPINA

#endif // ESPINA_IO_PROGRESS_REPORTER_H
