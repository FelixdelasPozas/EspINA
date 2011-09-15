#ifndef VTKSEGMHAFILEREADER_H
#define VTKSEGMHAFILEREADER_H

#include <vtkImageAlgorithm.h>


//BTX
class vtkInformation;
class vtkInformationVector;
class QString;
//ETX


class vtkSegmhaFileReader : public vtkImageAlgorithm
{
  //BTX
  struct SegmentationObject
  {
    unsigned char label;
    unsigned char taxonomyId;
    unsigned char selected;
    
    SegmentationObject(const QString &line);
  };
  //ETX
  
public:
    vtkTypeMacro(vtkSegmhaFileReader, vtkImageAlgorithm);
    static vtkSegmhaFileReader *New();

    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    vtkGetMacro(NumSegmentations,int);
//     vtkSetStringMacro(Trace);
//     vtkGetStringMacro(Trace);

//     vtkSetStringMacro(Taxonomy);
//     vtkGetStringMacro(Taxonomy);
    
protected:
  vtkSegmhaFileReader();
  virtual ~vtkSegmhaFileReader();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  
  
private:
  vtkSegmhaFileReader(const vtkSegmhaFileReader&);  // Not implemented.
  void operator=(const vtkSegmhaFileReader&);  // Not implemented.
  
  char* FileName;
  int NumSegmentations;
//   char* Trace;
//   char* Taxonomy;
};

#endif // VTKSEGMHAFILEREADER_H
