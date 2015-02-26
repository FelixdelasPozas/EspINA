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
#include <Support/Widgets/ToolGroup.h>
#include "ResetZoom.h"
#include "ZoomAreaTool.h"
#include "RepresentationsGroupTool.h"
#include "ToggleCrosshairVisibility.h"

// Qt
#include <QShortcut>

namespace ESPINA
{
  class ViewToolGroup
  : public ToolGroup
  {
    Q_OBJECT

  public:
    using RenderGroup = QString;

    static RenderGroup CHANNELS_GROUP;
    static RenderGroup SEGMENTATIONS_GROUP;

  public:
    /** \brief ViewTools class constructor.
     * \param[in] viewManager view manager smart pointer.
     * \param[in] parent raw pointer to the QWidget parent of this object.
     */
    explicit ViewToolGroup(ViewManagerSPtr viewManager, QWidget *parent = 0);

    /** \brief ViewTools class virtual destructor.
     *
     */
    virtual ~ViewToolGroup();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual ToolSList tools();

    /** \brief Add render switch to group render group tool
     *
     */
    void addRepresentationSwitch(RenderGroup              group,
                                 RepresentationSwitchSPtr repSwitch,
                                 QIcon                    groupIcon        = QIcon(),
                                 const QString           &groupDescription = QString());

  public slots:
    /** \brief Aborts current operation.
     *
     */
    void abortOperation();

  private:
    class SettingsTool;
    using SettingsToolSPtr = std::shared_ptr<SettingsTool>;

    using RenderGroupTools = QMap<RenderGroup, RenderGroupToolSPtr>;

    ToggleCrosshairVisibilitySPtr     m_toggleCrosshair;
    ResetZoomSPtr                     m_resetZoom;
    ZoomAreaToolSPtr                  m_zoomArea;
    SettingsToolSPtr                  m_renderSettings;
    RenderGroupToolSPtr               m_channelsRenderGroup;
    RenderGroupToolSPtr               m_segmentationsRenderGroup;
    RenderGroupTools                  m_dynamicRenderGroups;

    bool m_enabled;

    QShortcut *m_segmentationsShortcut;
    QShortcut *m_crosshairShortcut;
  };

} // namespace ESPINA

#endif // ESPINA_ZOOM_TOOL_GROUP_H
