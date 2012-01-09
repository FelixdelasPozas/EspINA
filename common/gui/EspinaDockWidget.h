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
// File:    EspinaDockWidget.h
// Purpose: Extends Qt Dock Widget to deal with Espina LOD/Activities
//----------------------------------------------------------------------------
#ifndef ESPINADOCKWIDGET_H
#define ESPINADOCKWIDGET_H

#include <QDockWidget>
#include "DynamicWidget.h"


class EspinaDockWidget : public QDockWidget, public DynamicWidget
{

public:
  EspinaDockWidget(QWidget *parent = 0);
  virtual ~EspinaDockWidget();

    virtual void increaseLOD(){}
    virtual void decreaseLOD(){}
    virtual void setLOD(){}
    virtual void setActivity(QString activity){}

protected:
//   QSharedPointer<DynamicWidgetState> m_state;
};

#endif // ESPINADOCKWIDGET_H
