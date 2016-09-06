/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

#include "Stencil.h"

#include <Core/Utils/VolumeBounds.h>

#include <vtkImageStencilData.h>
#include <vtkImplicitFunctionToImageStencil.h>
#include <vtkMath.h>
#include <vtkPlane.h>

using namespace ESPINA;
using namespace ESPINA::Filters::Utils;

vtkSmartPointer<vtkImageStencilData> Stencil::fromPlane(vtkSmartPointer<vtkPlane> plane, const VolumeBounds &bounds)
{
  auto origin  = bounds.origin();
  auto spacing = bounds.spacing();

  int extent[6]{ vtkMath::Round((bounds[0] + spacing[0] / 2) / spacing[0]),
                 vtkMath::Round((bounds[1] + spacing[0] / 2) / spacing[0]),
                 vtkMath::Round((bounds[2] + spacing[1] / 2) / spacing[1]),
                 vtkMath::Round((bounds[3] + spacing[1] / 2) / spacing[1]),
                 vtkMath::Round((bounds[4] + spacing[2] / 2) / spacing[2]),
                 vtkMath::Round((bounds[5] + spacing[2] / 2) / spacing[2]) };

  auto plane2stencil = vtkSmartPointer<vtkImplicitFunctionToImageStencil>::New();
  plane2stencil->SetInput(plane);
  plane2stencil->SetOutputOrigin(origin[0], origin[1], origin[2]);
  plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  plane2stencil->SetOutputWholeExtent(extent);
  plane2stencil->Update();

  return plane2stencil->GetOutput();
}
