/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#include "vtkEspinaSliceRepresentation.h"

#include "vtkPVEspinaView.h"
#include <vtkObjectFactory.h>
#include "vtkEspinaView.h"
#include <vtkRenderer.h>
#include <vtkPVLODActor.h>
#include <vtkProperty.h>

#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkImageResliceToColors.h>

vtkStandardNewMacro(vtkEspinaSliceRepresentation);

//----------------------------------------------------------------------------
vtkEspinaSliceRepresentation::vtkEspinaSliceRepresentation()
: vtkImageSliceRepresentation()
{
}

//----------------------------------------------------------------------------
bool vtkEspinaSliceRepresentation::AddToView(vtkView* view)
{
  vtkPVEspinaView* rview = vtkPVEspinaView::SafeDownCast(view);
  if (rview)
    {
      vtkSmartPointer<vtkImageData> cube = vtkSmartPointer<vtkImageData>::New();
      cube->SetExtent(0,400,0,400,0,20);
      cube->SetSpacing(1,1,2);
      //   vtkSmartPointer<vtkImageResliceToColors> planes = vtkSmartPointer<vtkImageResliceToColors>::New();
      //   planes->SetInput(cube);
      vtkSmartPointer<vtkImageActor> cubeActor = vtkSmartPointer<vtkImageActor>::New();
      cubeActor->SetInput(cube);
      cubeActor->SetOpacity(0.8);

      rview->AddActor(this->Actor);
//       rview->AddActor(cubeActor);
      return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkEspinaSliceRepresentation::SetType(int value)
{
  Type = value;
  if (Type == 1)
  {
    this->Actor->GetProperty()->SetOpacity(0.8);;
  }
}

