/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_VIEW2D_RENDERERS_PANEL_H_
#define ESPINA_VIEW2D_RENDERERS_PANEL_H_

// EspINA
#include <Support/Settings/SettingsPanel.h>
#include <GUI/Widgets/RenderersSelector.h>
#include <GUI/Representations/Renderers/Renderer.h>

namespace EspINA
{
  
  class View2DRenderersPanel
  : public SettingsPanel
  {
    Q_OBJECT
    public:
      /* \brief View2DRenderersPanel class constructor.
       * \param[in] renderers List of renderers
       * \param[in] activeRenderers Names of active renderers to show in active panel
       * \param[in] filter Specifies the type of filters to show in the panels.
       * \param[in] viewList List of View2D views, its here only to apply changes and clone.
       */
      explicit View2DRenderersPanel(RendererSList renderers,
                                    QStringList activeRenderers,
                                    RendererTypes filter,
                                    QList<View2D *> viewList);

      /* \brief View2DRenderersPanel class destructor.
       *
       */
      virtual ~View2DRenderersPanel();

      virtual const QString shortDescription()
      { return QString("View 2D"); };

      virtual const QString longDescription()
      { return QString("View 2D"); };

      virtual const QIcon icon()
      { return QIcon(); }

      virtual void acceptChanges();

      virtual void rejectChanges();

      virtual bool modified() const;

      virtual SettingsPanelPtr clone();

    private:
      RendererSList      m_renderers;
      QStringList        m_activeRenderers;
      RendererTypes      m_filter;
      QList<View2D *>    m_views;
      RenderersSelector *m_selector;
  };

} // namespace EspINA

#endif // ESPINA_VIEW2D_RENDERERS_PANEL_H_
