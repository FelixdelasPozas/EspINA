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


#include "vtkMorphologicalFeaturesFilter.h"

// VTK 
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkImageData.h>
#include <vtkInformationVector.h>

#define LABEL_VALUE 255

vtkStandardNewMacro(vtkMorphologicalFeaturesFilter);

int vtkMorphologicalFeaturesFilter::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  return vtkImageAlgorithm::RequestInformation(request, inputVector, outputVector);
}

int vtkMorphologicalFeaturesFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  
  vtkImageData *input  = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkDebugMacro(<< "Converting from VTK To ITK");
  
  typedef itk::VTKImageToImageFilter<InputImageType> vtk2itkType;
  vtk2itkType::Pointer vtk2itk_filter = vtk2itkType::New();

  vtk2itk_filter->SetInput(input);
  vtk2itk_filter->Update();
    
  vtkDebugMacro(<< "Converting from ITK to LabelMap");
  
  image2label = Image2LabelFilterType::New();
  image2label->SetInput(vtk2itk_filter->GetOutput());
  image2label->Update();//TODO: Check if needed
 
  return 1;
}

void vtkMorphologicalFeaturesFilter::GetSize(int* size)
{
  Update();
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  *size = object->GetSize();
}

void vtkMorphologicalFeaturesFilter::GetPhysicalSize(double* phySize)
{
  Update();
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  *phySize = object->GetPhysicalSize();
}


void vtkMorphologicalFeaturesFilter::GetCentroid(int* centroid)
{
  Update();
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  centroid[0] = object->GetCentroid()[0];
  centroid[1] = object->GetCentroid()[1];
  centroid[2] = object->GetCentroid()[2];
}
