/*
 * IOErrorHandler.h
 *
 *  Created on: Feb 18, 2013
 *      Author: felix
 */

#ifndef ESPINA_IO_ERRORHANDLER_H
#define ESPINA_IO_ERRORHANDLER_H

#include "Core/EspinaCore_Export.h"

#include <QDir>
#include <QFileInfo>
#include <QString>

#include <memory>

namespace ESPINA
{
  namespace IO 
  {

  enum class STATUS
  { SUCCESS
  , FILE_NOT_FOUND
  , INVALID_VERSION
  , IO_ERROR
  };

  class ErrorHandler
  {
  public:

  public:
    virtual ~ErrorHandler() {}

    virtual void warning(const QString& msg) = 0;
    virtual void error(const QString& msg) = 0;

    virtual QFileInfo fileNotFound(const QFileInfo& file,
                                   QDir dir = QDir(),
                                   const QString &nameFilters = QString(),
                                   const QString &hint = QString()) = 0;
  };

  } // namespace IO

  using ErrorHandlerPtr  = IO::ErrorHandler*;
  using ErrorHandlerSPtr = std::shared_ptr<IO::ErrorHandler>;
} // namespace ESPINA

#endif // ESPINA_IO_ERRORHANDLER_H
