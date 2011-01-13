#include "vtkConnectedThresholdImageFilter.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"

//ITK
#include "itkImage.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkImageToVTKImageFilter.h"

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
template <class IT>
void applyFilter(vtkImageData* input,
    vtkImageData* output,
    IT* inPtr, IT* outPtr)
{
  int dims[3];
  input->GetDimensions(dims);

  typedef unsigned short SegPixel;
  typedef itk::Image<IT,3> InputImageType;
  typedef itk::Image<SegPixel,3> OutputImageType;
  
//   if (input->GetScalarType() != output->GetScalarType())
//   {
//     vtkGenericWarningMacro(<< "Execute: input ScalarType, " << input->GetScalarType()
//     << ", must match out ScalarType " << output->GetScalarType());
//     return;
//   }
  
  typename itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType>::Pointer ctif =
    itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType>::New();
    
  
    
    
  
  
}

void vtkConnectedThresholdImageFilter::SimpleExecute(vtkImageData* input,
                                                vtkImageData* output)
{
  void* inPtr = input->GetScalarPointer();
  void* outPtr = output->GetScalarPointer();

  switch(output->GetScalarType())
    {
    // This is simply a #define for a big case list. It handles all
    // data types VTK supports.
     vtkTemplateMyMacro(
		      applyFilter<VTK_TT>(input, output,
					  static_cast<VTK_TT *>(inPtr), 
					  static_cast<VTK_TT *>(outPtr)));
    
    default:
      vtkGenericWarningMacro("Execute: Unknown input ScalarType");
      return;
    }
}

void vtkConnectedThresholdImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkSimpleImageToImageFilter::PrintSelf(os, indent);
    os << indent << "Threshold: " << m_threshold << endl;
    os << indent << "Seed: (" << m_seed[0] << ", " << m_seed[1]
      << ", "<< m_seed[2] << ")" << endl;
}




