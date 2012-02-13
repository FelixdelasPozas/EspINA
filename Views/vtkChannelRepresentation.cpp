/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "vtkChannelRepresentation.h"

#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkChannelRepresentation);

vtkChannelRepresentation::vtkChannelRepresentation()
{
}

vtkChannelRepresentation::~vtkChannelRepresentation()
{
}


vtkSmartPointer<vtkLookupTable> vtkChannelRepresentation::lut()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetRange (0, 255);
  double saturation = Color>=0?1.0:0;
  lut->SetSaturationRange(0.0, saturation);
  lut->SetValueRange(0.0, 1.0);
  lut->SetHueRange(Color, Color);
  lut->SetRampToLinear();
  lut->Build();

  return lut;
}

void vtkChannelRepresentation::AddToView(vtkPVSliceView* view)
{
  view->AddChannel(&SliceActor);
}
