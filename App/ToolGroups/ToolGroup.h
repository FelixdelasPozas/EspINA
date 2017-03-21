/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Support/Widgets/ProgressTool.h>

// C++
#include <memory>

// Qt
#include <QAction>

class QToolBar;

namespace ESPINA
{
  class RenderView;

  /** \class ToolGroup
   * \brief Implements behaviour of a group of tools.
   *
   */
  class ToolGroup
  : public QAction
  {
      Q_OBJECT

    public:
      using GroupedTools = QList<Support::Widgets::ToolSList>;

    public:
      /** \brief ToolGroup class constructor.
       * \param[in] icon of the tool group.
       * \param[in] text of the tool group
       * \param[in] parent of the tool group
       *
       */
      ToolGroup(const QString& icon, const QString& text, QObject* parent = nullptr);

      /** \brief ToolGroup class virtual destructor.
       *
       */
      virtual ~ToolGroup()
      {}

      /** \brief Returns the tools contained by the tool group
       *
       */
      GroupedTools groupedTools() const;

      /** \brief Adds a tool to the tool group
       *  \param[in] tool to be added
       *
       */
      void addTool(Support::Widgets::ToolSPtr tool);

    public slots:
      /** \brief Aborts all the tools of the group.
       *
       */
      virtual void abortOperations();

      /** \brief Communicates the activation of a exclusive tool to the tools of this group.
       * \brief tool activated tool that is exclusive, that is, overrides other activated tools.
       *
       */
      void onExclusiveToolInUse(Support::Widgets::ProgressTool *tool);

    signals:
      void activated(ToolGroup *tool);

      void exclusiveToolInUse(Support::Widgets::ProgressTool *tool);

    private slots:
      void activate(bool value);

    private:
      /** \brief Additional operations when a tool is added to a group.
       * \param[in] tool tool added to the group.
       *
       */
      virtual void onToolAdded(Support::Widgets::ToolSPtr tool)
      {};

    private:
      QMap<QString, Support::Widgets::ToolSList> m_tools; /** id<->tool map. */
  };

  using ToolGroupPtr = ToolGroup *;

  /** \brief Helper method to populate a QToolbar with all the tool actions.
   * \param[in] bar QToolBar object.
   * \param[in] tools grouped tools.
   *
   */
  void populateToolBar(QToolBar *bar, ToolGroup::GroupedTools tools);

} // namespace ESPINA

#endif // ESPINA_TOOL_GROUP_H
