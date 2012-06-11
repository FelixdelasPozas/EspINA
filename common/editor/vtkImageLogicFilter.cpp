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


  int inExtent[6];
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), OutExtent);
  for(int i=1; i < inputVector[0]->GetNumberOfInformationObjects(); i++)
  {
    inInfo = inputVector[0]->GetInformationObject(i);
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExtent);

    for(int j=0; j<=4; j+=2)
    {
      OutExtent[j] = std::min(inExtent[j], OutExtent[j]);
      OutExtent[j+1] = std::max(inExtent[j+1], OutExtent[j+1]);
    }
//     inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), inExtent, 6);
  }


  // Store the new whole extent for the output.
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), OutExtent, 6);
//   outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), OutExtent, 6);

  return 1;
}

//----------------------------------------------------------------------------
int vtkImageLogicFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                             vtkInformationVector **inputVector,
                                             vtkInformationVector *outputVector)
{
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
  padded1->SetOutputWholeExtent(OutExtent);
  padded1->Update();

  vtkSmartPointer<vtkImageConstantPad> padded2 = vtkSmartPointer<vtkImageConstantPad>::New();
  padded2->SetInput(disconnectedImage2);
  padded2->SetConstant(0.0);
  padded2->SetOutputWholeExtent(OutExtent);
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

  return 1;
}