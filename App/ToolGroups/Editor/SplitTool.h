/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/Widgets/ActionSelector.h>

#include <Support/Tool.h>
#include <Support/ViewManager.h>

#include <QUndoStack>

class QAction;

namespace EspINA
{
  
  class SplitTool
  : public Tool
  {
    public:
      SplitTool(ModelAdapterSPtr model,
                ModelFactorySPtr factory,
                ViewManagerSPtr  viewManager,
                QUndoStack      *undoStack);
      virtual ~SplitTool();

      virtual void setEnabled(bool value);

      virtual bool enabled() const;

      virtual QList<QAction *> actions() const;

      virtual void abortOperation();
    private:
      ActionSelector *m_splitToolSelector;
      QMap<QAction *, ToolSPtr> m_splitTools;

      ModelAdapterSPtr m_model;
      ModelFactorySPtr m_factory;
      ViewManagerSPtr  m_viewManager;
      QUndoStack      *m_undoStack;

      bool m_enabled;
  };

  using SplitToolPtr  = SplitTool *;
  using SplitToolSPtr = std::shared_ptr<SplitTool>;

} // namespace EspINA

#endif // ESPINA_SPLIT_TOOL_H_
