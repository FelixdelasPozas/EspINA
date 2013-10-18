/*
 * IOErrorHandler.h
 *
 *  Created on: Feb 18, 2013
 *      Author: felix
 */

#ifndef ESPINA_IO_ERRORHANDLER_H
#define ESPINA_IO_ERRORHANDLER_H

#include "EspinaCore_Export.h"

#include <QDir>
#include <QFileInfo>
#include <QString>

namespace EspINA
{
  namespace IO 
  {

  class ErrorHandler
  {
  public:
    enum class STATUS
    { SUCCESS
    , FILE_NOT_FOUND
    , INVALID_VERSION
    , IO_ERROR
    };

  public:
    virtual ~ErrorHandler() {}

    virtual void warning(const QString& msg) = 0;
    virtual void error(const QString& msg) = 0;

    virtual QFileInfo fileNotFound(const QFileInfo& file,
                                   QDir dir = QDir(),
                                   const QString &nameFilters = QString(),
                                   const QString &hint = QString()) = 0;
  };

  using ErrorHandlerPtr = ErrorHandler*;
  } // namespace IO
} // namespace EspINA

#endif // ESPINA_IO_ERRORHANDLER_H
