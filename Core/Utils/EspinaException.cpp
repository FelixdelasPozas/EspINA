/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include "EspinaException.h"

// C++
#include <iostream>

using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
EspinaException::EspinaException(const QString& what, const QString& info)
: m_what{what}
, m_info{info}
{
//  std::cerr << "ESPINA EXCEPTION" << std::endl;
//  std::cerr << what.toStdString().c_str() << std::endl;
//  std::cerr << info.toStdString().c_str() << std::endl;
//  std::cerr << std::flush;
}

//-----------------------------------------------------------------------------
EspinaException::~EspinaException()
{
}

//-----------------------------------------------------------------------------
const char* EspinaException::what() const noexcept
{
  return m_what.toStdString().c_str();
}

//-----------------------------------------------------------------------------
const QString& EspinaException::details() const
{
  return m_info;
}
