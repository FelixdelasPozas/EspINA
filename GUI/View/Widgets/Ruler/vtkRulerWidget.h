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
#include <Core/Utils/Bounds.h>

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
      /* \brief VTK-style New() class method.
       *
       */
      static vtkRulerWidget *New();

      vtkTypeMacro(vtkRulerWidget,vtkAbstractWidget);

      /* \brief Implements vtkAbstractWidget::CreateDefaultRepresentation.
       *
       * Create the default widget representation if one is not set.
       */
      void CreateDefaultRepresentation();

      /* \brief Implements vtkAbstractWidget::SetEnabled.
       *
       * The method for activating and deactivating this widget. This method
       * must be overridden because it is a composite widget and does more than
       * its superclasses' vtkAbstractWidget::SetEnabled() method.
       */
      virtual void SetEnabled(int);

      /* \brief Sets actors' bounds.
       * \param[in] bounds Bounds to pass to actor.
       *
       */
      void setBounds(Bounds bounds);

      /* \brief Returns current bounds.
       *
       */
      Bounds bounds()
      { return m_bounds;}

      /* \brief Sets the plane this widget operates on.
       * \param[in] plane
       */
      void setPlane(Plane plane)
      { m_plane = plane; }

      /* \brief Recomputes the actors and updates the screen.
       *
       */
      void drawActors();

    protected:
      /* \brief Transforms from world coordinates to normalized view coordinates.
       *
       */
      void transformCoordsWorldToNormView(Nm *inout);

      /* \brief vtkRulerWidget class constructor. Protected.
       *
       */
      vtkRulerWidget();

      /* \brief vtkRulerWidget class destructor. Protected.
       *
       */
      ~vtkRulerWidget();

      bool m_enabled;
      Plane m_plane;
      Bounds m_bounds;

      vtkSmartPointer<vtkAxisActor2D> m_up;
      vtkSmartPointer<vtkAxisActor2D> m_right;
      RulerCommand                   *m_command;
  };

  class RulerCommand
  : public vtkCommand
  {
    public:
      /* \brief RulerCommand class constructor.
       *
       */
      explicit RulerCommand();

      /* \brief RulerCommand class destructor.
       *
       */
      virtual ~RulerCommand() {};

      vtkTypeMacro(RulerCommand, vtkCommand);

      /* \brief Implements vtkCommand::Execute.
       *
       */
      void Execute(vtkObject *, unsigned long int, void*);

      /* \brief Sets the renderer the actors will be inserted.
       *
       */
      void setRenderer(vtkRenderer* renderer)
      { m_renderer = renderer; }

      /* \brief Sets the vtkAbstractWidget of the command.
       *
       */
      void setWidget(vtkRulerWidget *widget)
      { m_widget = widget; }

    private:
      vtkRenderer *m_renderer;
      vtkRulerWidget *m_widget;
  };


} /* namespace EspINA */
#endif /* VTKRULERWIDGET_H_ */
