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

#ifndef ESPINA_ESPINA_ERROR_HANDLER_H
#define ESPINA_ESPINA_ERROR_HANDLER_H

// ESPINA
#include <Core/IO/ErrorHandler.h>

// C++
#include <memory>

// Qt
#include <QMap>

class QWidget;

namespace ESPINA
{
  /** \class EspinaErrorHandler
   * \brief Implements the error handler for opening SEG files for the application.
   *
   */
  class EspinaErrorHandler
  : public IO::ErrorHandler
  {
    public:
      /** \brief EspinaErrorHandler class constructor.
       * \param[in] parent QWidget raw pointer of the parent of this object.
       *
       */
      explicit EspinaErrorHandler(QWidget *parent = nullptr)
      : m_parent(parent)
      {};

      virtual void warning(const QString &message);

      virtual void error(const QString &message);

      virtual QFileInfo fileNotFound(const QFileInfo                     &file,
                                     QDir                                 dir     = QDir(),
                                     const Core::Utils::SupportedFormats &filters = Core::Utils::SupportedFormats().addAllFormat(),
                                     const QString                       &hint    = QString());

    private:
      QWidget                 *m_parent; /** parent for the file open dialog. */
      QMap<QString, QFileInfo> m_files;  /** cache of found files.            */
  };

  using EspinaErrorHandlerSPtr = std::shared_ptr<EspinaErrorHandler>;
}

#endif // ESPINA_ESPINAERRORHANDLER_H
