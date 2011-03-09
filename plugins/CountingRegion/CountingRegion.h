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


#ifndef COUNTINGREGION_H
#define COUNTINGREGION_H

#include <QActionGroup>

// Forward declaration
class QAction;

class CountingRegion : public QActionGroup
{
  Q_OBJECT
  
public:
  CountingRegion(QObject* parent);

public slots:
  //void onAction(QAction *action);
  
  void buildUI();

};

#endif // COUNTINGREGION_H
