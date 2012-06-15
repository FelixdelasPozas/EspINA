#ifndef VTK_CLOSE_IMAGE_FILTER_H
#define VTK_CLOSE_IMAGE_FILTER_H

#include <vtkImageAlgorithm.h>

class vtkCloseImageFilter
: public vtkImageAlgorithm
{
public:
  static vtkCloseImageFilter *New();
  vtkTypeMacro(vtkCloseImageFilter,vtkImageAlgorithm);

  //! Gray level segmentation threshlod
  vtkSetMacro(Threshold,int);
  vtkGetMacro(Threshold,int);
  //! Seed coordinates
  vtkSetVector3Macro(m_seed,int);
  vtkGetVector3Macro(m_seed,int);

  void SetCheckPixel(int x, int y, int z, int value);
  void SetCheckPixel(int arg[4]);
  vtkGetVector4Macro(CheckPixel,int);

  vtkGetMacro(PixelValue,int);

  vtkGetVector6Macro(SegExtent,int);

  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkCloseImageFilter();
  ~vtkCloseImageFilter(){};

  virtual int RequestInformation(vtkInformation* request,
				 vtkInformationVector** inputVector,
				 vtkInformationVector* outputVector);
  virtual int RequestData(vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector);
    
private:
  vtkCloseImageFilter(const vtkCloseImageFilter& );// Not implemented
  void operator=(const vtkCloseImageFilter&);// Not implemented
    
private:
  int Threshold;
  int m_seed[3];
  int CheckPixel[4];
  int PixelValue;
  int SegExtent[6];
  //BTX
  vtkImageData *m_data;
  //ETX
  
};

#endif//VTK_CLOSE_IMAGE_FILTER_H
