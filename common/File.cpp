/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "File.h"


//------------------------------------------------------------------------
File::File(const QString file)
: QString(file)
{
}

//------------------------------------------------------------------------
QString File::extension(const QString file)
{
  return file.section('.',-1);
}

//------------------------------------------------------------------------
QString File::extension() const
{
  return extension(*this);
}

//------------------------------------------------------------------------
QString File::extendedName(const QString file)
{
  return file.section('/',-1);
}

//------------------------------------------------------------------------
QString File::extendedName() const
{
  return extendedName(*this);
}

//------------------------------------------------------------------------
QString File::name(const QString file)
{
  return file.section('/',-1).section('.',0,-2);
}

//------------------------------------------------------------------------
QString File::name() const
{
  return name(*this);
}

//------------------------------------------------------------------------
QString File::parentDirectory(const QString file)
{
  return file.section('/',0,-2);
}

//------------------------------------------------------------------------
QString File::parentDirectory() const
{
  return parentDirectory(*this);
}