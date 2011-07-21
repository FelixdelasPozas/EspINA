//Testing ImageLabelMapBlend
#include "vtkMorphologicalFeaturesFilter.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageMapper.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <QString>

#include "../../../tests/fileTests.h"

int pipeline(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> segImage =
    vtkSmartPointer<vtkMetaImageReader>::New();
    
  QString inputFileName = stackPath.filePath("reducedSeg1.mhd");
  segImage->SetFileName(inputFileName.toUtf8());
  
  // Testing filter
  vtkSmartPointer<vtkMorphologicalFeaturesFilter> features =
    vtkSmartPointer<vtkMorphologicalFeaturesFilter>::New();
    
  features->AddInputConnection(0,segImage->GetOutputPort());
  features->DebugOn();
//   features->Update();
  
  bool failed = false;
  
  return failed;
}
