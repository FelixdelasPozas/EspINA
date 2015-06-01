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
#include <Support/Widgets/Tool.h>

// C++
#include <memory>

// Qt
#include <QAction>

namespace ESPINA
{
  class RenderView;

  class ToolGroup
  : public QAction
  {
    Q_OBJECT

  public:
    /** \brief ToolGroup class constructor.
     * \param[in] icon of the tool group.
     * \param[in] text of the tool group
     * \param[in] parent of the tool group
     *
     */
    ToolGroup(const QString& icon, const QString& text, QObject* parent = nullptr);

    virtual ~ToolGroup();

    /** \brief Returns the tools contained by the tool group
     *
     */
    ToolSList tools() const;

    /** \brief Adds a tool to the tool group
     *
     *  \param[in] tool to be added
     */
    void addTool(ToolSPtr tool);

  public slots:
    void abortOperations();

    void onExclusiveToolInUse(Support::Widgets::ProgressTool *tool);

  signals:
    void activated(ToolGroup *tool);

    void exclusiveToolInUse(Support::Widgets::ProgressTool *tool);

  private slots:
    void activate(bool value);

  private:
    virtual void onToolAdded(ToolSPtr tool) {}

  private:
    QMap<QString, ToolSList> m_tools;
  };

  using ToolGroupPtr = ToolGroup *;
} // namespace ESPINA

#endif // ESPINA_TOOL_GROUP_H
