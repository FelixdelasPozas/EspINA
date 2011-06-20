#ifndef VTKSEGWRITER_H
#define VTKSEGWRITER_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStringArray.h>

class vtkSegWriter : public vtkPolyDataAlgorithm
{

public:
    vtkTypeMacro(vtkSegWriter,vtkPolyDataAlgorithm);
    static vtkSegWriter *New();

    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    vtkSetStringMacro(Trace);
    vtkGetStringMacro(Trace);

    vtkSetStringMacro(Taxonomy);
    vtkGetStringMacro(Taxonomy);
      
protected:
    vtkSegWriter();
    ~vtkSegWriter() {}

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
    vtkSegWriter(const vtkSegWriter&);  // Not implemented.
    void operator=(const vtkSegWriter&);  // Not implemented.

    char* FileName;
    char* Trace;
    char* Taxonomy;
};

#endif // VTKSEGWRITER_H
