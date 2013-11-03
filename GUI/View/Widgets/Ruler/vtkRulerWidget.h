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

#ifndef VTKRULERWIDGET_H_
#define VTKRULERWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <Core/EspinaTypes.h>

// VTK
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>
#include <vtkCommand.h>

class vtkAxisActor2D;
class vtkRenderer;

namespace EspINA
{
  class RulerCommand;

  class EspinaGUI_EXPORT vtkRulerWidget
  : public vtkAbstractWidget
  {
    public:
      static vtkRulerWidget *New();

      vtkTypeMacro(vtkRulerWidget,vtkAbstractWidget);

      // Description:
      // Create the default widget representation if one is not set.
      void CreateDefaultRepresentation();

      // Description:
      // The method for activating and deactivating this widget. This method
      // must be overridden because it is a composite widget and does more than
      // its superclasses' vtkAbstractWidget::SetEnabled() method.
      virtual void SetEnabled(int);

      // set actors bounds
      void setBounds(Nm *bounds);
      void bounds(Nm bounds[6])
      { memcpy(bounds, m_bounds, 6*sizeof(Nm));}

      void setPlane(Plane plane) { m_plane = plane; }
      void drawActors();

    protected:
      void transformCoordsWorldToNormView(Nm *inout);

      vtkRulerWidget();
      ~vtkRulerWidget();

      bool m_enabled;
      PlaneType m_plane;
      Nm m_bounds[6];

      vtkSmartPointer<vtkAxisActor2D> m_up;
      vtkSmartPointer<vtkAxisActor2D> m_right;
      RulerCommand                   *m_command;
  };

  class RulerCommand
  : public vtkCommand
  {
    public:
      explicit RulerCommand();
      virtual ~RulerCommand() {};

      vtkTypeMacro(RulerCommand, vtkCommand);

      // implements vtkCommand
      void Execute(vtkObject *, unsigned long int, void*);

      void setRenderer(vtkRenderer* renderer)
      { m_renderer = renderer; }

      void setWidget(vtkRulerWidget *widget)
      { m_widget = widget; }

    private:
      vtkRenderer *m_renderer;
      vtkRulerWidget *m_widget;
  };


} /* namespace EspINA */
#endif /* VTKRULERWIDGET_H_ */
