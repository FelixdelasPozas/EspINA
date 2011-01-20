/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include <qt4/QtCore/QObject>

//TODO: Temporary data type until proper definition
struct Point
{
  int x;
  int y;
  int z;
};

class ISelectableObject
{
public:
  ISelectableObject(){}
  virtual ~ISelectableObject(){}
};

//! Tuple containing the selected object and its selected coordinate
struct Selection 
{
  Point coord;
  ISelectableObject *object;
};

//! Interface to handle selections
class ISelectionHandler
{
  
public:
  ISelectionHandler(){};
  virtual ~ISelectionHandler() = 0;

  //! Handles @sel
  virtual void handle(const Selection sel) = 0;
  virtual void abortSelection() = 0;
};


class SelectionManager : public QObject
{
  Q_OBJECT

private:
  SelectionManager();
  ~SelectionManager(){}
  
public slots:
  //! Register @sh as active Selection Handler
  void setSelectionHandler(ISelectionHandler *sh);
  
  /*! Creates a list of selected objects and handles
  *  the selection.
  */
  void pointSelected(const Point coord);
  
  //! Returns a SelectionManager singleton
  static SelectionManager *singleton(){return m_singleton;}
  
private:
  ISelectionHandler *m_sh;
  static SelectionManager *m_singleton;
};

#endif // SELECTIONMANAGER_H
