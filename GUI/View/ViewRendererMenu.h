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

#ifndef ESPINA_VIEW_RENDERER_MENU_H_
#define ESPINA_VIEW_RENDERER_MENU_H_

// ESPINA
#include <GUI/Representations/Renderers/Renderer.h>

// Qt
#include <QMenu>
#include <QMap>

class QAction;

namespace ESPINA
{

  class EspinaGUI_EXPORT ViewRendererMenu
  : public QMenu
  {
    Q_OBJECT
    public:
			/** brief ViewRendererMenu class constructor.
			 *
			 */
      explicit ViewRendererMenu(QWidget *parent = nullptr);

      /** brief ViewRendererMenu class destructor.
       *
       */
      virtual ~ViewRendererMenu();

      /** brief Adds a renderer to the menu.
       * \param[in] renderer, smart pointer of the renderer to add.
       *
       */
      void add(RendererSPtr renderer);

      /** brief Removes a renderer to the menu.
       * \param[in] renderer, smart pointer of the renderer to remove.
       *
       */
      void remove(RendererSPtr renderer);

    public slots:
			/** brief Activates/deactivates the renderer associated with the given action.
			 * \param[in] action, QAction raw pointer.
			 *
			 */
      void activate(QAction *action);

    private:
      QMap<QAction *, RendererSPtr> m_renderers;
  };

} // namespace ESPINA

#endif // ESPINA_VIEW_RENDERER_MENU_H_
