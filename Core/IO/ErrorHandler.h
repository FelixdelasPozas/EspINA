/*
 * ErrorHandler.h
 *
 *  Created on: Feb 18, 2013
 *      Author: felix
 */

#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#include <QString>
#include <QFileInfo>

namespace EspINA
{
  
  class EspinaIO::ErrorHandler
  {
    public:
      virtual ~ErrorHandler() {}

     virtual void warning(const QString &msg) = 0;
     virtual void error(const QString &msg) = 0;

     // TODO: usar QApplication::restoreOverrideCursor();
     virtual QFileInfo fileNotFound(const QFileInfo &file,
                            QDir dir = QDir(),
                            const QString &nameFilters = QString(),
                            const QString &hint = QString()) = 0;
  };

} /* namespace EspINA */
#endif /* ERRORHANDLER_H_ */
