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


#ifndef TRANSLATORTABLE_H
#define TRANSLATORTABLE_H

#include "cajalTypes.h"

#include <QMap>

typedef NodeArg EspinaArg;
typedef NodeParamList EspinaParamList;

//! Different Types of Vtk Properties
enum VtkPropType
{ UNKOWN     = -1
, INPUT      = 0    
, INTVECT    = 1
, DOUBLEVECT = 2
};

//! Argument that can be interpreted as a vtk property
struct VtkArg
{
  VtkPropType type;
  std::string name;
};

typedef std::pair<VtkArg,ParamValue> VtkParam;
typedef std::vector<VtkParam> VtkParamList;

//! A class to create translation between Espina and Vtk parameter lists
class TranslatorTable{
public:
  //TODO: Sort args at translation time
  VtkParamList translate(EspinaParamList args) const;
  void addTranslation(const EspinaArg &espina,const VtkArg &vtk);
  
private:
  QMap<EspinaArg,VtkArg> m_table;
};


#endif // TRANSLATORTABLE_H
