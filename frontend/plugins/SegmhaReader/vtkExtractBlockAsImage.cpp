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


vtkStandardNewMacro(vtkExtractBlockAsImage);


//---------------------------------------------------------------------------
vtkExtractBlockAsImage::vtkExtractBlockAsImage()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkExtractBlockAsImage::~vtkExtractBlockAsImage()
{
  //delete [] Taxonomy;
  //delete [] Trace;
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
int vtkExtractBlockAsImage::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *inputInfo = inputVector[0]->GetInformationObject(0);
  inputInfo->PrintSelf(std::cout,vtkIndent(0));
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
    inputInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  vtkInformation *outputInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outputInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  
  output->ShallowCopy(input->GetBlock(ExtractBlock));
    
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