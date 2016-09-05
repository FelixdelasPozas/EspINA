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

// ESPINA
#include "Utils.h"

#include <Core/Types.h>

#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageConstantPad.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View;

//-----------------------------------------------------------------------------
void Utils::addPadding(vtkSmartPointer< vtkImageData > image, int normal)
{
  int extent[6];
  image->GetExtent(extent);

  bool needsPadding = false;

  for(auto i: {0,1,2})
  {
    if(i != normal && (extent[2*i] == extent[2*i+1]))
    {
      needsPadding = true;
      ++extent[2*i+1];
    }
  }

  if(needsPadding)
  {
    auto pad = vtkSmartPointer<vtkImageConstantPad>::New();
    pad->SetInputData(image);
    pad->SetOutputWholeExtent(extent);
    pad->SetConstant(SEG_BG_VALUE);
    pad->SetNumberOfThreads(1);
    pad->SetUpdateExtentToWholeExtent();
    pad->ReleaseDataFlagOn();
    pad->UpdateWholeExtent();

    image = pad->GetOutput();
  }
}
