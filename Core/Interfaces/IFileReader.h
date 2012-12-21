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


#ifndef READERFACTORY_H
#define READERFACTORY_H

#include <QtPlugin>

#include <Core/EspinaTypes.h>
#include <Core/Model/EspinaModel.h>

class QFileInfo;
class QString;
class QUndoStack;

namespace EspINA
{
typedef QString ReaderId;

class IFileReader
{
public:
  virtual ~IFileReader(){}

  virtual void initFileReader(EspinaModelSPtr model,
                              QUndoStack* undoStack,
                              ViewManager *vm) = 0;

  virtual bool readFile(const QFileInfo) = 0;
};

}// namespace EspINA

Q_DECLARE_INTERFACE(EspINA::IFileReader,
                    "es.upm.cesvima.EspINA.IFileReader/1.1")

#endif // READERFACTORY_H
