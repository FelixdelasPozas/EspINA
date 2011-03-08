#include "vtkConnectedThresholdImageFilter.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"

//ITK
#include "itkImage.h"
#include "itkVTKImageToImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include <itkLabelMap.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkRegionOfInterestImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkImageFileWriter.h>
#include <algorithm>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#define vtkTemplateMyMacro(call)                                              \
  vtkTemplateMacroCase(VTK_DOUBLE, double, call);                           \
  vtkTemplateMacroCase(VTK_FLOAT, float, call);                             \
  //vtkTemplateMacroCase_ll(VTK_LONG_LONG, long long, call)                   \
  vtkTemplateMacroCase_ll(VTK_UNSIGNED_LONG_LONG, unsigned long long, call) \
  vtkTemplateMacroCase_si64(VTK___INT64, __int64, call)                     \
  vtkTemplateMacroCase_ui64(VTK_UNSIGNED___INT64, unsigned __int64, call)   \
  vtkTemplateMacroCase(VTK_ID_TYPE, vtkIdType, call);                       \
  vtkTemplateMacroCase(VTK_LONG, long, call);                               \
  vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, call);             \
  vtkTemplateMacroCase(VTK_INT, int, call);                                 \
  vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, call);               \
  vtkTemplateMacroCase(VTK_SHORT, short, call);                             \
  vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, call);           \
  vtkTemplateMacroCase(VTK_CHAR, char, call);                               \
  vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, call);                 \
  vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, call)

vtkStandardNewMacro(vtkConnectedThresholdImageFilter);


// The switch statement in Execute will call this method with
// the appropriate input type (IT). Note that this example assumes
// that the output data type is the same as the input data type.
// This is not always the case.
//template <class IT>
void applyFilter(vtkImageData* input,
    vtkImageData* output,
   // IT* inPtr, IT* outPtr,
    /*
    std::vector<IndexType> SeedList,
    InputImagePixelType    Lower,
    InputImagePixelType    Upper,
    OutputImagePixelType   ReplaceValue,
  
    */
    double x, double y, double z,
    double threshold
    
    
		)
{
  std::cout << "REQUEST DATA" << std::endl;
  int dims[3];
  input->GetDimensions(dims);

  //Typedefs
  typedef unsigned short SegPixel;
  typedef unsigned char InputPixelType;
  typedef unsigned char OutputPixelType;
  typedef itk::Image<InputPixelType,3> InputImageType;
  typedef itk::Image<OutputPixelType,3> OutputImageType;
  typedef itk::VTKImageToImageFilter<InputImageType> VTK2ITKFilter;
  
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;
  WriterType::Pointer writer = WriterType::New();

  
  std::cout << "FLAG applyFilter 1" << std::endl;
//   if (input->GetScalarType() != output->GetScalarType())
//   {
//     vtkGenericWarningMacro(<< "Execute: input ScalarType, " << input->GetScalarType()
//     << ", must match out ScalarType " << output->GetScalarType());
//     return;
//   }
  //typename 
  VTK2ITKFilter::Pointer vtk2itk_filter = VTK2ITKFilter::New();
    
 // typename 
    itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType>::Pointer ctif =
    itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType>::New();
 
  //typename 
  itk::ImageToVTKImageFilter<OutputImageType>::Pointer itk2vtk_filter =
    itk::ImageToVTKImageFilter<OutputImageType>::New();
     
  vtk2itk_filter->SetInput(input);
  vtk2itk_filter->Update();
  
  ctif->SetInput( vtk2itk_filter->GetOutput() );

  double color = input->GetScalarComponentAsDouble(x, y, z, 0);
  
  ctif->SetReplaceValue(255); // 1 es ya el valor por defecto. Remplaza los valores de los pixeles que se encuentren entre lower y upper.
  ctif->SetLower(std::max(color - threshold, 0.0));
  ctif->SetUpper(std::min(color + threshold, 255.0));
  
  std::cout << "COLOR: " << color << std::endl << "LOWER: " << (int)ctif->GetLower() << std::endl<< "UPPER: " << (int)ctif->GetUpper() << std::endl;
  
  InputImageType::IndexType seed;
  seed[0] = x;
  seed[1] = y;
  seed[2] = z;
  
  std::cout << "FLAG applyFilter 2" << std::endl;
  std::cout << x << " " << y << " " << z << std::endl;
  std::cout << seed[0] << " " << seed[1] << " " << seed[2] << std::endl;
  
  ctif->AddSeed( seed );
  
  // Convert label image to label map
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<OutputImageType, LabelMapType> Image2LabelFilter;
  
  Image2LabelFilter::Pointer image2label = Image2LabelFilter::New();
  image2label->SetInput(ctif->GetOutput());
  image2label->Update();//TODO: Check if needed
  
  //TODO: Sobra?
  itk::LabelMapToLabelImageFilter<LabelMapType,OutputImageType>::Pointer label2image =
  itk::LabelMapToLabelImageFilter<LabelMapType,OutputImageType>::New();
  label2image->SetInput(image2label->GetOutput());
  label2image->Update();

  
  // Get the roi of the object
  LabelMapType *labelMap = image2label->GetOutput();
  std::cout << "Labels: " << labelMap->GetLabels().at(0) << std::endl;;
  
  LabelObjectType *object = labelMap->GetLabelObject(255);
  unsigned char label = object->GetLabel();
  LabelObjectType::RegionType objectROI = object->GetRegion();
  //OutputImageType::SizeType t;
  //t[0] = t[1] = t[2] = 50;
  //objectROI.SetSize(t);
  
  //itk::RegionOfInterestImageFilter<OutputImageType,OutputImageType>::Pointer roi =
  //itk::RegionOfInterestImageFilter<OutputImageType,OutputImageType>::New();
  itk::ExtractImageFilter<OutputImageType,OutputImageType>::Pointer extract =
  itk::ExtractImageFilter<OutputImageType,OutputImageType>::New();
  
  //InputImageType::RegionType inputRegion = ctif->GetOutput()->GetLargestPossibleRegion();
  
  //InputImageType::SizeType size = inputRegion.GetSize();
  //InputImageType::IndexType start = inputRegion.GetIndex();
  //start[0] = 50;
  
  //InputImageType::RegionType miROI;
  //size[0] = 100;
  //miROI.SetSize(size);
  //miROI.SetIndex(start);
  
  extract->SetInput(ctif->GetOutput());
  extract->SetExtractionRegion(objectROI);
  extract->Update();
  //extract->GetOutput()->Print(std::cout);
  
  writer->SetFileName("Test.mha");
  writer->SetInput(extract->GetOutput());
  writer->Write();
  
  //std:cout << objectROI.GetSize() << std::endl;
  //roi->SetRegionOfInterest(objectROI);
  //roi->SetInput(label2image->GetOutput());
  //roi->Update();
  
  
  itk2vtk_filter->SetInput( extract->GetOutput() );
  itk2vtk_filter->Update();
  std::cout << "FLAG applyFilter 3" << std::endl;
  output->DeepCopy( itk2vtk_filter->GetOutput() );
  
  
}

void vtkConnectedThresholdImageFilter::SimpleExecute(vtkImageData* input,
                                                vtkImageData* output)
{
  std::cerr << "FLAG simpleExecute" << std::endl;
  
  	      applyFilter(input, output, m_seed[0], m_seed[1], m_seed[2], m_threshold);

  /*
  switch(output->GetScalarType())
    {
    // This is simply a #define for a big case list. It handles all
    // data types VTK supports.
     vtkTemplateMyMacro(
		      applyFilter(input, output,
					  static_cast<VTK_TT *>(inPtr), 
					  static_cast<VTK_TT *>(outPtr),
					  m_seed[0], m_seed[1], m_seed[2], m_threshold)
 		      );
    
    default:
      vtkGenericWarningMacro("Execute: Unknown input ScalarType");
      return;
    }
    */
  
}

int vtkConnectedThresholdImageFilter::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  
  int wholeExtent[6];
  double spacing[3];
  
  // Use the same input spacing
  inInfo->Get(vtkDataObject::SPACING(), spacing);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  
  std::cout << "REQUEST EXTENT" << std::endl;
  wholeExtent[0] = 0;
  wholeExtent[1] = 82;
  wholeExtent[2] = 164;
  wholeExtent[3] = 420;
  wholeExtent[4] = 0;
  wholeExtent[5] = 22;
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),wholeExtent,6); 
  
  return 1;
}


void vtkConnectedThresholdImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkSimpleImageToImageFilter::PrintSelf(os, indent);
    os << indent << "Threshold: " << m_threshold << endl;
    os << indent << "Seed: (" << m_seed[0] << ", " << m_seed[1]
      << ", "<< m_seed[2] << ")" << endl;
}




