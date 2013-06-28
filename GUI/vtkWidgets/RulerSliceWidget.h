/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef RULERSLICEWIDGET_H_
#define RULERSLICEWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "EspinaWidget.h"

namespace EspINA
{
  class EspinaGUI_EXPORT RulerSliceWidget
  : public SliceWidget
  {
    public:
      explicit RulerSliceWidget(vtkAbstractWidget*);
      virtual ~RulerSliceWidget();

      virtual void setEnabled(int);
      virtual void setBounds(Nm *bounds);

      virtual void setSlice(Nm pos, PlaneType plane);

    private:
      Nm        m_pos;
      PlaneType m_plane;

      bool m_insideBounds;
      bool m_enabled;
  };

} /* namespace EspINA */
#endif /* RULERSLICEWIDGET_H_ */
