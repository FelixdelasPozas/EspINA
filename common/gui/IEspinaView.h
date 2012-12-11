/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


//----------------------------------------------------------------------------
// File:    IEspinaView.h
// Purpose: Group different specific views and the way they are displayed
//          (i.e. main window widget, dock widgets, independent widget, etc)
//----------------------------------------------------------------------------

#ifndef IESPINAVIEW_H
#define IESPINAVIEW_H

#include <QString>

// Forward-declaration
class EspinaModel;
class ISettingsPanel;

class IEspinaView
{
public:
  explicit IEspinaView(){}
  virtual ~IEspinaView(){}

  virtual void updateSelection() = 0;
  virtual void updateSegmentationRepresentations() = 0;

/*TODO BUG 2012-10-05
signals:
  void statusMsg(QString);
  */
};

#endif //IESPINAVIEW_H
