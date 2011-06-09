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
  QRegExp extensionRE("\\..*$");
  filters << "*.pvd";
  QDir rootFileDir(QString(FileName).remove(extensionRE));

  // Obtain all the files with the segmentation information
  QStringList segFiles = rootFileDir.entryList(filters);
  for(int i=0; i < segFiles.count(); i++)
  {
    segFiles[i] = rootFileDir.filePath(segFiles[i]);
  }
  qDebug() << "vtkSegWriter: segmentation files" << segFiles;
  // Save Trace, Tax and Segmentation files
  IOEspinaFile::saveFile(FileNameAux, TraceAux, taxAux, segFiles);
  qDebug() << "vtkSegWriter: File "<< FileNameAux << "saved. Removing temporary files";
  // Delete the segmentation files after Save
  foreach( FileNameAux, segFiles)
  {
    // Remove the file
    QFile::remove(FileNameAux);
    // Retrieve the directory with extra information to remove it
    QDir segDir(FileNameAux.remove(extensionRE));
    // delete all the files inside and the root directory
    filters.clear();
    filters << "*.vti"; // With the .pvd files, vti files are allways generated
    foreach(QString f, segDir.entryList(filters) )
      segDir.remove(f);
    segDir.rmdir(segDir.path());
  }
  // Delete de root file if it is possible
  rootFileDir.rmdir(rootFileDir.path());
  return 1;
}

//---------------------------------------------------------------------------
void vtkSegWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << this->FileName << " - " << this->Trace << " - " << this->Taxonomy;
}

