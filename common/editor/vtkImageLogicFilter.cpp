#include "vtkImageLogicFilter.h"

#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
//#include <vtkDataObject.h>
#include <vtkSmartPointer.h>
#include <vtkImageConstantPad.h>
#include <vtkImageLogic.h>

vtkStandardNewMacro(vtkImageLogicFilter)

//----------------------------------------------------------------------------
vtkImageLogicFilter::vtkImageLogicFilter(){
    this->SetNumberOfInputPorts(1);
}

//----------------------------------------------------------------------------
vtkImageLogicFilter::~vtkImageLogicFilter(){
}

//----------------------------------------------------------------------------
int vtkImageLogicFilter::FillInputPortInformation(int port,
						  vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");

  return 1;
}

//----------------------------------------------------------------------------
int vtkImageLogicFilter::RequestInformation(vtkInformation *request,
					    vtkInformationVector **inputVector,
					    vtkInformationVector *outputVector)
{
  // Get input and output pipeline information.
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo1 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo2 = inputVector[0]->GetInformationObject(1);

  // Get the input whole extent.
  int i1_extent[6],i2_extent[6];
  inInfo1->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), i1_extent);
  inInfo2->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), i2_extent);


  output_extent[0] = std::min(i1_extent[0],i2_extent[0]);
  output_extent[1] = std::max(i1_extent[1],i2_extent[1]);
  output_extent[2] = std::min(i1_extent[2],i2_extent[2]);
  output_extent[3] = std::max(i1_extent[3],i2_extent[3]);
  output_extent[4] = std::min(i1_extent[4],i2_extent[4]);
  output_extent[5] = std::max(i1_extent[5],i2_extent[5]);

  // Store the new whole extent for the output.
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), output_extent, 6);
  //outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), output_extent, 6);

  //inInfo1->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), output_extent, 6);
  //inInfo1->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), output_extent, 6);

  //inInfo2->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), output_extent, 6);
  //inInfo2->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), output_extent, 6);

  return 1;
}

//----------------------------------------------------------------------------
int vtkImageLogicFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                             vtkInformationVector **inputVector,
                                             vtkInformationVector *outputVector)
{
   // int i1_extent[6], i2_extent[6];

  // Get the info objects
  vtkInformation *inInfo1 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo2 = inputVector[0]->GetInformationObject(1);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Get the input and ouptut
  vtkImageData *image1 = vtkImageData::SafeDownCast(
      inInfo1->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *image2;
      image2 = vtkImageData::SafeDownCast(
          inInfo2->Get(vtkDataObject::DATA_OBJECT()));

  vtkImageData *output = vtkImageData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkImageData> disconnectedImage1 = vtkSmartPointer<vtkImageData>::New();
  disconnectedImage1->ShallowCopy(image1);

  vtkSmartPointer<vtkImageData> disconnectedImage2 = vtkSmartPointer<vtkImageData>::New();
  disconnectedImage2->ShallowCopy(image2);

  vtkSmartPointer<vtkImageConstantPad> padded1 = vtkSmartPointer<vtkImageConstantPad>::New();
  padded1->SetInput(disconnectedImage1);
  padded1->SetConstant(0.0);
  padded1->SetOutputWholeExtent(output_extent);
  padded1->Update();

  vtkSmartPointer<vtkImageConstantPad> padded2 = vtkSmartPointer<vtkImageConstantPad>::New();
  padded2->SetInput(disconnectedImage2);
  padded2->SetConstant(0.0);
  padded2->SetOutputWholeExtent(output_extent);
  padded2->Update();

  if (Operation == ADDITION){
      vtkSmartPointer<vtkImageLogic> add1 = vtkSmartPointer<vtkImageLogic>::New();
      add1->AddInputConnection(0,padded1->GetOutputPort());
      add1->AddInputConnection(1,padded2->GetOutputPort());
      add1->SetOperation(VTK_OR);
      add1->Update();

      output->ShallowCopy(add1->GetOutput());

  } else if (Operation == SUBSTRACTION){
      vtkSmartPointer<vtkImageLogic> sub1 = vtkSmartPointer<vtkImageLogic>::New();
      sub1->AddInputConnection(0,padded2->GetOutputPort());
      sub1->SetOperation(VTK_NOT);
      sub1->Update();

      vtkSmartPointer<vtkImageLogic> sub2 = vtkSmartPointer<vtkImageLogic>::New();
      sub2->AddInputConnection(0,padded1->GetOutputPort());
      sub2->AddInputConnection(1,sub1->GetOutputPort());
      sub2->SetOperation(VTK_AND);
      sub2->Update();

      output->ShallowCopy(sub2->GetOutput());
  }

  // Without the following lines, the output will appear real but will not work as the input to any other filters

  output->SetExtent(output->GetExtent());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());

  return 1;
}