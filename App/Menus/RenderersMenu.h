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

#ifndef RENDERERSMENU_H_
#define RENDERERSMENU_H_

// ESPINA
#include <Deprecated/GUI/Representations/Renderers/Renderer.h>
#include <Support/ViewManager.h>

// Qt
#include <QMenu>

class QWidget;
class QString;
class QAction;

namespace ESPINA
{
  class RenderersMenu
  : public QMenu
  {
    Q_OBJECT
    public:
			/** \brief RenderersMenu class constructor.
			 * \param[in] vm, view manager smart pointer.
			 * \param[in] parent, QWidget raw pointer of the parent of the menu.
			 *
			 */
      explicit RenderersMenu(ViewManagerSPtr vm,
                             QWidget* parent);

      /** \brief RenderersMenu class virtual destructor.
       *
       */
      virtual ~RenderersMenu();

      /** \brief Adds a renderer to the menu.
       * \param[in] renderer, renderer smart pointer to add to the menu.
       *
       */
      void addRenderer(RendererSPtr renderer);

      /** \brief Removes a renderer from the menu.
       * \param[in] renderer, renderer smart pointer to remove from the menu.
       *
       */
      void removeRenderer(RendererSPtr renderer);

      /** \brief Shadows QMenu::clear().
       *
       */
      void clear();

    protected slots:
    	/** \brief Manages activation of one of the menu entries.
    	 *
    	 */
      void activate(QAction *);

    private:
      ViewManagerSPtr m_viewManager;
      QMenu          *m_menu2D;
      QMenu          *m_menu3D;
  };

} /* namespace ESPINA */

#endif /* RENDERERSMENU_H_ */
