#ifndef VTKSEGREADER_H
#define VTKSEGREADER_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStringArray.h>

class vtkSegReader : public vtkPolyDataAlgorithm
{
public:
    vtkTypeMacro(vtkSegReader,vtkPolyDataAlgorithm);
    static vtkSegReader *New();

    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    vtkSetStringMacro(Trace);
    vtkGetStringMacro(Trace);

    vtkSetStringMacro(Taxonomy);
    vtkGetStringMacro(Taxonomy);

protected:
    vtkSegReader();
    ~vtkSegReader();

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
    vtkSegReader(const vtkSegReader&);  // Not implemented.
    void operator=(const vtkSegReader&);  // Not implemented.

    char* FileName;
    char* Trace;
    char* Taxonomy;
};

#endif // VTKSEGREADER_H
