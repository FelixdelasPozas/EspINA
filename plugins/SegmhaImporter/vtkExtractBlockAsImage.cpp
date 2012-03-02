#include "vtkExtractBlockAsImage.h"

#include <QDebug>

// #include "vtkFileContent.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkVertexGlyphFilter.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkExtractImageFilter.h>
#include <vtkMultiBlockDataSet.h>


#include <QFile>
#include <QString>
#include <QStringList>
#include <vtkPointData.h>


vtkStandardNewMacro(vtkExtractBlockAsImage);


//---------------------------------------------------------------------------
vtkExtractBlockAsImage::vtkExtractBlockAsImage()
: ExtractBlock(-1)
, Label(-1)
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkExtractBlockAsImage::~vtkExtractBlockAsImage()
{
}



//---------------------------------------------------------------------------
int vtkExtractBlockAsImage::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),
      "vtkMultiBlockDataSet"
    );
    return 1;
  }
  return 0;
}


//---------------------------------------------------------------------------
int vtkExtractBlockAsImage::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  int res = vtkImageAlgorithm::RequestInformation(request, inputVector, outputVector);

  // Get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()) );

  // Use the same input spacing
  vtkImageData *inputImage = vtkImageData::SafeDownCast(input->GetBlock(ExtractBlock));
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inputImage->GetExtent(),6);
  // Use the same input spacing
  outInfo->Set(vtkDataObject::SPACING(), inputImage->GetSpacing(), 3);

  return res;
}

//---------------------------------------------------------------------------
int vtkExtractBlockAsImage::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *inputInfo = inputVector[0]->GetInformationObject(0);
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
    inputInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  vtkInformation *outputInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outputInfo->Get(vtkDataObject::DATA_OBJECT())
  );

  vtkImageData *inputImage = vtkImageData::SafeDownCast(input->GetBlock(ExtractBlock));

  output->ShallowCopy(inputImage);
  output->CopyInformation(inputImage);
  outputInfo->Set(vtkDataObject::SPACING(), inputImage->GetSpacing(), 3);
  output->SetWholeExtent(inputImage->GetExtent());

  Label = input->GetFieldData()->GetArray("Label")->GetTuple1(ExtractBlock);

  return 1;
}



//---------------------------------------------------------------------------
void vtkExtractBlockAsImage::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Extract Block: "
      << ExtractBlock << "\n";
//   os << this->Trace << this->Taxonomy;
}
