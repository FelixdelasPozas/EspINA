/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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
#include "translatorTable.h"

//-----------------------------------------------------------------------------
// TRANSLATORTABLE
//-----------------------------------------------------------------------------
void TranslatorTable::addTranslation(const EspinaArg& espina, const VtkArg& vtk)
{
  m_table.insert(espina,vtk);
}


VtkParamList TranslatorTable::translate(EspinaParamList args) const
{
  VtkParamList result;
  for (unsigned int arg = 0; arg < args.size(); arg++)
  {
    VtkParam p;
    EspinaArg key = args[arg].first;
    if (!m_table.contains(key))
      continue;
    p.first = m_table[key];
    p.second = args[arg].second;
    result.push_back(p);
  }
  return result;
}

