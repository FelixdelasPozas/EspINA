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

#ifndef ESPINA_VIEW_RENDERER_MENU_H_
#define ESPINA_VIEW_RENDERER_MENU_H_

// EspINA
#include <GUI/Representations/Renderers/Renderer.h>

// Qt
#include <QMenu>
#include <QMap>

class QAction;

namespace EspINA
{
  
  class ViewRendererMenu
  : public QMenu
  {
    Q_OBJECT
    public:
      explicit ViewRendererMenu(QWidget *parent = nullptr);
      virtual ~ViewRendererMenu();

      void add(RendererSPtr renderer);
      void remove(RendererSPtr renderer);

    public slots:
      void activate(QAction *);

    signals:
    protected:
    private:
      QMap<QAction *, RendererSPtr> m_renderers;
  };

} // namespace EspINA

#endif // ESPINA_VIEW_RENDERER_MENU_H_
