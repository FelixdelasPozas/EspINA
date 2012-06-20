#ifndef VTK_ERODE_IMAGE_FILTER_H
#define VTK_ERODE_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

class vtkErodeImageFilter
: public vtkImageAlgorithm
{
public:
  static vtkErodeImageFilter *New();
  vtkTypeMacro(vtkErodeImageFilter,vtkImageAlgorithm);

  //! Structuring Element Radius
  vtkSetMacro(Radius, int);
  vtkGetMacro(Radius, int);

  void PrintSelf(ostream& os, vtkIndent indent){}

protected:
  vtkErodeImageFilter();
  ~vtkErodeImageFilter(){};

//   virtual int RequestInformation(vtkInformation* request,
// 				 vtkInformationVector** inputVector,
// 				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);

private:
  vtkErodeImageFilter(const vtkErodeImageFilter& );// Not implemented
  void operator=(const vtkErodeImageFilter&);// Not implemented

private:
  //BTX
  int Radius;
  //ETX
};

#endif // VTK_ERODE_IMAGE_FILTER_H
