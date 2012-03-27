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


#include "vtkSegmentationRepresentation.h"

#include <vtkImageActor.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSegmentationRepresentation);

vtkSegmentationRepresentation::vtkSegmentationRepresentation()
{
}

vtkSegmentationRepresentation::~vtkSegmentationRepresentation()
{

}

vtkSmartPointer< vtkLookupTable > vtkSegmentationRepresentation::lut()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues ( 2 );
  double bg[4] = { 0.0, 0.0, 0.0, 0.0 };
  double fg[4] = {RGBColor[0], RGBColor[1], RGBColor[2], 1.0 };
  lut->SetTableValue ( 0,bg );
  lut->SetTableValue ( 1,fg );
  lut->Build();
  return lut;
}

void vtkSegmentationRepresentation::AddToView(vtkPVSliceView* view)
{
  view->AddSegmentation(&SliceActor);
}

void vtkSegmentationRepresentation::SetRGBColor(double r, double g, double b)
{
  RGBColor[0] = r;
  RGBColor[1] = g;
  RGBColor[2] = b;

  if (SliceActor.lut == NULL)
    return;

  SliceActor.lut->SetNumberOfTableValues ( 2 );
  double bg[4] = { 0.0, 0.0, 0.0, 0.0 };
  double fg[4] = {r, g, b, 1.0 };
  SliceActor.lut->SetTableValue(0, bg);
  SliceActor.lut->SetTableValue(1, fg);
  SliceActor.lut->Build();
}

void vtkSegmentationRepresentation::SetRGBColor(double rgb[3])
{
  SetRGBColor(rgb[0], rgb[1], rgb[2]);
}