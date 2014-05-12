/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#ifndef ESPINA_IO_ZIP_UTILS_H
#define ESPINA_IO_ZIP_UTILS_H

#include <quazip.h>

namespace EspINA {
  namespace IO {

    struct IO_Zip_Exception{};

    class ZipUtils
    {
    public:
      static void AddFileToZip(const QString&    fileName,
                               const QByteArray& content,
                               QuaZip&           zip)
      throw (IO_Zip_Exception);

      static QByteArray readFileFromZip(const QString&  fileName,
                                        QuaZip&         zip);

      static QByteArray readCurrentFileFromZip(QuaZip& zip);
    };

  } // namespace IO
} // namesace EspINA

#endif // ESPINA_IO_ZIP_UTILS_H
