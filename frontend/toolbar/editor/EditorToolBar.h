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


#ifndef EDITORTOOLBAR_H
#define EDITORTOOLBAR_H

#include <QToolBar>
#include <common/gui/DynamicWidget.h>

#include <selection/SelectionHandler.h>

class PixelSelector;
class EditorToolBar
: public QToolBar
, public DynamicWidget
{
  Q_OBJECT
public:
  explicit EditorToolBar(QWidget *parent = 0);

  virtual void setActivity(QString activity);
  virtual void setLOD();
  virtual void decreaseLOD();
  virtual void increaseLOD();

protected slots:
  void combineSegmentations();
  void substractSegmentations();

private:
  QAction *m_addition;
  QAction *m_substraction;
};

#endif // EDITORTOOLBAR_H
