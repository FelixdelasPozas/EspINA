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
#include "GUI/View/Widgets/OrthogonalRegion/OrthogonalWidget2D.h"

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
          /** \class vtkOrthogonalWidget2D
           * \brief Implements a vtk widget to represent and manage an orthogonal region.
           *
           */
          class EspinaGUI_EXPORT vtkOrthogonalWidget2D
          : public vtkAbstractWidget
          {
          public:
            static vtkOrthogonalWidget2D *New();

            vtkTypeMacro(vtkOrthogonalWidget2D, vtkAbstractWidget);
            void PrintSelf(ostream& os, vtkIndent indent);

            /** \brief Sets the plane of the view the representation will be shown.
             * \param[in] plane Plane enum value.
             *
             */
            void SetPlane(const Plane plane);

            /** \brief Sets the slice of the representation.
             * \param[in] pos Slice in Nm.
             *
             */
            void SetSlice(const Nm pos);

            /** \brief Sets the depth of the representation in the planar view.
             * \param[in] depth Depth in Nm.
             *
             */
            void SetDepth(const Nm depth);

            /** \brief Sets the representation bounds.
             * \param[in] bounds Representation bounds in 3D.
             *
             */
            void SetBounds(const Bounds bounds);

            /** \brief Returns the representation bounds.
             *
             */
            const Bounds GetBounds();

            void CreateDefaultRepresentation();

            /** \brief Sets the color of the representation.
             * \param[in] color Color rgb values.
             *
             */
            void setRepresentationColor(const double *color);

            /** \brief Sets the representation pattern.
             * \param[in] pattern Pattern value.
             */
            void setRepresentationPattern(const int pattern);

          protected:
            /** \brief vtkOrthogonalWidget2D class constructor.
             *
             */
            vtkOrthogonalWidget2D();

            /** \brief vtkOrthogonalWidget2D class virtual destructor.
             *
             */
            virtual ~vtkOrthogonalWidget2D()
            {};

          private:
            /** \brief Creates the default representation if none.
             *
             */
            void ensureRepresentationIsAvailable();

            /** \brief Updates the visual representation.
             *
             */
            void updateRepresentation();

          private:
            //BTX - manage the state of the widget
            int WidgetState;
            enum _WidgetState {Start=0,Active};
            //ETX

            /** \brief Widget callback method to manage selection actions.
             * \param[in] widget Widget raw pointer.
             *
             */
            static void SelectAction(vtkAbstractWidget *widget);

            /** \brief Widget callback method to manage ending a selection action.
             * \param[in] widget Widget raw pointer.
             *
             */
            static void EndSelectAction(vtkAbstractWidget *widget);

            /** \brief Widget callback method to manage a translate action.
             * \param[in] widget Widget raw pointer.
             *
             */
            static void TranslateAction(vtkAbstractWidget *widget);

            /** \brief Widget callback method to manage a movement action.
             * \param[in] widget Widget raw pointer.
             *
             */
            static void MoveAction(vtkAbstractWidget *widget);

            virtual void SetCursor(int state);

            Plane  m_plane;    /** plane of the representation.         */
            Nm     m_slice;    /** current slice of the representation. */
            Bounds m_bounds;   /** representation bounds.               */
            double m_color[3]; /** representation color rgb values.     */
            int    m_pattern;  /** representation pattern.              */

          private:
            vtkOrthogonalWidget2D(const vtkOrthogonalWidget2D&);  // copy constructor not implemented
            void operator=(const vtkOrthogonalWidget2D&);  // assign operator not implemented
          };
        }
      }
    }
  }
} // namespace ESPINA

#endif // ESPINA_GUI_VIEW_WIDGETS_ORTHOGONAL_REGION_VTK_WIDGET_2D_H
