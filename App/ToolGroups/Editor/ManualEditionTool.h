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

#ifndef ESPINA_MANUAL_EDITION_TOOL_H_
#define ESPINA_MANUAL_EDITION_TOOL_H_

#include <Support/Tool.h>

#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Selectors/Selector.h>

class QAction;

namespace EspINA
{
  class SpinBoxAction;
  
  class ManualEditionTool
  : public Tool
  {
    public:
      ManualEditionTool();
      virtual ~ManualEditionTool();

      virtual void setEnabled(bool value);

      virtual bool enabled() const;

      virtual QList<QAction *> actions() const;

    private:
      ActionSelector *m_drawToolSelector;
      QMap<QAction *, SelectorSPtr> m_drawTools;

      SpinBoxAction *m_radiusWidget;

      bool m_enabled;
  };

  using ManualEditionToolPtr = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace EspINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_
