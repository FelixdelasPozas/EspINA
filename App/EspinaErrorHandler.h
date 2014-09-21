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

  class EspinaErrorHandler
  : public IO::ErrorHandler
  {
  public:
  	/** brief EspinaErrorHandler class constructor.
  	 * \param[in] parent, QWidget raw pointer of the parent of this object.
  	 */
    EspinaErrorHandler(QWidget *parent = nullptr)
    : m_parent(parent)
  	{};

    /** brief Sets the default directory.
     * \param[in] dir, QDir const reference.
     */
    void setDefaultDir(const QDir &dir)
    { m_defaultDir = dir; }

    /** brief Implements IO::ErrorHandler::warning().
     *
     */
    void warning(const QString &msg);

    /** brief Implements IO::ErrorHandler::error().
     *
     */
    void error(const QString &msg);

    /** brief Implements IO::ErrorHandler::fileNotFound().
     *
     */
    QFileInfo fileNotFound(const QFileInfo &file,
                           QDir dir = QDir(),
                           const QString &nameFilters = QString(),
                           const QString &hint = QString());

  private:
    QWidget *m_parent;
    QDir     m_defaultDir;

    QMap<QString, QFileInfo> m_files;
  };

  using EspinaErrorHandlerSPtr = std::shared_ptr<EspinaErrorHandler>;
}

#endif // ESPINA_ESPINAERRORHANDLER_H
