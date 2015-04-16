/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_GUI_VIEW_WIDGETS_SELECTION_MEASURE_VTK_WIDGET_2D_H_
#define ESPINA_GUI_VIEW_WIDGETS_SELECTION_MEASURE_VTK_WIDGET_2D_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Bounds.h>

// VTK
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkAxisActor2D;
class vtkRenderer;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace SelectionMeasure
        {
          class EspinaGUI_EXPORT vtkWidget2D
          : public vtkAbstractWidget
          {
          public:
            /** \brief VTK-style New() class method.
             *
             */
            static vtkWidget2D *New();

            vtkTypeMacro(vtkWidget2D,vtkAbstractWidget);

            virtual void CreateDefaultRepresentation();

            virtual void SetEnabled(int enabled);

            /** \brief Sets actors' bounds.
             * \param[in] bounds Bounds to pass to actor.
             *
             */
            void setBounds(Bounds bounds);

            /** \brief Returns current bounds.
             *
             */
            Bounds bounds()
            { return m_bounds;}

            /** \brief Sets the plane this widget operates on.
             * \param[in] plane
             */
            void setPlane(Plane plane)
            { m_plane = plane; }

            /** \brief Recomputes the actors and updates the screen.
             *
             */
            void drawActors();

          protected:
            /** \brief Transforms from world coordinates to normalized view coordinates.
             *
             */
            void transformCoordsWorldToNormView(Nm *inout);

            /** \brief vtkWidget2D class constructor. Protected.
             *
             */
            explicit vtkWidget2D();

            /** \brief vtkWidget2D class destructor. Protected.
             *
             */
            virtual ~vtkWidget2D();

          private:
            void setTitle(vtkAxisActor2D *actor, const Axis axis);

          private:
            bool   m_enabled;
            Plane  m_plane;
            Bounds m_bounds;

            vtkSmartPointer<vtkAxisActor2D> m_up;
            vtkSmartPointer<vtkAxisActor2D> m_right;
          };

        }
      }
    }
  }
} /* namespace ESPINA */
#endif /* VTKRULERWIDGET_H_ */
