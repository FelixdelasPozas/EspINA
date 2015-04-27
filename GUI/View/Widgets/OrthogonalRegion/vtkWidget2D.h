/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_GUI_VIEW_WIDGETS_ORTHOGONAL_REGION_VTK_WIDGET_2D_H
#define ESPINA_GUI_VIEW_WIDGETS_ORTHOGONAL_REGION_VTK_WIDGET_2D_H

#include "EspinaGUI_Export.h"

// ESPINA
#include <Core/Utils/Bounds.h>
#include "GUI/View/Widgets/OrthogonalRegion/Widget2D.h"

// VTK
#include "vtkAbstractWidget.h"

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          class EspinaGUI_EXPORT vtkWidget2D
          : public vtkAbstractWidget
          {
          public:
            // Description:
            // Instantiate the object.
            static vtkWidget2D *New();

            // Description:
            // Standard class methods for type information and printing.
            vtkTypeMacro(vtkWidget2D, vtkAbstractWidget);
            void PrintSelf(ostream& os, vtkIndent indent);

            void SetPlane(Plane plane);

            void SetSlice(Nm pos);

            void SetDepth(double depth);

            void SetBounds(Bounds bounds);

            Bounds GetBounds();

            // Description:
            // Create the default widget representation if one is not set. By default,
            // this is an instance of the vtkRectangularRectangularRepresentation class.
            void CreateDefaultRepresentation();

            // modify representation methods
            void setRepresentationColor(double *);

            void setRepresentationPattern(int);

          protected:
            vtkWidget2D();

            virtual ~vtkWidget2D();

          private:
            void ensureRepresentationIsAvailable();

            void updateRepresentation();

          private:
            //BTX - manage the state of the widget
            int WidgetState;
            enum _WidgetState {Start=0,Active};
            //ETX

            // These methods handle events
            static void SelectAction(vtkAbstractWidget*);
            static void EndSelectAction(vtkAbstractWidget*);
            static void TranslateAction(vtkAbstractWidget*);
            static void MoveAction(vtkAbstractWidget*);

            // helper methods for cursoe management
            virtual void SetCursor(int state);

            Plane  m_plane;
            Nm     m_slice;
            Bounds m_bounds;

            double m_color[3];
            int    m_pattern;

          private:
            vtkWidget2D(const vtkWidget2D&);  //Not implemented
            void operator=(const vtkWidget2D&);  //Not implemented
          };
        }
      }
    }
  }
} // namespace ESPINA

#endif // ESPINA_GUI_VIEW_WIDGETS_ORTHOGONAL_REGION_VTK_WIDGET_2D_H
