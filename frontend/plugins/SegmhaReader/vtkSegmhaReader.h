#ifndef VTKSEGMHAFILEREADER_H
#define VTKSEGMHAFILEREADER_H

#include <vtkMultiBlockDataSetAlgorithm.h>


//BTX
class vtkInformation;
class vtkInformationVector;
class QString;
//ETX


class vtkSegmhaReader : public vtkMultiBlockDataSetAlgorithm
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
    vtkTypeMacro(vtkSegmhaReader, vtkMultiBlockDataSetAlgorithm);
    static vtkSegmhaReader *New();

    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    vtkGetMacro(NumSegmentations,int);
//     vtkSetStringMacro(Trace);
//     vtkGetStringMacro(Trace);

//     vtkSetStringMacro(Taxonomy);
//     vtkGetStringMacro(Taxonomy);
    
protected:
  vtkSegmhaReader();
  virtual ~vtkSegmhaReader();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  
  
private:
  vtkSegmhaReader(const vtkSegmhaReader&);  // Not implemented.
  void operator=(const vtkSegmhaReader&);  // Not implemented.
  
  char* FileName;
  int NumSegmentations;
//   char* Trace;
//   char* Taxonomy;
};

#endif // VTKSEGMHAFILEREADER_H
