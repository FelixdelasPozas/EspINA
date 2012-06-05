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
  QRegExp extensionRE("\\..*$");
  QDir fileRootDir(QString(FileName).remove(extensionRE));

  // Obtain all the files with the segmentation information
  QStringList segmentationPaths = fileRootDir.entryList(filters);
  // Retrive the vti files. The structure of the directories are for a given
  // fileName.pvd there is a directory called fileName/ and 1 or more files
  // fileName_\d+.vti inside fileName/ dir.
  filters.clear();
  filters << "*.vti";
  foreach(QString path, segmentationPaths)
  {
    path.remove(extensionRE);
    QDir d(fileRootDir.filePath(path));
    QStringList vtiFiles = d.entryList(filters);
    for(int i=0; i < vtiFiles.count(); i++)
      vtiFiles[i] = QDir(path).filePath(vtiFiles[i]);
    segmentationPaths.append(vtiFiles);
  }

  // Add at the beginning the path
  for(int i=0; i < segmentationPaths.count(); i++)
    segmentationPaths[i] = fileRootDir.filePath( segmentationPaths[i]);
  qDebug() << "vtkSegWriter: Segmentation paths" << segmentationPaths;

  // Save Trace, Tax and Segmentation files
  IOEspinaFile::saveFile(FileNameAux, TraceAux, taxAux, segmentationPaths, fileRootDir.filePath("").append("/"));

  qDebug() << "vtkSegWriter: File "<< FileNameAux << ". Removing temporary files";
  // Delete the segmentation files after Save: name.pvd, name/name_0.vti, name/
  foreach( QString segPath, segmentationPaths)
  {
    // Remove the files
    qDebug() << "vtkSegWriter: Removing" << segPath <<
    QFile::remove(QString( segPath));
    // delete all the files inside and the root directory
    if( segPath.endsWith(".vti") )
    {
      QDir segmentationDir( segPath );
      segmentationDir.cdUp();
      qDebug() << "Removing dir" << segmentationDir.path();
      segmentationDir.rmdir( segmentationDir.path());
    }
  }
  // Delete de root file if it is possible
  fileRootDir.rmdir(fileRootDir.path());
  return 1;
}

//---------------------------------------------------------------------------
void vtkSegWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << this->FileName << " - " << this->Trace << " - " << this->Taxonomy;
}