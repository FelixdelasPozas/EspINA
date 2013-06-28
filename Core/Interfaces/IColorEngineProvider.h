/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef ICOLORENGINEPROVIDER_H
#define ICOLORENGINEPROVIDER_H

#include "EspinaCore_Export.h"

#include <QtPlugin>
#include <QPair>

#include <Core/ColorEngines/IColorEngine.h>

namespace EspINA
{
  class EspinaCore_EXPORT IColorEngineProvider
  {
  public:
    typedef QPair<QString, ColorEnginePtr> Engine;
    typedef QList<IColorEngineProvider::Engine> EngineList;

    virtual ~IColorEngineProvider();

    virtual EngineList colorEngines() = 0;
  };
} // namespace EspINA

Q_DECLARE_INTERFACE(EspINA::IColorEngineProvider,
                    "es.upm.cesvima.EspINA.ColorEngineProviderInterface/1.2")

#endif // ICOLORENGINEPROVIDER_H
