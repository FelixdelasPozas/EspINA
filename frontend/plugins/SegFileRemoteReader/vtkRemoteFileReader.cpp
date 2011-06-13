#include "vtkRemoteFileReader.h"

// #include "vtkFileContent.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkVertexGlyphFilter.h"
#include <qstring.h>
#include "FilePack.h"

vtkStandardNewMacro(vtkRemoteFileReader);

//---------------------------------------------------------------------------
vtkRemoteFileReader::vtkRemoteFileReader()
{
  this->FileName = NULL;
  this->Trace = NULL;
  this->Taxonomy = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkRemoteFileReader::~vtkRemoteFileReader()
{
  //delete [] Taxonomy;
  //delete [] Trace;
}



/*
//---------------------------------------------------------------------------
int vtkRemoteFileReader::ProcessRequest(vtkInformation* request,
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

//   if (request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
//   {
//     return this->RequestUpdateExtent(request, inputVector, outputVector);
//   }

  // execute information
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    return this->RequestInformation(request, inputVector, outputVector);
  }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkRemoteFileReader::RequestInformation(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  // do nothing let subclasses handle it
  // It should be configured when using a paralell reader
  return 1;
}
*/
#include "qdebug.h"
//---------------------------------------------------------------------------
int vtkRemoteFileReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  //vtkFileContent* outData = vtkFileContent::SafeDownCast(outInfo->Get(vtkFileContent::DATA_OBJECT()));

  QString TraceContent, TaxonomyContent;
  QTextStream TraceStream(&TraceContent), TaxonomyStream(&TaxonomyContent);

  if(IOEspinaFile::loadFile(this->GetFileName(), TraceStream, TaxonomyStream))
  {
  //qDebug() << TaxonomyStream.string()->toStdString().c_str() << "\n" << TraceStream.string()->toStdString().c_str();
  
    this->SetTaxonomy(TaxonomyStream.string()->toUtf8());
    this->SetTrace(TraceStream.string()->toUtf8());
  }
  /*
  std::string readed;
  std::ifstream file(this->GetFileName());

  char buffer[256];
  while( !file.eof() ){
      file.getline(buffer, 256);
      readed.append(buffer);
      readed.append("\n");
  }
  file.close();
//   // get the info object
//   vtkInformation *outInfo = outputVector->GetInformationObject(0);
// 
//   // get the ouptut
//    vtkFileContent *outData = vtkFileContent::SafeDownCast(
//             outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Here is where you would read the data from the file. In this example,
  // we simply create a point.
  //std::cout << readed.c_str() << std::endl;
  this->SetContent(readed.c_str());

  /*
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(0.0, 0.0, 0.0);
  polydata->SetPoints(points);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter =
    vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polydata->GetProducerPort());
  glyphFilter->Update();
  */
  //output->ShallowCopy(output);

  return 1;
}
/*
//----------------------------------------------------------------------------
int vtkRemoteFileReader::RequestDataObject(vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{
  
  for (int i = 0; i < this->GetNumberOfOutputPorts(); ++i)
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(i);
    vtkFileContent* output = vtkFileContent::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    //vtkDataObject* output = outInfo->Get(vtkDataObject::DATA_OBJECT());
    
    if (! output)
    {
    
      output = vtkFileContent::New();
    
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
int vtkRemoteFileReader::FillOutputPortInformation(int , vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkFileContent");
  return 1;
}
*/

//---------------------------------------------------------------------------
void vtkRemoteFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  //this->Superclass::PrintSelf(os,indent);
  os << this->Trace << this->Taxonomy;
  /*os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
      */
}



/*
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataObject.h"

#include <iostream>
#include <fstream>


vtkStandardNewMacro(vtkRemoteFileReader);
vtkCxxRevisionMacro(vtkRemoteFileReader, "$Revision: 1.1 $");
//---------------------------------------------------------------------------
vtkRemoteFileReader::vtkRemoteFileReader()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  //this->FileName = NULL;
}
//---------------------------------------------------------------------------
vtkRemoteFileReader::~vtkRemoteFileReader()
{
  this->SetFilePath(0);
}
//---------------------------------------------------------------------------
void vtkRemoteFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkAlgorithm::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkFileContent* vtkRemoteFileReader::GetOutput()
{
  return this->GetOutput(0);
}

//---------------------------------------------------------------------------
vtkFileContent* vtkRemoteFileReader::GetOutput(int port)
{
  //return (vtkStdString*)(this->GetOutputDataObject(port));
  return vtkFileContent::SafeDownCast(this->GetOutputDataObject(port));
}

//---------------------------------------------------------------------------
void vtkRemoteFileReader::SetOutput(vtkDataObject* d)
{
  this->GetExecutive()->SetOutputData(0, d);
}

int vtkRemoteFileReader::ProcessRequest(vtkInformation* request,
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
int vtkRemoteFileReader::FillOutputPortInformation(int , vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkFileContent");
  return 1;
}

//----------------------------------------------------------------------------
int vtkRemoteFileReader::RequestDataObject(vtkInformation* request,
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
int vtkRemoteFileReader::RequestData(vtkInformation* request,
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
int vtkRemoteFileReader::RequestInformation(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  // do nothing let subclasses handle it
  return 1;
}

//----------------------------------------------------------------------------
int vtkRemoteFileReader::RequestUpdateExtent(vtkInformation* request,
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

*/