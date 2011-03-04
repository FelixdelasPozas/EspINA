#ifndef VTKCONTENTFILE_H
#define VTKCONTENTFILE_H


#include "vtkDataObject.h"


class vtkFileContent: public vtkDataObject
{
public:
  static vtkFileContent* New();
  vtkTypeRevisionMacro(vtkFileContent, vtkDataObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //vtkGetMacro(Content, double);
  //  vtkGetStringMacro(FilePath);
  //vtkSetStringMacro(FilePath);
      
  //void SetContent(const char* name);
  //void GetContent(char* name);
  
  vtkGetStringMacro(Content);
  vtkSetStringMacro(Content);

protected:
  vtkFileContent();
  ~vtkFileContent();

private:
  vtkFileContent(const vtkFileContent&);   // Not implemented.
  void operator = (const vtkFileContent&);   // Not implemented.

  //  double Content;
  char* Content;
  //  char* FilePath;
};

#endif
