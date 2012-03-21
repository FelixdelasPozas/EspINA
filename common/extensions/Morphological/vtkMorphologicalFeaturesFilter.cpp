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

vtkMorphologicalFeaturesFilter::vtkMorphologicalFeaturesFilter()
: ComputeFeret(false)
, Size(0)
, PhysicalSize(0)
, FeretDiameter(0)
{
  memset(Centroid, 0, 3*sizeof(double));
  memset(Region, 0, 3*sizeof(int));
  memset(BinaryPrincipalMoments, 0, 3*sizeof(double));
  memset(BinaryPrincipalAxes, 0, 9*sizeof(double));
  memset(EquivalentEllipsoidSize, 0, 3*sizeof(double));
}


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
  image2label->SetComputeFeretDiameter(ComputeFeret);
  image2label->Update();//TODO: Check if needed
 
  LabelMapType *labelMap = image2label->GetOutput();
  labelMap->Update();
  assert(labelMap->GetNumberOfLabelObjects() > 0);
  // std::cout << labelMap->GetNumberOfLabelObjects() << std::endl;
  LabelObjectType *object = labelMap->GetNthLabelObject(0);

  Size = object->GetSize();

  PhysicalSize = object->GetPhysicalSize();
  
  Centroid[0] = object->GetCentroid()[0];
  Centroid[1] = object->GetCentroid()[1];
  Centroid[2] = object->GetCentroid()[2];

  Region[0] = object->GetRegion().GetSize()[0];
  Region[1] = object->GetRegion().GetSize()[1];
  Region[2] = object->GetRegion().GetSize()[2];

  BinaryPrincipalMoments[0] = object->GetBinaryPrincipalMoments()[0];
  BinaryPrincipalMoments[1] = object->GetBinaryPrincipalMoments()[1];
  BinaryPrincipalMoments[2] = object->GetBinaryPrincipalMoments()[2];

  BinaryPrincipalAxes[0] = object->GetBinaryPrincipalAxes()[0][0];
  BinaryPrincipalAxes[1] = object->GetBinaryPrincipalAxes()[0][1];
  BinaryPrincipalAxes[2] = object->GetBinaryPrincipalAxes()[0][2];
  BinaryPrincipalAxes[3] = object->GetBinaryPrincipalAxes()[1][0];
  BinaryPrincipalAxes[4] = object->GetBinaryPrincipalAxes()[1][1];
  BinaryPrincipalAxes[5] = object->GetBinaryPrincipalAxes()[1][2];
  BinaryPrincipalAxes[6] = object->GetBinaryPrincipalAxes()[2][0];
  BinaryPrincipalAxes[7] = object->GetBinaryPrincipalAxes()[2][1];
  BinaryPrincipalAxes[8] = object->GetBinaryPrincipalAxes()[2][2];

  FeretDiameter = ComputeFeret? object->GetFeretDiameter() : 0;
  
  EquivalentEllipsoidSize[0] = object->GetEquivalentEllipsoidSize()[0];
  EquivalentEllipsoidSize[1] = object->GetEquivalentEllipsoidSize()[1];
  EquivalentEllipsoidSize[2] = object->GetEquivalentEllipsoidSize()[2];

  return 1;
}
