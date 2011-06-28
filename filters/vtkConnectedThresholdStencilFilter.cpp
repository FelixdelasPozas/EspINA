#include "vtkConnectedThresholdStencilFilter.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>

//ITK
#include "itkImage.h"
#include "itkVTKImageToImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include <itkLabelMap.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkExtractImageFilter.h>
#include <algorithm>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkConnectedThresholdStencilFilter);


vtkConnectedThresholdStencilFilter::vtkConnectedThresholdStencilFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}


#define LABEL_VALUE 255

int vtkConnectedThresholdStencilFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  vtkImageData *input  = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkDebugMacro(<< "Request Data");
  
  typedef unsigned short SegPixel;
  typedef unsigned char InputPixelType;
  typedef unsigned char OutputPixelType;
  typedef itk::Image<InputPixelType,3> InputImageType;
  typedef itk::Image<OutputPixelType,3> OutputImageType;
  
  vtkDebugMacro(<< "Converting from VTK To ITK");
  
  typedef itk::VTKImageToImageFilter<InputImageType> vtk2itkType;
  vtk2itkType::Pointer vtk2itk_filter = vtk2itkType::New();

  vtk2itk_filter->SetInput(input);
  vtk2itk_filter->Update();
    
  vtkDebugMacro(<< "Computing Original Size Connected Threshold");
  
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> ConnectedThresholdFilterType;
  ConnectedThresholdFilterType::Pointer ctif = ConnectedThresholdFilterType::New();

  double color = input->GetScalarComponentAsDouble(m_seed[0], m_seed[1], m_seed[2], 0);
  ctif->SetInput( vtk2itk_filter->GetOutput() );
  ctif->SetReplaceValue(LABEL_VALUE); // 1 es ya el valor por defecto. Remplaza los valores de los pixeles que se encuentren entre lower y upper.
  ctif->SetLower(std::max(color - m_threshold, 0.0));
  ctif->SetUpper(std::min(color + m_threshold, 255.0));
  InputImageType::IndexType seed; //TODO: Use class seed
  seed[0] = m_seed[0]; seed[1] = m_seed[1]; seed[2] = m_seed[2];
  ctif->AddSeed(seed);

  vtkDebugMacro(<< "COLOR: " << color << "THRESHOLD: " << m_threshold << " => LOWER: " << (int)ctif->GetLower() <<  " UPPER: " << (int)ctif->GetUpper());
  vtkDebugMacro(<< "SEED: " << seed[0] << " " << seed[1] << " " << seed[2]);

  vtkDebugMacro(<< "Converting from ITK to LabelMap");
  
  // Convert label image to label map
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<InputImageType, LabelMapType> Image2LabelFilterType;
  
  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
  image2label->SetInput(ctif->GetOutput());
  image2label->Update();//TODO: Check if needed
  
  vtkDebugMacro(<< "Getting Segmentation Region");
  
  // Get the roi of the object
  LabelMapType *    labelMap = image2label->GetOutput();
  LabelObjectType * object   = labelMap->GetLabelObject(LABEL_VALUE);
  LabelObjectType::RegionType objectROI = object->GetRegion();
  
  vtkDebugMacro(<< "Extracting Segmentation Region");
  
  typedef itk::ExtractImageFilter<InputImageType,InputImageType> ExtractFilterType;
  ExtractFilterType::Pointer extract = ExtractFilterType::New();
  
  extract->SetInput(ctif->GetOutput());
  extract->SetExtractionRegion(objectROI);
  extract->Update();
  
  //typedef itk::ImageFileWriter< OutputImageType >  WriterType;
  //WriterType::Pointer writer = WriterType::New();
  //writer->SetFileName("Test.mha");
  //writer->SetInput(extract->GetOutput());
  //writer->Write();
  
  vtkDebugMacro(<< "Converting from ITK to VTK");
  
  typedef itk::ImageToVTKImageFilter<OutputImageType> itk2vtkFilterType;
  itk2vtkFilterType::Pointer itk2vtk_filter = itk2vtkFilterType::New();
  
  itk2vtk_filter->SetInput( extract->GetOutput() );
  itk2vtk_filter->Update();
  
  output->DeepCopy( itk2vtk_filter->GetOutput() );

  vtkDebugMacro(<< "Updating Information");
  
  // Without these lines, the output will appear real but will not work as the input to any other filters
  output->SetExtent(itk2vtk_filter->GetOutput()->GetExtent());
  output->SetSpacing(input->GetSpacing());
  output->SetUpdateExtent(output->GetExtent());
  output->SetWholeExtent(output->GetExtent());
  
  return 1;
}

// int vtkConnectedThresholdStencilFilter::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
// {
//   vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
//   vtkInformation* outInfo = outputVector->GetInformationObject(0);
//   
//   int wholeExtent[6];
//   double spacing[3];
//   
//   // Use the same input spacing
//   inInfo->Get(vtkDataObject::SPACING(), spacing);
//   outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
//   
//   std::cout << "REQUEST EXTENT" << std::endl;
//   wholeExtent[0] = 0;
//   wholeExtent[1] = 82;
//   wholeExtent[2] = 164;
//   wholeExtent[3] = 420;
//   wholeExtent[4] = 0;
//   wholeExtent[5] = 22;
//   
//   outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),wholeExtent,6); 
//   
//   return 1;
// }


void vtkConnectedThresholdStencilFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkImageAlgorithm::PrintSelf(os, indent);
    os << indent << "Threshold: " << m_threshold << endl;
    os << indent << "Seed: (" << m_seed[0] << ", " << m_seed[1]
      << ", "<< m_seed[2] << ")" << endl;
}
