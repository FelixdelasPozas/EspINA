/*
 * IOErrorHandler.h
 *
 *  Created on: Feb 18, 2013
 *      Author: felix
 */

#ifndef ESPINA_IO_ERRORHANDLER_H
#define ESPINA_IO_ERRORHANDLER_H

#include "Core/EspinaCore_Export.h"

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

  class EspinaCore_EXPORT ErrorHandler
  {
  public:

  public:
  	/* \brief ErrorHandler class destructor.
  	 *
  	 */
    virtual ~ErrorHandler()
    {}

    /* \brief Message to show in case of a warning.
     * \param[in] msg, warning message.
     *
     */
    virtual void warning(const QString& msg) = 0;

    /* \brief Message to show in case of an error.
     * \param[in] msg, error message.
     *
     */
    virtual void error(const QString& msg) = 0;

    /* \brief Opens a dialog asking for the given file when it could't be found automatically.
     * \param[in] file, QFileInfo onbject with the file information.
     * \param[in] dir, initial directory for the open file dialog.
     * \param[in] nameFilters, filters for the open file dialog.
     * \param[in] hint, hint for the open file dialog.
     *
     */
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
