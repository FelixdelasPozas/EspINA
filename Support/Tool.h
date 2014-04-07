/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef ESPINA_TOOL_H
#define ESPINA_TOOL_H

#include "Support/EspinaSupport_Export.h"

#include <memory>

#include <QObject>
#include <QCursor>

class QEvent;
class QAction;

namespace EspINA
{
  class RenderView;

  class EspinaSupport_EXPORT Tool
  : public QObject
  {
    public:
      virtual void setEnabled(bool value) = 0;

      virtual bool enabled() const = 0;

      virtual QList<QAction *> actions() const = 0;

    signals:
      void changedActions();
  };

  using ToolSPtr  = std::shared_ptr<Tool>;
  using ToolSList = QList<ToolSPtr>;
} // namespace EspINA

#endif // ESPINA_TOOL_H
