/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef ESPINA_SPLIT_TOOL_H_
#define ESPINA_SPLIT_TOOL_H_

#include <Support/Tool.h>

#include <GUI/Widgets/ActionSelector.h>

class QAction;

namespace EspINA
{
  
  class SplitTool
  : public Tool
  {
    public:
      SplitTool();
      virtual ~SplitTool();

      virtual void setEnabled(bool value);

      virtual bool enabled() const;

      virtual QList<QAction *> actions() const;

    private:
      ActionSelector *m_splitToolSelector;
      QMap<QAction *, ToolSPtr> m_splitTools;

      bool m_enabled;
  };

  using SplitToolPtr  = SplitTool *;
  using SplitToolSPtr = std::shared_ptr<SplitTool>;

} // namespace EspINA

#endif // ESPINA_SPLIT_TOOL_H_
