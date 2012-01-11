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
// File:    Layout.h
// Purpose: Group different views and the way they are displayed
//          (i.e. main window widget, dock widgets, independent widget, etc)
//----------------------------------------------------------------------------

#ifndef LAYOUT_H
#define LAYOUT_H

#include <QWidget>

// Forward-declaration
class QDockWidget;
class QMainWindow;
class SliceView;

class Layout : public QWidget
{
  Q_OBJECT
public:
  explicit Layout(QMainWindow * parent = 0, Qt::WindowFlags f = 0);
  virtual ~Layout(){}

public slots:
  virtual void setShowSegmentations(bool visibility) = 0;
};

class DefaultLayout : public Layout
{
public:
  explicit DefaultLayout(QMainWindow* parent = 0, Qt::WindowFlags f = 0);

  virtual void setShowSegmentations(bool visibility);
private:
  SliceView *xyView, *yzView, *xzView;
  QSharedPointer<QDockWidget> volDock, yzDock, xzDock;
};

#endif // LAYOUT_H
