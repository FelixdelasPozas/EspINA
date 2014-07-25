/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ESPINA_SETTINGS_H
#define ESPINA_SETTINGS_H

#include "Support/EspinaSupport_Export.h"
#include <QString>
#include <QSettings>

const QString CESVIMA = "CeSViMa";

#define ESPINA_SETTINGS(settings) QSettings settings("CeSViMa", "ESPINA");

const QString USER_NAME("UserName");

namespace ESPINA
{
  // TODO 2012-12-05 Remove this function and pass the general settings to
  // all the tools that require it
  QString userName();
};

#endif//ESPINA_SETTINGS_H
