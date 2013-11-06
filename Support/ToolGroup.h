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

#include "EspinaSupport_Export.h"
#include "Tool.h"
#include <GUI/Selectors/Selector.h>

#include <memory>

#include <QAction>

namespace EspINA
{
  class RenderView;

  class EspinaSupport_EXPORT ToolGroup
  : public QAction
  {
  public:
    virtual void setInUse(bool value) = 0;

    virtual void setEnabled(bool value) = 0;

    virtual bool enabled() const = 0;

    virtual void setActiveTool(ToolSPtr tool) = 0;

    virtual SelectorSPtr selector() const = 0;
//     { return m_selector; }
// 
//   protected:
//     SelectorSPtr m_selector;
  };

  using ToolGroupSPtr = std::shared_ptr<ToolGroup>;
} // namespace EspINA

#endif // ESPINA_TOOL_H
