/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_ESPINA_ERROR_HANDLER_H
#define ESPINA_ESPINA_ERROR_HANDLER_H

#include <Core/IO/ErrorHandler.h>
#include <memory>
#include <QMap>

class QWidget;

namespace EspINA {

  class EspinaErrorHandler 
  : public IO::ErrorHandler
  {
  public:
    EspinaErrorHandler(QWidget *parent = nullptr)
    : m_parent(parent) {};

    void setDefaultDir(const QDir &dir)
    {m_defaultDir = dir;}

    void warning(const QString &msg);

    void error(const QString &msg);

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
