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

#ifndef ESPINA_IO_ZIP_UTILS_H
#define ESPINA_IO_ZIP_UTILS_H

#include "Core/EspinaCore_Export.h"

// QuaZip
#include <quazip/quazip.h>

namespace ESPINA {
  namespace IO {

    struct IO_Zip_Exception{};

    class EspinaCore_EXPORT ZipUtils
    {
    public:
    	/** \brief Adds a file to a QuaZip file.
    	 * \param[in] fileName, file name.
    	 * \param[in] content, file content as a byte array.
    	 * \param[in] zip, QuaZip handler.
    	 *
    	 */
      static void AddFileToZip(const QString&    fileName,
                               const QByteArray& content,
                               QuaZip&           zip) throw (IO_Zip_Exception);

    	/** \brief Reads a file from a QuaZip file and returns its content as a byte array.
    	 * \param[in] fileName, file name.
    	 * \param[in] zip, QuaZip handler.
    	 *
    	 */
      static QByteArray readFileFromZip(const QString&  fileName,
                                        QuaZip&         zip);

    	/** \brief Reads current file from a QuaZip file and returns its content as a byte array.
    	 * \param[in] zip, QuaZip handler.
    	 *
    	 */
      static QByteArray readCurrentFileFromZip(QuaZip& zip);
    };

  } // namespace IO
} // namesace ESPINA

#endif // ESPINA_IO_ZIP_UTILS_H
