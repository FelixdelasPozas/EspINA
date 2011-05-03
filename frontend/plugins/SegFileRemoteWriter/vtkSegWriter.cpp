#include "vtkSegWriter.h"

// #include "vtkFileContent.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkVertexGlyphFilter.h"
#include <qstring.h>
#include <qdebug.h>
#include <qfile.h>
#include "FilePack.h"

vtkStandardNewMacro(vtkSegWriter);

//---------------------------------------------------------------------------
vtkSegWriter::vtkSegWriter()
{
  qDebug() << "vtkSegWriter: vtkSegWriter created!";
  this->FileName = NULL;
  this->Trace = NULL;
  this->Taxonomy = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
int vtkSegWriter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{

  FilePack pack( FileName, FilePack::WRITE );

  // Retrive ProcessingTrace
  QString s(Trace);
//   qDebug() << "Remote Trace " << s;
  pack.addSource(FilePack::TRACE, s);
  // Retrive Taxonomy
  QString tax_data(Taxonomy);
//   qDebug() << "Remote Taxonomy " << tax_data;
  pack.addSource(FilePack::TAXONOMY, tax_data);

  pack.close();

/*  QFile file(this->GetFileName()); // File.seg
  file.open(QIODevice::Truncate | QIODevice::WriteOnly);
  file.write(this->GetContent());
  file.close();*/
  
  return 1;
}

//---------------------------------------------------------------------------
void vtkSegWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << this->FileName << " - " << this->Trace << " - " << this->Taxonomy;
}

