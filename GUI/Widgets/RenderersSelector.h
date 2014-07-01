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

#ifndef ESPINA_RENDERERS_SELECTOR_H_
#define ESPINA_RENDERERS_SELECTOR_H_

// EspINA
#include <Support/Settings/SettingsPanel.h>
#include <GUI/Representations/Renderers/Renderer.h>
#include "ui_RenderersSelector.h"

namespace EspINA
{
  
  class RenderersSelector
  : public QWidget
  , public Ui_RenderersSelector
  {
    Q_OBJECT
    public:
      /* \brief RenderersSelector class constructor.
       * \param[in] renderers List of renderers.
       * \param[in] activeRenderers List of renderers to show as active.
       * \param[in] filter Types of renderers to show in the widgets.
       */
      explicit RenderersSelector(RendererSList renderersList,
                                 QStringList   activeRenderersList,
                                 RendererTypes filter);

      /* \brief RenderersSelector class destructor.
       *
       */
      virtual ~RenderersSelector();

      /* \brief Returns the renderers actually in the active list of the widgets.
       *
       */
      RendererSList getActiveRenderers();

    private slots:
      void onActivateRenderersDropped();
      void onAvailableRenderersDropped();
      void activateRenderers();
      void deactivateRenderers();
      void moveSelection(QListView *source, QListView *destination);

    protected:
      RendererSPtr renderer(const QString& name) const;

      RendererSList m_renderers;
      QStringList   m_activeRenderers;
  };

} // namespace EspINA

#endif // ESPINA_RENDERERS_SELECTOR_H_
