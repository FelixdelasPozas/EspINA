#include "vtkConnectedThresholdImageFilter.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"

//ITK
#include "itkImage.h"
#include "itkVTKImageToImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include <algorithm>


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
  int dims[3];
  input->GetDimensions(dims);

  typedef unsigned short SegPixel;
  typedef itk::Image<unsigned char,1> InputImageType;
  typedef itk::Image<unsigned char,1> OutputImageType;
    std::cout << "FLAG applyFilter 1" << std::endl;
//   if (input->GetScalarType() != output->GetScalarType())
//   {
//     vtkGenericWarningMacro(<< "Execute: input ScalarType, " << input->GetScalarType()
//     << ", must match out ScalarType " << output->GetScalarType());
//     return;
//   }
  //typename 
  itk::VTKImageToImageFilter<InputImageType>::Pointer vtk2itk_filter =
    itk::VTKImageToImageFilter<InputImageType>::New();
    
 // typename 
    itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType>::Pointer ctif =
    itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType>::New();
 
  //typename 
  itk::ImageToVTKImageFilter<OutputImageType>::Pointer itk2vtk_filter =
    itk::ImageToVTKImageFilter<OutputImageType>::New();
     
  vtk2itk_filter->SetInput(input);
  vtk2itk_filter->Update();
  
  ctif->SetInput( vtk2itk_filter->GetOutput() );
  ctif->GetInput()->Print(std::cout);
  double color = input->GetScalarComponentAsDouble(x, y, z, 0); // No tengo muy claro lo de la componente, aunque está igual que en el de Juan
  
  ctif->SetReplaceValue(255); // 1 es ya el valor por defecto. Remplaza los valores de los pixeles que se encuentren entre lower y upper.
  ctif->SetLower(0);//std::max(color - threshold, 0.0));
  ctif->SetUpper(255); //std::min(color + threshold, 255.0));
  
  std::cout << "COLOR: " << color << std::endl << "LOWER: " << ctif->GetLower() << std::endl<< "UPPER: " << ctif->GetUpper() << std::endl;
  
  InputImageType::IndexType seed;
  seed[0] = x;
  seed[1] = y;
  seed[2] = z;

   seed2[] = {10, 20, 30};
  
  std::cout << "FLAG applyFilter 2" << std::endl;
  std::cout << x << " " << y << " " << z << std::endl;
  std::cout << seed[0] << " " << seed[1] << " " << seed[2] << std::endl;
  
  ctif->AddSeed( seed2 );
  std::cout << "FLAG applyFilter 3" << std::endl;
  itk2vtk_filter->SetInput( vtk2itk_filter->GetOutput() );//ctif->GetOutput() );
  itk2vtk_filter->Update();
  
  output->ShallowCopy( itk2vtk_filter->GetOutput() );
  
}

void vtkConnectedThresholdImageFilter::SimpleExecute(vtkImageData* input,
                                                vtkImageData* output)
{
  void* inPtr = input->GetScalarPointer();
  void* outPtr = output->GetScalarPointer();

  std::cerr << "FLAG simpleExecute" << std::endl;
  
  	      applyFilter(input, output,
					  //static_cast<unsigned char *>(inPtr), 
					  //static_cast<unsigned char *>(outPtr),
					  m_seed[0], m_seed[1], m_seed[2], m_threshold);

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

void vtkConnectedThresholdImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkSimpleImageToImageFilter::PrintSelf(os, indent);
    os << indent << "Threshold: " << m_threshold << endl;
    os << indent << "Seed: (" << m_seed[0] << ", " << m_seed[1]
      << ", "<< m_seed[2] << ")" << endl;
}




