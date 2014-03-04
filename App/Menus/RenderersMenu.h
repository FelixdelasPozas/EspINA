/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include <GUI/Representations/Renderers/Renderer.h>
#include <Support/ViewManager.h>

// Qt
#include <QMenu>

class QWidget;
class QString;
class QAction;

namespace EspINA
{
  
  class RenderersMenu: public QMenu
  {
    Q_OBJECT
    public:
      explicit RenderersMenu(ViewManagerSPtr vm,
                             QWidget* parent);
      virtual ~RenderersMenu();

      void addRenderer(RendererSPtr renderer);
      void removeRenderer(RendererSPtr renderer);

      void clear();

    protected slots:
      void activate(QAction *);

    private:
      ViewManagerSPtr m_viewManager;
      QMenu          *m_menu2D;
      QMenu          *m_menu3D;
  };

} /* namespace EspINA */

#endif /* RENDERERSMENU_H_ */
