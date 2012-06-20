#ifndef VTK_CLOSING_IMAGE_FILTER_H
#define VTK_CLOSING_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

class vtkClosingImageFilter
: public vtkImageAlgorithm
{
public:
  static vtkClosingImageFilter *New();
  vtkTypeMacro(vtkClosingImageFilter,vtkImageAlgorithm);

  //! Structuring Element Radius
  vtkSetMacro(Radius, int);
  vtkGetMacro(Radius, int);

  void PrintSelf(ostream& os, vtkIndent indent){}

protected:
  vtkClosingImageFilter();
  ~vtkClosingImageFilter(){};

//   virtual int RequestInformation(vtkInformation* request,
// 				 vtkInformationVector** inputVector,
// 				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);

private:
  vtkClosingImageFilter(const vtkClosingImageFilter& );// Not implemented
  void operator=(const vtkClosingImageFilter&);// Not implemented

private:
  //BTX
  int Radius;
  //ETX
};

#endif // VTK_CLOSING_IMAGE_FILTER_H
