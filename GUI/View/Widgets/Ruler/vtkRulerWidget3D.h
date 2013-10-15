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

#ifndef VTKRULERWIDGET3D_H_
#define VTKRULERWIDGET3D_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <Core/EspinaTypes.h>

//VTK
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkCubeAxesActor2D;

namespace EspINA
{
  class EspinaGUI_EXPORT vtkRulerWidget3D
  : public vtkAbstractWidget
  {
    public:
      static vtkRulerWidget3D *New();

      vtkTypeMacro(vtkRulerWidget3D,vtkAbstractWidget);

      // Description:
      // Create the default widget representation if one is not set.
      void CreateDefaultRepresentation();

      // Description:
      // The method for activating and deactivating this widget. This method
      // must be overridden because it is a composite widget and does more than
      // its superclasses' vtkAbstractWidget::SetEnabled() method.
      virtual void SetEnabled(int);

      // Set vtkCubeAxesActor2D representation bounds
      void setBounds(Nm *bounds);

    protected:
      vtkRulerWidget3D();
      ~vtkRulerWidget3D();

      bool m_enabled;
      vtkSmartPointer<vtkCubeAxesActor2D> m_actor;
  };

} /* namespace EspINA */
#endif /* VTKRULERWIDGET3D_H_ */
