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
#include <QDir>

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
  // Retrive Trace
  QString TraceAux(Trace);
  // Retrive Taxonomy
  QString taxAux(Taxonomy);
  // Retrive path of the new file
  QString FileNameAux(FileName);
  
  // Retrive the files of the segmentations
  
  QStringList filters;
  filters << "*.pvd";
  QDir segDir(QString(FileName).remove(QRegExp("\\..*$")));

  QStringList segFiles = segDir.entryList(filters);
  for(int i=0; i < segFiles.count(); i++)
  {
    segFiles[i] = segDir.filePath(segFiles[i]);
  }
  qDebug() << segFiles;
  IOEspinaFile::saveFile(FileNameAux, TraceAux, taxAux, segFiles);

  // Delete the segmentation files
  foreach( FileNameAux, segFiles)
    QFile::remove(FileNameAux);
  
  return 1;
}

//---------------------------------------------------------------------------
void vtkSegWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << this->FileName << " - " << this->Trace << " - " << this->Taxonomy;
}

