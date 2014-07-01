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

#include "Core/IO/ErrorHandler.h"
#include <Core/Analysis/Category.h>

#include <QFileInfo>

namespace EspINA {
  namespace IO {
    namespace ClassificationXML
    {
      struct IO_Exception{};
      struct Parse_Exception{};

      ClassificationSPtr load(const QFileInfo& file,
                              ErrorHandlerSPtr handler = ErrorHandlerSPtr());

      void save(ClassificationSPtr classification,
                const QFileInfo&   file,
                ErrorHandlerSPtr    handler = ErrorHandlerSPtr());


      QByteArray dump(const ClassificationSPtr classification,
                      ErrorHandlerSPtr         handler = ErrorHandlerSPtr());

      ClassificationSPtr parse(const QByteArray& serialization,
                               ErrorHandlerSPtr  handler = ErrorHandlerSPtr());
    }
  }
}

#endif // ESPINA_IO_CLASSIFICATIONXML_H
