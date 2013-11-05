/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef ESPINA_SLICE_SELECTOR_WIDGET_H
#define ESPINA_SLICE_SELECTOR_WIDGET_H

#include "EspinaGUI_Export.h"

#include <Core/Utils/Spatial.h>
#include <QObject>

namespace EspINA
{
  class View2D;

  class EspinaGUI_EXPORT SliceSelectorWidget
  : public QObject
  {
  public:
    virtual ~SliceSelectorWidget() {}

    virtual void setPlane(const Plane plane)
    { m_plane = plane; }

    virtual void setView(SliceView* view)
    { m_view = view; }

    virtual QWidget* leftWidget()  const = 0;
    virtual QWidget* rightWidget() const = 0;

    virtual SliceSelectorWidget *clone() = 0;

  protected:
    explicit SliceSelectorWidget()
    : m_plane(Plane::XY), m_view(nullptr) {}

    Plane      m_plane;
    SliceView* m_view;
  };

} // namespace EspINA

#endif // ESPINA_SLICE_SELECTOR_WIDGET_H
