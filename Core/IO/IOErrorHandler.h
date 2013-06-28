/*
 * IOErrorHandler.h
 *
 *  Created on: Feb 18, 2013
 *      Author: felix
 */

#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#include "EspinaCore_Export.h"

#include <QDir>
#include <QFileInfo>
#include <QString>

namespace EspINA
{

  class IOErrorHandler
  {
  public:
    enum STATUS
    { SUCCESS
      , FILE_NOT_FOUND
      , INVALID_VERSION
      , IO_ERROR
    };

  public:
    virtual ~IOErrorHandler() {}

    virtual void warning(const QString &msg) = 0;
    virtual void error(const QString &msg) = 0;

    virtual QFileInfo fileNotFound(const QFileInfo &file,
                                   QDir dir = QDir(),
                                   const QString &nameFilters = QString(),
                                   const QString &hint = QString()) = 0;
  };

} /* namespace EspINA */
#endif /* ERRORHANDLER_H_ */
