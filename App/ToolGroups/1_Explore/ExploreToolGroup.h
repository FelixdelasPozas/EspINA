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

#ifndef ESPINA_ZOOM_TOOL_GROUP_H
#define ESPINA_ZOOM_TOOL_GROUP_H

// ESPINA
#include "ResetZoom.h"
#include "ZoomAreaTool.h"
#include "RepresentationsGroupTool.h"
#include "ToggleCrosshairVisibility.h"
#include <ToolGroups/ToolGroup.h>

// Qt
#include <QShortcut>

namespace ESPINA
{
  class ExploreToolGroup
  : public ToolGroup
  {
    Q_OBJECT

  public:
    using RepresentationGroup = QString;

    static RepresentationGroup CHANNELS_GROUP;
    static RepresentationGroup SEGMENTATIONS_GROUP;

  public:
    /** \brief ViewTools class constructor.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] parent raw pointer to the QWidget parent of this object.
     */
    explicit ExploreToolGroup(ViewManagerSPtr viewManager, QWidget *parent = 0);

    /** \brief ViewTools class virtual destructor.
     *
     */
    virtual ~ExploreToolGroup();

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

    using RenderGroupTools = QMap<RepresentationGroup, RenderGroupToolSPtr>;

    ToggleCrosshairVisibilitySPtr     m_toggleCrosshair;
    RenderGroupToolSPtr               m_channelsRepGroup;
    RenderGroupToolSPtr               m_segmentationsRepGroup;
    RenderGroupTools                  m_dynamicRepresentationGroups;

    bool m_enabled;

    QShortcut *m_segmentationsShortcut;
    QShortcut *m_crosshairShortcut;
  };

} // namespace ESPINA

#endif // ESPINA_ZOOM_TOOL_GROUP_H
