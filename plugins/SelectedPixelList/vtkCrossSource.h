#ifndef VTK_CROSS_SOURCE_H
#define VTK_CROSS_SOURCE_H

#include "vtkImageAlgorithm.h"

// Forward Declarations
class vtkAlgorithmOutput;

//! vtkCrossSource creates a new cross image whose 
//! center is positioned at @Center
class VTK_IMAGING_EXPORT vtkCrossSource : 
  public vtkImageAlgorithm
{
  //BTX
  typedef unsigned char OutputPixelType;
  //ETX
  
public:
  static vtkCrossSource *New();
  vtkTypeMacro(vtkCrossSource,vtkImageAlgorithm);
  
  vtkGetVector3Macro(Center,int);
  vtkSetVector3Macro(Center,int);
  
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
  vtkCrossSource();
  virtual ~vtkCrossSource(){};
  
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
  
private:
    vtkCrossSource(const vtkCrossSource&);// Not implemented
    void operator=(const vtkCrossSource&);// Not implemented
    
private:
  int Center[3];
  int Radius;
};

#endif//VTK_CROSS_SOURCE_H
