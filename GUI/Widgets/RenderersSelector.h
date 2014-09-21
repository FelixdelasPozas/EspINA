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

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Representations/Renderers/Renderer.h>
#include "ui_RenderersSelector.h"

namespace ESPINA
{

  class EspinaGUI_EXPORT RenderersSelector
  : public QWidget
  , public Ui_RenderersSelector
  {
    Q_OBJECT
    public:
      /** \brief RenderersSelector class constructor.
       * \param[in] renderers, List of Renderer smart pointers.
       * \param[in] activeRenderers, List of renderers' names to show as active.
       * \param[in] filter, Types of renderers to show in the widgets.
       */
      explicit RenderersSelector(RendererSList renderersList,
                                 QStringList   activeRenderersList,
                                 RendererTypes filter);

      /** \brief RenderersSelector class destructor.
       *
       */
      virtual ~RenderersSelector();

      /** \brief Returns the renderers actually in the active list of the widgets.
       *
       */
      RendererSList getActiveRenderers();

    private slots:
			/** \brief Update the internal data when a renderer is dropped on the active widget.
			 *
			 */
      void onActivateRenderersDropped();

			/** \brief Update the internal data when a renderer is dropped on the inactive widget.
			 *
			 */
      void onAvailableRenderersDropped();

			/** \brief Activates the renderes in the active list.
			 *
			 */
      void activateRenderers();

			/** \brief Deactivates the renderers on the inactive list.
			 *
			 */
      void deactivateRenderers();

			/** \brief Moves the elements of the source to the destination view.
			 * \param[in] source, source list view.
			 * \param[in] destination, destination list view.
			 *
			 */
      void moveSelection(QListView *source, QListView *destination);

    protected:
      /** \brief Returns the smart pointer of the renderer with the given name.
       * \param[in] name, renderer name.
       *
       */
      RendererSPtr renderer(const QString& name) const;

      RendererSList m_renderers;
      QStringList   m_activeRenderers;
  };

} // namespace ESPINA

#endif // ESPINA_RENDERERS_SELECTOR_H_
