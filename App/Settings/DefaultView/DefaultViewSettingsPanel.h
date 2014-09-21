/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Support/Settings/SettingsPanel.h>
#include <GUI/View/View2D.h>
#include <GUI/View/View3D.h>
#include <Menus/RenderersMenu.h>
#include <Settings/View2D/View2DRenderersPanel.h>
#include <Settings/View2D/View2DSettingsPanel.h>
#include <Settings/View3D/View3DSettingsPanel.h>

namespace ESPINA
{

  class DefaultViewSettingsPanel
  : public SettingsPanel
  {
  public:
 		/** brief DefaultViewSettingsPanel class constructor.
 		 * \param[in] viewXY, raw pointer of the XY view.
 		 * \param[in] viewXZ, raw pointer of the XZ view.
 		 * \param[in] viewYZ, raw pointer of the YZ view.
 		 * \param[in] view3D, raw pointer of the 3D view.
 		 * \param[in] renderers, list of renderer smart pointers.
 		 * \param[in] menu, raw pointer of the renderers menu.
		 *
 		 */
    explicit DefaultViewSettingsPanel(View2D *viewXY,
                                      View2D* viewXZ,
                                      View2D* viewYZ,
                                      View3D* view3D,
                                      RendererSList renderers,
                                      RenderersMenu *menu);

    /** brief Overrides SettingsPanel::shortDescription().
     *
     */
    virtual const QString shortDescription() override
    { return QObject::tr("View"); }

    /** brief Overrides SettingsPanel::longDescription().
     *
     */
    virtual const QString longDescription() override
    { return QObject::tr("%1").arg(shortDescription()); }

    /** brief Overrides SettingsPanel::icon().
     *
     */
    virtual const QIcon icon() override
    {return QIcon(":/espina/show_all.svg");}

    /** brief Overrides SettingsPanel::acceptChanges().
     *
     */
    virtual void acceptChanges() override;

    /** brief Overrides SettingsPanel::rejectChanges().
     *
     */
    virtual void rejectChanges() override;

    /** brief Overrides SettingsPanel::modified().
     *
     */
    virtual bool modified() const override;

    /** brief Overrides SettingsPanel::clone().
     *
     */
    virtual SettingsPanelPtr clone() override;

  private:
    View2D *m_viewXY;
    View2D *m_viewXZ;
    View2D *m_viewYZ;
    View3D *m_view3D;

    RendererSList m_renderers;

    View2DSettingsPanel  *m_panelXY, *m_panelXZ, *m_panelYZ;
    View3DSettingsPanel  *m_panel3D;
    View2DRenderersPanel *m_panel2D;
    RenderersMenu        *m_menu;
  };
}

#endif // ESPINA_DEFAULT_VIEWSETTINGS_PANEL_H
