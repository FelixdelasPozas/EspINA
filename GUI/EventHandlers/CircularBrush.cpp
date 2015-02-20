/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CircularBrush.h"
#include <GUI/View/View2D.h>
#include <Filters/Utils/vtkTube.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
Brush::StrokePoint CircularBrush::createStrokePoint(NmVector3 point)
{

  double sRadius = (m_plane == Plane::YZ) ? 0 : m_strokeRadius;
  double cRadius = (m_plane == Plane::XZ) ? 0 : m_strokeRadius;
  double aRadius = (m_plane == Plane::XY) ? 0 : m_strokeRadius;

  Bounds brushBounds {point[0] - sRadius,
                      point[0] + sRadius,
                      point[1] - cRadius,
                      point[1] + cRadius,
                      point[2] - aRadius,
                      point[2] + aRadius };

  double baseCenter[3], topCenter[3];

  for (int i=0; i<3; i++)
  {
    baseCenter[i] = topCenter[i] = point[i];
  }

  auto index = normalCoordinateIndex(m_plane);

  topCenter [index] += 0.5 * m_resolution[index];
  baseCenter[index] -= 0.5 * m_resolution[index];

  auto brush = vtkSmartPointer<vtkTube>::New();
  brush->SetBaseCenter(baseCenter);
  brush->SetBaseRadius(m_strokeRadius);
  brush->SetTopCenter(topCenter);
  brush->SetTopRadius(m_strokeRadius);

  return StrokePoint{brush, brushBounds};
}

//-----------------------------------------------------------------------------
void CircularBrush::configureBrush(RenderView *view)
{
  auto view2D = view2D_cast(view);

  m_plane        = view2D->plane();
  m_resolution   = view2D->sceneResolution();
  m_strokeRadius = radius() * view2D->scale();
}
