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

#ifndef ESPINA_IO_CLASSIFICATIONXML_H
#define ESPINA_IO_CLASSIFICATIONXML_H

// ESPINA
#include "Core/IO/ErrorHandler.h"
#include <Core/Analysis/Category.h>

// Qt
#include <QFileInfo>

namespace ESPINA {
  namespace IO {
    namespace ClassificationXML
    {
      /** \brief Loads a classification from a file.
       * \param[in] file, QFileInfo object with the file details.
       * \param[in] handler, error handler smart pointer.
       *
       */
      ClassificationSPtr EspinaCore_EXPORT load(const QFileInfo& file,
                                                ErrorHandlerSPtr handler = ErrorHandlerSPtr());

      /** \brief Saves a classification to a file.
       * \param[in] classification, classification to save.
       * \param[in] file, QFileInfo object with the file details.
       * \param[in] handler, error handler smart pointer.
       *
       */
      void EspinaCore_EXPORT save(ClassificationSPtr classification,
                                  const QFileInfo&   file,
                                  ErrorHandlerSPtr    handler = ErrorHandlerSPtr());


      /** \brief Returns the classification as a byte stream.
       * \param[in] classification, classification to dump.
       * \param[in] handler, error handler smart pointer.
       *
       */
      QByteArray EspinaCore_EXPORT dump(const ClassificationSPtr classification,
                                        ErrorHandlerSPtr         handler = ErrorHandlerSPtr());

      /** \brief Parses a byte array and builds and returns a classification.
       * \param[in] serialization, byte array.
       * \param[in] handler, error handler smart pointer.
       *
       */
      ClassificationSPtr EspinaCore_EXPORT parse(const QByteArray& serialization,
                                                 ErrorHandlerSPtr  handler = ErrorHandlerSPtr());
    }
  }
}

#endif // ESPINA_IO_CLASSIFICATIONXML_H
