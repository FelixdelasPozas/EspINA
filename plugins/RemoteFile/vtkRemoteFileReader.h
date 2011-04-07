#ifndef VTKREMOTEFILEREADER_H
#define VTKREMOTEFILEREADER_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStringArray.h>

class vtkRemoteFileReader : public vtkPolyDataAlgorithm
{
public:
    vtkTypeMacro(vtkRemoteFileReader,vtkPolyDataAlgorithm);
    static vtkRemoteFileReader *New();

    void PrintSelf(ostream& os, vtkIndent indent);

//     int ProcessRequest(vtkInformation* request,
//                        vtkInformationVector** inputVector,
//                        vtkInformationVector* outputVector);
// 
//     int RequestInformation(vtkInformation* request,
//                            vtkInformationVector** inputVector,
//                            vtkInformationVector* outputVector);
// 
//     int RequestDataObject(vtkInformation* request,
//                           vtkInformationVector** inputVector,
//                           vtkInformationVector* outputVector);
// 
//     int FillOutputPortInformation(int , vtkInformation* info);
    
    //vtkStringArray* GetContent(){ return Content;}
//     char* GetContent(){ return Content;}
    
    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    vtkSetStringMacro(Content);
    vtkGetStringMacro(Content);

protected:
    vtkRemoteFileReader();
    ~vtkRemoteFileReader() {}

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
    vtkRemoteFileReader(const vtkRemoteFileReader&);  // Not implemented.
    void operator=(const vtkRemoteFileReader&);  // Not implemented.

    char* FileName;
    char* Content;
};

/*
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

};*/

#endif // VTKREMOTEFILEREADER_H
