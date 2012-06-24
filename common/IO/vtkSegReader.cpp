#include "vtkSegReader.h"

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

vtkStandardNewMacro(vtkSegReader);

//---------------------------------------------------------------------------
vtkSegReader::vtkSegReader()
{
  this->FileName = NULL;
  this->Trace = NULL;
  this->Taxonomy = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkSegReader::~vtkSegReader()
{
  //delete [] Taxonomy;
  //delete [] Trace;
}

//---------------------------------------------------------------------------
int vtkSegReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  //vtkFileContent* outData = vtkFileContent::SafeDownCast(outInfo->Get(vtkFileContent::DATA_OBJECT()));

  QString TraceContent, TaxonomyContent;
  QTextStream TraceStream(&TraceContent), TaxonomyStream(&TaxonomyContent);

  if(!IOEspinaFile::loadFile(this->GetFileName(), TraceStream, TaxonomyStream))
    return 0;

  this->SetTaxonomy(TaxonomyStream.string()->toUtf8());
  this->SetTrace(TraceStream.string()->toUtf8());

  return 1;
}

//---------------------------------------------------------------------------
void vtkSegReader::PrintSelf(ostream& os, vtkIndent indent)
{
  //this->Superclass::PrintSelf(os,indent);
  os << this->Trace << this->Taxonomy;
  /*os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
      */
}