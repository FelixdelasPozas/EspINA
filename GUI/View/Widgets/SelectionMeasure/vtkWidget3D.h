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

#ifndef ESPINA_GUI_VIEW_WIDGETS_SELECTION_MEASURE_VTK_WIDGET_3D_H_
#define ESPINA_GUI_VIEW_WIDGETS_SELECTION_MEASURE_VTK_WIDGET_3D_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Core/Utils/Bounds.h>

//VTK
#include <vtkAbstractWidget.h>
#include <vtkSmartPointer.h>

class vtkCubeAxesActor2D;

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
          class EspinaGUI_EXPORT vtkWidget3D
          : public vtkAbstractWidget
          {
          public:
            /** \brief VTK-style New() class method.
             *
             */
            static vtkWidget3D *New();

            vtkTypeMacro(vtkWidget3D, vtkAbstractWidget);

            virtual void CreateDefaultRepresentation();

            virtual void SetEnabled(int);

            /** \brief Set measure bounds
             *
             */
            void setBounds(Bounds bounds);

          protected:
            explicit vtkWidget3D();
            ~vtkWidget3D();

          private:
            QByteArray label(const Axis axis, const Bounds &bounds);

          private:
            bool   m_enabled;
            vtkSmartPointer<vtkCubeAxesActor2D> m_actor;
          };
        }
      }
    }
  }
} // namespace ESPINA
#endif // ESPINA_GUI_VIEW_WIDGETS_SELECTION_MEASURE_VTK_WIDGET_3D_H_
