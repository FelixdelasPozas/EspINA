/*
 Copyright (C) 2013  Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_IO_ERRORHANDLER_H
#define ESPINA_IO_ERRORHANDLER_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Utils/SupportedFormats.h>

// Qt
#include <QDir>
#include <QFileInfo>
#include <QString>

// C++
#include <memory>
#include <cstdint>

namespace ESPINA
{
  namespace IO
  {
    enum class STATUS: std::int8_t {	SUCCESS = 1, FILE_NOT_FOUND = 2, INVALID_VERSION = 3, IO_ERROR = 4 };

    /** \class ErrorHandler
     * \brief Base class to manage errors during SEG file loading.
     *
     */
    class EspinaCore_EXPORT ErrorHandler
    {
      public:
        /** \brief ErrorHandler class destructor.
         *
         */
        virtual ~ErrorHandler()
        {}

        /** \brief Sets the default directory.
         * \param[in] dir QDir const reference.
         *
         */
        void setDefaultDir(const QDir &dir)
        { m_defaultDir = dir; }

        /** \brief Returns the default directory.
         *
         */
        const QDir defaultDir() const
        { return m_defaultDir; }

        /** \brief Message to show in case of a warning.
         * \param[in] msg warning message.
         *
         */
        virtual void warning(const QString& msg) = 0;

        /** \brief Message to show in case of an error.
         * \param[in] msg error message.
         *
         */
        virtual void error(const QString& msg) = 0;

        /** \brief Opens a dialog asking for the given file when it could't be found automatically.
         * \param[in] file QFileInfo onbject with the file information.
         * \param[in] dir initial directory for the open file dialog.
         * \param[in] nameFilters filters for the open file dialog.
         * \param[in] hint hint for the open file dialog.
         *
         */
        virtual QFileInfo fileNotFound(const QFileInfo& file,
                                       QDir dir = QDir(),
                                       const Core::Utils::SupportedFormats &filters = Core::Utils::SupportedFormats().addAllFormat(),
                                       const QString &hint = QString()) = 0;

        static Core::Utils::SupportedFormats SameFormat(const QFileInfo &file, bool addAll = true)
        {
          auto format = Core::Utils::SupportedFormats(QObject::tr("%1 files").arg(file.suffix()), file.suffix());

          if (addAll)
          {
            format.addAllFormat();
          }

          return format;
        }

      private:
        QDir m_defaultDir; /** default directory to show in file dialog. */
    };

  } // namespace IO

  using ErrorHandlerPtr  = IO::ErrorHandler*;
  using ErrorHandlerSPtr = std::shared_ptr<IO::ErrorHandler>;
} // namespace ESPINA

#endif // ESPINA_IO_ERRORHANDLER_H
