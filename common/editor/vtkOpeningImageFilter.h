#ifndef VTK_OPENING_IMAGE_FILTER_H
#define VTK_OPENING_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

class vtkOpeningImageFilter
: public vtkImageAlgorithm
{
public:
  static vtkOpeningImageFilter *New();
  vtkTypeMacro(vtkOpeningImageFilter,vtkImageAlgorithm);

  //! Structuring Element Radius
  vtkSetMacro(Radius, int);
  vtkGetMacro(Radius, int);

  void PrintSelf(ostream& os, vtkIndent indent){}

protected:
  vtkOpeningImageFilter();
  ~vtkOpeningImageFilter(){};

//   virtual int RequestInformation(vtkInformation* request,
// 				 vtkInformationVector** inputVector,
// 				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);

private:
  vtkOpeningImageFilter(const vtkOpeningImageFilter& );// Not implemented
  void operator=(const vtkOpeningImageFilter&);// Not implemented

private:
  //BTX
  int Radius;
  //ETX
};

#endif // VTK_OPENING_IMAGE_FILTER_H
