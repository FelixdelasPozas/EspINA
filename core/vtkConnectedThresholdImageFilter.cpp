#include "vtkConnectedThresholdImageFilter.h"

// VTK
#include "vtkImageData.h"
#include "vtkObjectFactory.h"

//ITK
#include "itkImage.h"
#include "itkConnectedThresholdImageFilter.h"

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
  typedef itk::Image<SegPixel,1> OutputImageType;
  
//   if (input->GetScalarType() != output->GetScalarType())
//   {
//     vtkGenericWarningMacro(<< "Execute: input ScalarType, " << input->GetScalarType()
//     << ", must match out ScalarType " << output->GetScalarType());
//     return;
//   }
  
  //itkConnectedThresholdImageFilter<InputImageType, OutputImageType> *ctif =
    //new itkConnectedThresholdImageFilter<InputImageType, OutputImageType>();
  
  
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
    vtkTemplateMacro(
      applyFilter(input, output,
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




