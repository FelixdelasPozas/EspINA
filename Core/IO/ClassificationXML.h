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

#ifndef ESPINA_IO_CLASSIFICATIONXML_H
#define ESPINA_IO_CLASSIFICATIONXML_H

#include <Core/EspinaTypes.h>
#include "Core/IO/ErrorHandler.h"

#include <QFileInfo>

namespace EspINA {
  namespace IO {
    namespace ClassificationXML
    {
      ErrorHandler::STATUS load(const QFileInfo&   file,
                                ClassificationSPtr classification,
                                ErrorHandlerPtr    handler = nullptr);

      ErrorHandler::STATUS save(ClassificationSPtr classification,
                                const QFileInfo&   file,
                                ErrorHandlerPtr    handler = nullptr);


      ErrorHandler::STATUS dump(const ClassificationSPtr classification,
                                QString&                 serialization,
                                ErrorHandlerPtr          handler = nullptr);

      ErrorHandler::STATUS parse(const QString&     serialization,
                                 ClassificationSPtr classification,
                                 ErrorHandlerPtr    handler = nullptr);
    }
  }
}

#endif // ESPINA_IO_CLASSIFICATIONXML_H
