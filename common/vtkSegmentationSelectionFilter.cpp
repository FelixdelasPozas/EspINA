/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "vtkSegmentationSelectionFilter.h"

// VTK 
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkImageData.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkSegmentationSelectionFilter);

vtkSegmentationSelectionFilter::vtkSegmentationSelectionFilter()
: PixelValue(0)
{
  bzero(CheckPixel, 3*sizeof(int));
}

int vtkSegmentationSelectionFilter::RequestUpdateExtent(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  
  //Only update requested pixel extent
  int pixelExtent[6] = {CheckPixel[0], CheckPixel[0],
		     CheckPixel[1], CheckPixel[1],
		     CheckPixel[2], CheckPixel[2]};

  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
	      pixelExtent,
	      6);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
	      pixelExtent,
	      6);
    
    return 1;
}


int vtkSegmentationSelectionFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *input  = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  std::cout << "\t\t\tUpdating" << std::endl;
  PixelValue = input->GetScalarComponentAsDouble(CheckPixel[0],CheckPixel[1],CheckPixel[2],0);
  return 1;
}
