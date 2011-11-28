#ifndef VTKEXTRACTBLOCKASIMAGE_H
#define VTKEXTRACTBLOCKASIMAGE_H

#include <vtkImageAlgorithm.h>


//BTX
class vtkInformation;
class vtkInformationVector;
class QString;
//ETX


class vtkExtractBlockAsImage : public vtkImageAlgorithm
{
public:
    vtkTypeMacro(vtkExtractBlockAsImage, vtkImageAlgorithm);
    static vtkExtractBlockAsImage *New();

    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetMacro(ExtractBlock,int);
    vtkGetMacro(Label,int);
//     vtkSetStringMacro(Trace);
//     vtkGetStringMacro(Trace);

//     vtkSetStringMacro(Taxonomy);
//     vtkGetStringMacro(Taxonomy);
    
protected:
  vtkExtractBlockAsImage();
  virtual ~vtkExtractBlockAsImage();

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  
  
private:
  vtkExtractBlockAsImage(const vtkExtractBlockAsImage&);  // Not implemented.
  void operator=(const vtkExtractBlockAsImage&);  // Not implemented.
  
  int ExtractBlock;
  int Label;
//   char* Trace;
//   char* Taxonomy;
};

#endif // VTKSEGMHAFILEREADER_H
