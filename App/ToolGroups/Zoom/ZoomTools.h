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

#ifndef ESPINA_ZOOM_TOOLS_H
#define ESPINA_ZOOM_TOOLS_H

#include <Support/ToolGroup.h>
#include "ResetZoom.h"

namespace EspINA
{
  class ZoomTools
  : public ToolGroup
  {
    Q_OBJECT

  public:
    explicit ZoomTools(ViewManagerSPtr viewManager, QWidget *parent = 0);
    virtual ~ZoomTools();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual ToolSList tools();

  public slots:
    void resetViews();
    void initZoomTool(bool);

  private:
    ResetZoomSPtr m_resetZoom;
  };

} // namespace EspINA

#endif // ESPINA_ZOOM_TOOLS_H
