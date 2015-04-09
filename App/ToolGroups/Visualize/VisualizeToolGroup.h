/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <@>

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

#ifndef ESPINA_EXPLORE_TOOL_GROUP_H
#define ESPINA_EXPLORE_TOOL_GROUP_H

#include <ToolGroups/ToolGroup.h>

// ESPINA
#include "RepresentationsGroupTool.h"
#include <Support/Representations/RepresentationUtils.h>

// Qt
#include <QShortcut>

namespace ESPINA
{
  class VisualizeToolGroup
  : public ToolGroup
  {
    Q_OBJECT

  public:
    /** \brief ViewTools class constructor.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] parent raw pointer to the QWidget parent of this object.
     */
    explicit VisualizeToolGroup(Support::Context &context, QWidget *parent);

    /** \brief ViewTools class virtual destructor.
     *
     */
    virtual ~VisualizeToolGroup();

    /** \brief Add render switch to group render group tool
     *
     */
    void addRepresentationSwitch(RepresentationGroup      group,
                                 RepresentationSwitchSPtr repSwitch,
                                 const QIcon             &groupIcon        = QIcon(),
                                 const QString           &groupDescription = QString());

  private:
    class SettingsTool;
    using SettingsToolSPtr = std::shared_ptr<SettingsTool>;

    using RepresentationGroupTools = QMap<RepresentationGroup, RepresentationGroupToolSPtr>;

    RepresentationGroupToolSPtr m_channelsRepGroup;
    RepresentationGroupToolSPtr m_segmentationsRepGroup;
    RepresentationGroupTools    m_dynamicRepresentationGroups;

    Support::Context &m_context;

    QShortcut *m_segmentationsShortcut;
    QShortcut *m_crosshairShortcut;
  };

} // namespace ESPINA

#endif // ESPINA_EXPLORE_TOOL_GROUP_H
