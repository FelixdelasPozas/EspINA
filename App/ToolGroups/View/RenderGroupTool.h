/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_RENDER_GROUP_TOOL_H
#define ESPINA_RENDER_GROUP_TOOL_H

#include <Support/RenderSwitch.h>
#include <Support/Widgets/Tool.h>
#include <QIcon>

class QWidgetAction;

namespace ESPINA
{
  class RenderGroupTool
  : public Tool
  {
    Q_OBJECT
  public:
    explicit RenderGroupTool(QIcon icon, QString description);

    virtual void setEnabled ( bool value ) override;

    virtual bool enabled() const override;

    virtual QList<QAction *> actions() const override;

    /** \brief Add render switch to this render tool group
     *
     */
    void addRenderSwitch(RenderSwitchSPtr renderSwitch);

    void showRenders(ViewTypeFlags views);

  private slots:
    void onToolToggled(bool toggled);

  private:
    QAction                *m_globalSwitch;
    QWidgetAction          *m_content;
    QWidget                *m_contentWidget;
    QList<RenderSwitchSPtr> m_switches;

    ViewTypeFlags m_viewFlags;
  };

  using RenderGroupToolSPtr  = std::shared_ptr<RenderGroupTool>;
  using RenderGroupToolSList = QList<RenderGroupToolSPtr>;
}

#endif // ESPINA_RENDER_GROUP_TOOL_H
