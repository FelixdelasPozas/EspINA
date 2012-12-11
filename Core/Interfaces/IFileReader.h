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

class EspinaModel;
class QFileInfo;
class QString;
class QUndoStack;
class ViewManager;

typedef QString ReaderId;

class IFileReader
{
public:
  virtual ~IFileReader(){}

  virtual void initFileReader(EspinaModel* model,
                              QUndoStack* undoStack,
                              ViewManager *vm) = 0;

  virtual bool readFile(const QFileInfo) = 0;
};

Q_DECLARE_INTERFACE(IFileReader,
                    "es.upm.cesvima.EspINA.IFileReader/1.0")

#endif // READERFACTORY_H