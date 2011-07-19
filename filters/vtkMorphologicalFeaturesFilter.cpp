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
  image2label->SetComputeFeretDiameter(true);
  image2label->Update();//TODO: Check if needed
 
  return 1;
}

void vtkMorphologicalFeaturesFilter::GetSize(long unsigned int* size)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  *size = object->GetSize();
}

void vtkMorphologicalFeaturesFilter::GetPhysicalSize(double* phySize)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  *phySize = object->GetPhysicalSize();
}


void vtkMorphologicalFeaturesFilter::GetCentroid(double* centroid)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  centroid[0] = object->GetCentroid()[0];
  centroid[1] = object->GetCentroid()[1];
  centroid[2] = object->GetCentroid()[2];
}

void vtkMorphologicalFeaturesFilter::GetRegion(int* region)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  region[0] = object->GetRegion().GetSize()[0];
  region[1] = object->GetRegion().GetSize()[1];
  region[2] = object->GetRegion().GetSize()[2];
}

void vtkMorphologicalFeaturesFilter::GetBinaryPrincipalMoments(double* bpm)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  bpm[0] = object->GetBinaryPrincipalMoments()[0];
  bpm[1] = object->GetBinaryPrincipalMoments()[1];
  bpm[2] = object->GetBinaryPrincipalMoments()[2];
}

void vtkMorphologicalFeaturesFilter::GetBinaryPrincipalAxes(double* bpa)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  bpa[0] = object->GetBinaryPrincipalAxes()[0][0];
  bpa[1] = object->GetBinaryPrincipalAxes()[0][1];
  bpa[2] = object->GetBinaryPrincipalAxes()[0][2];
  bpa[3] = object->GetBinaryPrincipalAxes()[1][0];
  bpa[4] = object->GetBinaryPrincipalAxes()[1][1];
  bpa[5] = object->GetBinaryPrincipalAxes()[1][2];
  bpa[6] = object->GetBinaryPrincipalAxes()[2][0];
  bpa[7] = object->GetBinaryPrincipalAxes()[2][1];
  bpa[8] = object->GetBinaryPrincipalAxes()[2][2];
}

void vtkMorphologicalFeaturesFilter::GetFeretDiameter(double* feret)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE);
  *feret = object->GetFeretDiameter();
}

void vtkMorphologicalFeaturesFilter::GetEquivalentEllipsoidSize(double* ees)
{
  LabelMapType *labelMap = image2label->GetOutput();
  LabelObjectType *object = labelMap->GetLabelObject(LABEL_VALUE); 
  ees[0] = object->GetEquivalentEllipsoidSize()[0];
  ees[1] = object->GetEquivalentEllipsoidSize()[1];
  ees[2] = object->GetEquivalentEllipsoidSize()[2];
}
