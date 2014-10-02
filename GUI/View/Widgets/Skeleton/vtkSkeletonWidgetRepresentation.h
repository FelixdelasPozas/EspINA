/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_
#define ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_

// VTK
#include <vtkWidgetRepresentation.h>

namespace ESPINA
{
  /** \class vtkSkeletonWidgetRepresentation.
   * Representation for the SkeletonWidget class.
   */
  class vtkSkeletonWidgetRepresentation
  : public vtkWidgetRepresentation
  {
    public:
      /** \brief Implements vtkObject::New() for this class.
       *
       */
      static vtkWidgetRepresentation* New()
      { return new vtkSkeletonWidgetRepresentation(); }

      /** \brief Implements vtkAbstractWidgetRepresentation::BuildRepresentation().
       *
       */
      void BuildRepresentation();

    private:
      /** \brief vtkSkeletonWidgetRepresentation class private constructor.
       *
       */
      vtkSkeletonWidgetRepresentation();

      /** \brief vtkSkeletonWidgetRepresentation class virtual destructor.
       *
       */
      virtual ~vtkSkeletonWidgetRepresentation();
  };

} // namespace EspINA

#endif // ESPINA_VTK_SKELETON_WIDGET_REPRESENTATION_H_
