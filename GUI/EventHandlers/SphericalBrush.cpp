/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <Core/Utils/Bounds.h>
#include <GUI/EventHandlers/SphericalBrush.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkSphere.h>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  Brush::StrokePoint SphericalBrush::createStrokePoint(NmVector3 point)
  {
    Bounds brushBounds{ point[0] - m_strokeRadius,
                        point[0] + m_strokeRadius,
                        point[1] - m_strokeRadius,
                        point[1] + m_strokeRadius,
                        point[2] - m_strokeRadius,
                        point[2] + m_strokeRadius};

    double brushCenter[3]{ point[0], point[1], point[2] };
    auto brush = vtkSmartPointer<vtkSphere>::New();
    brush->SetCenter(brushCenter);
    brush->SetRadius(m_strokeRadius);

    return StrokePoint{brush, brushBounds};
  }

  //-----------------------------------------------------------------------------
  void SphericalBrush::configureBrush(RenderView* view)
  {
    auto view2D = view2D_cast(view);

    m_plane        = view2D->plane();
    m_resolution   = view2D->sceneResolution();
    m_strokeRadius = radius() * view2D->scale();
  }

} // namespace ESPINA
