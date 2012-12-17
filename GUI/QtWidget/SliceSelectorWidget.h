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


#ifndef SLICESELECTORWIDGET_H
#define SLICESELECTORWIDGET_H

#include <Core/EspinaTypes.h>
#include <QObject>

namespace EspINA
{
  class SliceView;
  class SliceSelectorWidget
  : public QObject
  {
  public:
    virtual ~SliceSelectorWidget() {}

    virtual void setPlane(const PlaneType plane) { m_plane = plane; }
    virtual void setView(SliceView *view) { m_view = view; }

    virtual QWidget *leftWidget() const = 0;
    virtual QWidget *rightWidget()   const = 0;

    virtual SliceSelectorWidget *clone() = 0;

  protected:
    explicit SliceSelectorWidget()
    : m_plane(AXIAL), m_view(NULL) {}

    PlaneType  m_plane;
    SliceView *m_view;
  };

} // namespace EspINA

#endif // SLICESELECTORWIDGET_H
