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

#ifndef ESPINA_REPRESENTATIONS_GROUP_TOOL_H
#define ESPINA_REPRESENTATIONS_GROUP_TOOL_H

#include <Support/Representations/RepresentationSwitch.h>
#include <GUI/Utils/Timer.h>
#include <Support/Widgets/Tool.h>
#include <QIcon>

class QWidgetAction;

namespace ESPINA
{
  class RepresentationsGroupTool
  : public Tool
  {
    Q_OBJECT

  public:
    explicit RepresentationsGroupTool(const QIcon &icon, QString description, Timer &timer);

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

    /** \brief Shows all active representations of the group
     *
     */
    void showActiveRepresentations();

    /** \brief Hides all active representations of the group
     *
     */
    void hideActiveRepresentations();

    /** \brief Add representation switch to this representation tool group
     *
     */
    void addRepresentationSwitch(RepresentationSwitchSPtr representationSwitch);

  public slots:
    void toggleRepresentationsVisibility();

  private:
    virtual void onToolEnabled(bool enabled) override;

    void changeSwitchStatus(RepresentationSwitchSPtr representationSwitch, bool showRepresentations);

  private slots:
    void setActiveRepresentationsVisibility(bool value);

  private:
    Timer                    &m_timer;
    QAction                  *m_globalSwitch;
    QWidgetAction            *m_content;
    QWidget                  *m_contentWidget;
    RepresentationSwitchSList m_switches;

    ViewTypeFlags m_viewFlags;

    bool m_representationsVisibility;
  };

  using RepresentationGroupToolSPtr  = std::shared_ptr<RepresentationsGroupTool>;
  using RepresentationGroupToolSList = QList<RepresentationGroupToolSPtr>;
}

#endif // ESPINA_REPRESENTATIONS_GROUP_TOOL_H
