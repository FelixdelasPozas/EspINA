#include "vtkConnectedThresholdImageFilter.h"

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

//#include "vtkMultiProcessController.h"

#include <iostream>

vtkStandardNewMacro(vtkConnectedThresholdImageFilter);


vtkConnectedThresholdImageFilter::vtkConnectedThresholdImageFilter()
: m_data(NULL)
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}


#define LABEL_VALUE 255

int vtkConnectedThresholdImageFilter::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  //Get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  
//   int updateExtent[6];
//   outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), updateExtent);
// 
//   std::cout << "\tOutput Update Extent: " << updateExtent[0] << " " << updateExtent[1] << " " <<  updateExtent[2] <<  " " <<  updateExtent[3] <<  " " <<  updateExtent[4]<<  " " <<  updateExtent[5] << std::endl;
//   
//   bool noSeed = m_seed[0] < updateExtent[0] || m_seed[0] > updateExtent[1] ||
// 	        m_seed[1] < updateExtent[2] || m_seed[1] > updateExtent[3] ||
// 	        m_seed[2] < updateExtent[4] || m_seed[2] > updateExtent[5];
//     
//   if (noSeed)
//   {
//     vtkDebugMacro(<< "Request Information: No Seed");
//     outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
// 		 0,-1,0,-1,0,-1);
//     return 1;
//   }
  return vtkImageAlgorithm::RequestInformation(request,inputVector,outputVector);
  
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
}



int vtkConnectedThresholdImageFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  vtkImageData *input  = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkDebugMacro(<< "Request Data");

  int wholeExtent[6];
  const vtkTypeInt32* inExtent;
  int outUpdateExtent[6];
  
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),wholeExtent);
  inExtent = input->GetExtent();
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),outUpdateExtent);
  
  //std::cout << "Process "<< vtkMultiProcessController::GetGlobalController()->GetLocalProcessId() << std::endl;
//   std::cout << "\tSeed: " << m_seed[0] << m_seed[1] << m_seed[2] << std::endl;
//   std::cout << "\tInput Extent: " << inExtent[0] << " " << inExtent[1] << " " <<  inExtent[2] <<  " " <<  inExtent[3] <<  " " <<  inExtent[4]<<  " " <<  inExtent[5] << std::endl;
//   std::cout << "\tWhole Extent: " << wholeExtent[0] << " " << wholeExtent[1] << " " <<  wholeExtent[2] <<  " " <<  wholeExtent[3] <<  " " <<  wholeExtent[4]<<  " " <<  wholeExtent[5] << std::endl;
//   std::cout << "\tOutput Update Extent: " << outUpdateExtent[0] << " " << outUpdateExtent[1] << " " <<  outUpdateExtent[2] <<  " " <<  outUpdateExtent[3] <<  " " <<  outUpdateExtent[4]<<  " " <<  outUpdateExtent[5] << std::endl;
  bool noSeed = m_seed[0] < inExtent[0] || m_seed[0] > inExtent[1] ||
	        m_seed[1] < inExtent[2] || m_seed[1] > inExtent[3] ||
	        m_seed[2] < inExtent[4] || m_seed[2] > inExtent[5];
    
  if (noSeed)
  {
//     std::cout << "\tNo Seed" << std::endl;
    
    vtkDebugMacro(<< "Request Data: No Seed");
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
		 0,-1,0,-1,0,-1);
    return 1;
  }
  
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
  
  ctif->Update();
  itk2vtk_filter->SetInput( ctif->GetOutput() );
  itk2vtk_filter->Update();
  
  output->DeepCopy( itk2vtk_filter->GetOutput() );

  vtkDebugMacro(<< "Updating Information");
  output->CopyInformation(itk2vtk_filter->GetOutput());
  
//   // Without these lines, the output will appear real but will not work as the input to any other filters
//   output->SetExtent(itk2vtk_filter->GetOutput()->GetExtent());
//   output->SetSpacing(input->GetSpacing());
// //   output->SetUpdateExtent(output->GetExtent());//WARNING: TODO: Review this
//   output->SetWholeExtent(output->GetExtent());
  
  // Keep a pointer for furhter reference
  m_data = output;
  
  return 1;
}

// int *vtkConnectedThresholdImageFilter::SetCheckPixel()
// { 
//   std::cout << "Using this" << std::endl;
//   //vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning " << #name " pointer " << this->name); 
//   return this->PixelValue; 
// } 

void vtkConnectedThresholdImageFilter::SetCheckPixel(int x, int y, int z, int value)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning " << "PixelValue  = (" << x << "," << y << "," << z << "," << value << ")");
  if ((this->CheckPixel[0] != x)||(this->CheckPixel[1] != y)||(this->CheckPixel[2] != z)||(this->CheckPixel[3] != value)) 
  { 
    this->CheckPixel[0] = x; 
    this->CheckPixel[1] = y;
    this->CheckPixel[2] = z; 
    this->CheckPixel[3] = value; 
//     this->Modified(); 
//     std::cout << "GETTING PIXEL: " << x << " " << y << " " << z << std::endl;
  } 
  if (!m_data)
    return;
  
  PixelValue = m_data->GetScalarComponentAsDouble(x,y,z,0);
}

void vtkConnectedThresholdImageFilter::SetCheckPixel(int arg[4])
{
  this->SetCheckPixel(arg[0], arg[1], arg[2], arg[3]);
}


void vtkConnectedThresholdImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkImageAlgorithm::PrintSelf(os, indent);
    os << indent << "Threshold: " << m_threshold << endl;
    os << indent << "Seed: (" << m_seed[0] << ", " << m_seed[1]
      << ", "<< m_seed[2] << ")" << endl;
}
