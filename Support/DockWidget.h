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

#ifndef ESPINA_DOCK_WIDGET_H
#define ESPINA_DOCK_WIDGET_H

#include "Support/EspinaSupport_Export.h"
#include <GUI/Model/ModelAdapter.h>
#include "ViewManager.h"

#include <QtPlugin>
#include <QDockWidget>

class QUndoStack;

namespace EspINA
{
  class EspinaSupport_EXPORT DockWidget
  : public QDockWidget
  {
    Q_OBJECT
  public:
    explicit DockWidget(QWidget* parent = 0)
    : QDockWidget(parent) {}

    virtual ~DockWidget(){}

    virtual void initDockWidget(ModelAdapterSPtr model,
                                QUndoStack      *undoStack,
                                ViewManagerSPtr  viewManager) = 0;
  public slots:
    virtual void reset() = 0;
  };
}

Q_DECLARE_INTERFACE(EspINA::DockWidget,
                    "es.upm.cesvima.EspINA.DockWidgetInterface/2.0")

#endif // ESPINA_DOCK_WIDGET_H