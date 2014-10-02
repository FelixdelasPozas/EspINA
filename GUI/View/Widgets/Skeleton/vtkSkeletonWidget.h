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

#ifndef ESPINA_VTK_SKELETON_WIDGET_H_
#define ESPINA_VTK_SKELETON_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// VTK
#include <vtkAbstractWidget.h>

using namespace std;

namespace ESPINA
{
  class EspinaGUI_EXPORT vtkSkeletonWidget
  : public vtkAbstractWidget
  {
    public:
      /** \brief Creates a new instance.
       *
       */
      static vtkSkeletonWidget *New();

      vtkTypeMacro(vtkSkeletonWidget,vtkAbstractWidget);

      /** \brief Overrides vtkAbstractWidget::SetEnabled().
       *
       */
      virtual void SetEnabled(int) override;

      /** \brief Overrides vtkAbstractWidget::SetProcessEvents().
       *
       */
      virtual void SetProcessEvents(int) override;

      /** \brief Implements vtkAbstractWidget::CreateDefaultRepresentation().
       *
       */
      void CreateDefaultRepresentation();

      /** \brief Overrides vtkAbstractWidget::PrintSelf().
       *
       */
      virtual void PrintSelf(ostream &os, vtkIndent indent) override;

    protected:
      /** \brief vtkSkeletonWidget class constructor.
       *
       */
      vtkSkeletonWidget();

      /** \brief vtkSkeletonWidget class virtual destructor.
       *
       */
      virtual ~vtkSkeletonWidget();
  };

} // namespace ESPINA

#endif // ESPINA_VTK_SKELETON_WIDGET_H_
