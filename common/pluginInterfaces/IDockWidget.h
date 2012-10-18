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

#ifndef IDOCKWIDGET_H
#define IDOCKWIDGET_H

#include <QtPlugin>
#include <QDockWidget>

class EspinaModel;
class ViewManager;
class QUndoStack;

class IDockWidget
: public QDockWidget
{
public:
  explicit IDockWidget(QWidget* parent = 0)
  : QDockWidget(parent){}
  virtual ~IDockWidget(){}

  virtual void initDockWidget(EspinaModel *model,
                              QUndoStack  *undoStack,
                              ViewManager *viewManager) = 0;
  // Reset All Components in the Dock Widget
  virtual void reset() = 0;
};

Q_DECLARE_INTERFACE(IDockWidget,
                    "es.upm.cesvima.EspINA.DockWidgetInterface/1.2")
#endif //IDOCKWIDGET_H
