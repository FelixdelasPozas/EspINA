#ifndef VTKTRACEREADER_H
#define VTKTRACEREADER_H


#include <vtkAlgorithm.h>
//#include <vtkPolyDataAlgorithm.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
//#include <vtkStdString.h>

class vtkDataSet;
//class vtkFileContent;
#include "vtkFileContent.h"


class vtkRemoteFileReader: public vtkAlgorithm //vtkPolyDataAlgorithm //
{

public:
  static vtkRemoteFileReader* New();
  vtkTypeRevisionMacro(vtkRemoteFileReader, vtkAlgorithm);//vtkPolyDataAlgorithm);//vtkAlgorithm);

  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(FilePath);
  vtkGetStringMacro(FilePath);

  vtkFileContent* GetOutput();
  vtkFileContent* GetOutput(int port);

  virtual void SetOutput(vtkDataObject* d);

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector);

protected:
  vtkRemoteFileReader();
  ~vtkRemoteFileReader();

  virtual int FillOutputPortInformation(int, vtkInformation* info);
  virtual int RequestDataObject(vtkInformation* request,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
  //Paraview
  //CanReadFile( const char* filePath )
  virtual int RequestInformation(vtkInformation *request,
                                 vtkInformationVector **inputVector,
                                 vtkInformationVector *outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestUpdateExtent(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector);

  char* FilePath;

};

#endif // VTKTRACEREADER_H
