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

#ifndef VTKZOOMSELECTIONWIDGETREPRESENTATION_H_
#define VTKZOOMSELECTIONWIDGETREPRESENTATION_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "vtkZoomSelectionWidget.h"
#include <Core/Utils/Spatial.h>

// VTK
#include <vtkWidgetRepresentation.h>
#include <vtkSmartPointer.h>

class vtkPoints;
class vtkActor;
namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {

        class EspinaGUI_EXPORT vtkZoomSelectionWidgetRepresentation
        : public vtkWidgetRepresentation
        {
          vtkTypeMacro(vtkZoomSelectionWidgetRepresentation,vtkWidgetRepresentation);

          public:
            static vtkZoomSelectionWidgetRepresentation *New();

            void SetWidgetType(vtkZoomSelectionWidget::WidgetType type);
            void SetRepresentationDepth(Nm depth);

            /** \brief Sets the slice where the representation should be.
             * \param[in] slice slice position in Nm.
             *
             */
            void SetSlice(Nm slice);

            // Description:
            // These are methods that satisfy vtkWidgetRepresentation's API.
            virtual void BuildRepresentation();
            virtual int ComputeInteractionState(int X, int Y, int modify = 0);
            virtual void StartWidgetInteraction(double e[2]);
            virtual void WidgetInteraction(double e[2]);
            virtual void EndWidgetInteraction(double e[2]);

            // Description:
            // Methods required by vtkProp superclass.
            virtual void ReleaseGraphicsResources(vtkWindow *w);
            virtual int RenderOverlay(vtkViewport *viewport);
            virtual int RenderOpaqueGeometry(vtkViewport *viewport);

          protected:
            vtkZoomSelectionWidgetRepresentation();
            virtual ~vtkZoomSelectionWidgetRepresentation();

            // attributes
            vtkZoomSelectionWidget::WidgetType m_type;
            vtkSmartPointer<vtkPoints>         m_displayPoints;
            vtkSmartPointer<vtkPoints>         m_worldPoints;
            vtkSmartPointer<vtkActor>          m_lineActor;
            Nm                                 m_depth;
            Nm                                 m_slice;

          private:
            // helper methods
            void DisplayPointsToWorldPoints();

            vtkZoomSelectionWidgetRepresentation(const vtkZoomSelectionWidgetRepresentation&);  //Not implemented
            void operator=(const vtkZoomSelectionWidgetRepresentation&);  //Not implemented
        };

      }
    }
  }
}
#endif /* VTKZOOMSELECTIONWIDGETREPRESENTATION_H_ */
