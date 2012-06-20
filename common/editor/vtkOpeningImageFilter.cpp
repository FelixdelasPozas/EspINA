#include "vtkOpeningImageFilter.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>

//ITK
#include <itkImage.h>
#include <itkVTKImageToImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <algorithm>

#include <iostream>

vtkStandardNewMacro(vtkOpeningImageFilter);

const unsigned int LABEL_VALUE = 255;

vtkOpeningImageFilter::vtkOpeningImageFilter()
: Radius(3)
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

int vtkOpeningImageFilter::RequestData(vtkInformation* request,
				      vtkInformationVector** inputVector,
				      vtkInformationVector* outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkImageData *input  = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  input->Update();

  vtkDebugMacro(<< "Request Data");
/*
  if (false)
  {
    vtkDebugMacro(<< "Request Data: Invalid output");
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
		 0,-1,0,-1,0,-1);
    return 1;
  }*/

  typedef itk::Image<unsigned char, 3> ImageType;

  vtkDebugMacro(<< "Converting from VTK To ITK");
  typedef itk::VTKImageToImageFilter<ImageType> vtk2itkType;
  vtk2itkType::Pointer vtk2itk_filter = vtk2itkType::New();
  //vtk2itk_filter->ReleaseDataFlagOn();
  vtk2itk_filter->SetInput(input);
  vtk2itk_filter->Update();

  vtkDebugMacro(<< "Compute Image Opening");
  typedef itk::BinaryBallStructuringElement<ImageType::PixelType, 3> StructuringElementType;
  typedef itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, StructuringElementType> bmcifType;

  StructuringElementType ball;
  ball.SetRadius(Radius);
  ball.CreateStructuringElement();

  bmcifType::Pointer bmcif = bmcifType::New();
  bmcif->SetInput(vtk2itk_filter->GetOutput());
  bmcif->SetKernel(ball);
  bmcif->SetForegroundValue(LABEL_VALUE);
//   bmcif->ReleaseDataFlagOn();
  bmcif->Update();

  vtkDebugMacro(<< "Converting from ITK to VTK");
  typedef itk::ImageToVTKImageFilter<ImageType> itk2vtkFilterType;
  itk2vtkFilterType::Pointer itk2vtk_filter = itk2vtkFilterType::New();
  itk2vtk_filter->SetInput(bmcif->GetOutput());
//   itk2vtk_filter->ReleaseDataFlagOn();
  itk2vtk_filter->Update();

  output->DeepCopy(itk2vtk_filter->GetOutput());

  vtkDebugMacro(<< "Updating Information");
  output->CopyInformation(itk2vtk_filter->GetOutput());
//   // Without these lines, the output will appear real but will not work as the input to any other filters
  output->SetExtent(itk2vtk_filter->GetOutput()->GetExtent());
  output->SetSpacing(itk2vtk_filter->GetOutput()->GetSpacing());
  //output->SetUpdateExtent(output->GetExtent());//WARNING: TODO: Review this
  return 1;
}