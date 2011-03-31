#include "vtkRemoteFileReader.h"

#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataObject.h"

#include <iostream>
#include <fstream>

vtkStandardNewMacro(vtkTraceReader);
vtkCxxRevisionMacro(vtkTraceReader, "$Revision: 1.1 $");
//---------------------------------------------------------------------------
vtkTraceReader::vtkTraceReader()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  //this->FileName = NULL;
}
//---------------------------------------------------------------------------
vtkTraceReader::~vtkTraceReader()
{
  //this->SetFileName(0);
}
//---------------------------------------------------------------------------
void vtkTraceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkAlgorithm::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkFileContent* vtkTraceReader::GetOutput()
{
  return this->GetOutput(0);
}

//---------------------------------------------------------------------------
vtkFileContent* vtkTraceReader::GetOutput(int port)
{
  //return (vtkStdString*)(this->GetOutputDataObject(port));
  return vtkFileContent::SafeDownCast(this->GetOutputDataObject(port));
}

//---------------------------------------------------------------------------
void vtkTraceReader::SetOutput(vtkDataObject* d)
{
  this->GetExecutive()->SetOutputData(0, d);
}

//---------------------------------------------------------------------------
int vtkTraceReader::ProcessRequest(vtkInformation* request,
                                   vtkInformationVector** inputVector,
                                   vtkInformationVector* outputVector)
{
  // Create an output object of the correct type.
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
  {
    return this->RequestDataObject(request, inputVector, outputVector);
  }
  // generate the data
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
  {
    return this->RequestData(request, inputVector, outputVector);
  }

  if (request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
  {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
  }

  // execute information
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    return this->RequestInformation(request, inputVector, outputVector);
  }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkTraceReader::FillOutputPortInformation(int , vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkFileContent");
  return 1;
}

//----------------------------------------------------------------------------
int vtkTraceReader::RequestDataObject(vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{
  for (int i = 0; i < this->GetNumberOfOutputPorts(); ++i)
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(i);
    vtkFileContent* output = vtkFileContent::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    //vtkStdString* output = (vtkStdString*)(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    if (! output)
    {
      //output = NULL; //vtkStdString::New();
      output = vtkFileContent::New();
      //outInfo->Set(vtkDataObject::DATA_OBJECT(), (*output).c_str());
      outInfo->Set(vtkDataObject::DATA_OBJECT(), output);
      output->FastDelete();
      output->SetPipelineInformation(outInfo);
      this->GetOutputPortInformation(i)->Set(
        vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType());
    }
  }
  return 1;
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkTraceReader::RequestData(vtkInformation* request, 
				vtkInformationVector** inputVector, 
				vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkFileContent* outData = vtkFileContent::SafeDownCast(outInfo->Get(vtkFileContent::DATA_OBJECT()));

  std::string readed;
  std::ifstream file(this->GetFilePath());
  
  char buffer[256];
  while( !file.eof() ){
      file.getline(buffer, 256);
      readed.append(buffer);
      readed.append("\r\n"); 
  }
  file.close();

  outData->SetContent(readed.c_str());
  return 1;
}

//----------------------------------------------------------------------------
int vtkTraceReader::RequestInformation(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  // do nothing let subclasses handle it
  return 1;
}

//----------------------------------------------------------------------------
int vtkTraceReader::RequestUpdateExtent(vtkInformation* request, 
					vtkInformationVector** inputVector, 
					vtkInformationVector* outputVector)
{
  int numInputPorts = this->GetNumberOfInputPorts();
  for (int i = 0; i < numInputPorts; i++)
  {
    int numInputConnections = this->GetNumberOfInputConnections(i);
    for (int j = 0; j < numInputConnections; j++)
    {
      vtkInformation* inputInfo = inputVector[i]->GetInformationObject(j);
      inputInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
    }
  }
  return 1;
}

