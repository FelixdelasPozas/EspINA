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


#ifndef LODTOOLBAR_H
#define LODTOOLBAR_H

#include <QToolBar>
#include "common/gui/DynamicWidget.h"


class LODToolBar
: public QToolBar
{
  Q_OBJECT
public:
  explicit LODToolBar(QWidget* parent = 0);
  virtual ~LODToolBar();

  virtual void setActivity(QString activity);
  virtual void setLOD();
  
public slots:
  virtual void decreaseLOD();
  virtual void increaseLOD();

private:
  QAction *m_decreaseLOD;
  QAction *m_increaseLOD;
};

#endif // LODTOOLBAR_H
