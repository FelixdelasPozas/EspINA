/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#ifndef ESPINA_DEFAULT_VIEWSETTINGS_PANEL_H
#define ESPINA_DEFAULT_VIEWSETTINGS_PANEL_H

#include <Support/Settings/SettingsPanel.h>

#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Settings/View2D/View2DSettingsPanel.h>
#include <Settings/View3D/View3DSettingsPanel.h>

namespace EspINA {

  class DefaultViewSettingsPanel
  : public SettingsPanel
  {
  public:
    explicit DefaultViewSettingsPanel(View2D *viewXY,
                                      View2D* viewXZ,
                                      View2D* viewYZ,
                                      View3D* view3D,
                                      RendererSList renderers);

    virtual const QString shortDescription()
    { return QObject::tr("View"); }

    virtual const QString longDescription()
    { return QObject::tr("%1").arg(shortDescription()); }

    virtual const QIcon icon()
    {return QIcon(":/espina/show_all.svg");}

    virtual void acceptChanges();

    virtual void rejectChanges();

    virtual bool modified() const;

    virtual SettingsPanelPtr clone();

  private:
    View2D *m_viewXY;
    View2D *m_viewXZ;
    View2D *m_viewYZ;
    View3D *m_view3D;

    RendererSList m_renderers;

    Nm m_slicingStep;

    View2DSettingsPanel *m_panelXY, *m_panelXZ, *m_panelYZ;
    View3DSettingsPanel *m_panel3D;
  };
}

#endif // ESPINA_DEFAULT_VIEWSETTINGS_PANEL_H
