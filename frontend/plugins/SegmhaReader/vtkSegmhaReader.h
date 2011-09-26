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
    unsigned int label;
    unsigned int taxonomyId;
    unsigned char selected;
    
    SegmentationObject(const QString &line);
  };
  
  struct TaxonomyObject
  {
    QString *name;
    unsigned int label;
    unsigned char color[3];
    
    TaxonomyObject(const QString &line);
    QString &toString();
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

    vtkSetStringMacro(Taxonomy);
    vtkGetStringMacro(Taxonomy);
    
    vtkSetStringMacro(SegTaxonomies);
    vtkGetStringMacro(SegTaxonomies);
    
protected:
  vtkSegmhaReader();
  virtual ~vtkSegmhaReader();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  
  
private:
  vtkSegmhaReader(const vtkSegmhaReader&);  // Not implemented.
  void operator=(const vtkSegmhaReader&);  // Not implemented.
  
  char *FileName;
  int   NumSegmentations;
  char *Taxonomy;
  char *SegTaxonomies; 
//   char* Trace;
};

#endif // VTKSEGMHAFILEREADER_H
