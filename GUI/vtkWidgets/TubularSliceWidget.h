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

#ifndef SLICEDTUBULARWIDGET_H
#define SLICEDTUBULARWIDGET_H

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/vtkWidgets/EspinaWidget.h>

namespace EspINA
{
  //Forward Declarations
  class vtkTubularWidget;

  class EspinaGUI_EXPORT TubularSliceWidget
  : public SliceWidget
  {
    public:
      explicit TubularSliceWidget(vtkTubularWidget *widget);

      virtual void setSlice(Nm pos, PlaneType plane);

    private:
      vtkTubularWidget *m_sliceWidget;
  };
}
#endif // SLICEDTUBULARWIDGET_H