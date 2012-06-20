#ifndef VTK_DILATE_IMAGE_FILTER_H
#define VTK_DILATE_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

class vtkDilateImageFilter
: public vtkImageAlgorithm
{
public:
  static vtkDilateImageFilter *New();
  vtkTypeMacro(vtkDilateImageFilter,vtkImageAlgorithm);

  //! Structuring Element Radius
  vtkSetMacro(Radius, int);
  vtkGetMacro(Radius, int);

  void PrintSelf(ostream& os, vtkIndent indent){}

protected:
  vtkDilateImageFilter();
  ~vtkDilateImageFilter(){};

//   virtual int RequestInformation(vtkInformation* request,
// 				 vtkInformationVector** inputVector,
// 				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);

private:
  vtkDilateImageFilter(const vtkDilateImageFilter& );// Not implemented
  void operator=(const vtkDilateImageFilter&);// Not implemented

private:
  //BTX
  int Radius;
  //ETX
};

#endif // VTK_DILATE_IMAGE_FILTER_H
