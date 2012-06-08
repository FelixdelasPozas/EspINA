#ifndef __vtkImageLogicFilter_h
#define __vtkImageLogicFilter_h

#include <vtkImageAlgorithm.h>

#define ADDITION 0
#define SUBSTRACTION 1

class vtkImageLogicFilter : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkImageLogicFilter,vtkImageAlgorithm);
  static vtkImageLogicFilter *New();

  vtkSetMacro(Operation, int);
  vtkGetMacro(Operation, int);

protected:
  vtkImageLogicFilter();
  ~vtkImageLogicFilter();

  int FillInputPortInformation(int port, vtkInformation* info);
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  int Operation;
  int output_extent[6];


  vtkImageLogicFilter(const vtkImageLogicFilter&);  // Not implemented.
  void operator=(const vtkImageLogicFilter&);  // Not implemented.
};

#endif
