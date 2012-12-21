/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012 Félix de las Pozas Álvarez <@>

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

#ifndef ZOOMTOOLBAR_H_
#define ZOOMTOOLBAR_H_

#include <Core/Interfaces/IToolBar.h>
#include <Tools/Zoom/ZoomTool.h>

class QAction;

namespace EspINA
{
  class ViewManager;
  class ITool;
  class ZoomTool;

  class ZoomToolBar
  : public IToolBar
  {
    Q_OBJECT

  public:
    explicit ZoomToolBar(ViewManager *viewManager,
                         QWidget     *parent = 0);
    virtual ~ZoomToolBar();

    virtual void initToolBar(EspinaModelSPtr model,
                             QUndoStack     *undoStack,
                             ViewManager    *viewManager);
  public slots:
    virtual void resetViews();
    virtual void initZoomTool(bool);

    virtual void reset();

  private:
    ViewManager *m_viewManager;

    ZoomToolSPtr m_zoomTool;

    QAction     *m_resetViews;
    QAction     *m_zoomToolAction;
  };

} // namespace EspINA

#endif /* ZOOMTOOLBAR_H_ */
