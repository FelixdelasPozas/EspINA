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


#ifndef ESPINA_TOOL_GROUP_H
#define ESPINA_TOOL_GROUP_H

#include "Support/EspinaSupport_Export.h"

#include "Support/Tool.h"

#include <GUI/Selectors/Selector.h>
#include "Support/ViewManager.h"

#include <memory>

#include <QAction>

namespace EspINA
{
  class RenderView;

  class EspinaSupport_EXPORT ToolGroup
  : public QAction
  {
    Q_OBJECT
  public:
    ToolGroup(ViewManagerSPtr viewManager, const QIcon& icon, const QString& text, QObject* parent);

    virtual void setEnabled(bool value) = 0;

    virtual bool enabled() const = 0;

    virtual ToolSList tools() = 0;

  public slots:
    virtual void showTools(bool value);

  protected:
    ViewManagerSPtr m_viewManager;
  };

  using ToolGroupPtr = ToolGroup *;
} // namespace EspINA

#endif // ESPINA_TOOL_GROUP_H
