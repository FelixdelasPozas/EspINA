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

int features(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> segImage =
    vtkSmartPointer<vtkMetaImageReader>::New();
    
  QString inputFileName = stackPath.filePath("reducedSeg1.mhd");
  segImage->SetFileName(inputFileName.toUtf8());
  segImage->Update();
  
  // Testing filter
  vtkSmartPointer<vtkMorphologicalFeaturesFilter> features =
    vtkSmartPointer<vtkMorphologicalFeaturesFilter>::New();
    
  features->AddInputConnection(0,segImage->GetOutputPort());
//   features->DebugOn();
  
  bool failed = false;
  
  std::cout << "Features Computed: "  << std::endl;
  
  int centroid[3] = {-1,-1,1};
  features->GetCentroid(centroid);  
  std::cout << "\tCentroid\t" << centroid[0] << " " << centroid[1] << " " << centroid[2] << std::endl;
  failed = failed || centroid[0] == -1 || centroid[1] == -1 || centroid[2] == -1;
  
  int size = -1;
  features->GetSize(&size);  
  std::cout << "\tSize\t\t" << size << std::endl;
  failed = failed || size == -1;

  double phySize = -1;
  features->GetPhysicalSize(&phySize);  
  std::cout << "\tPhysicalSize\t" << phySize << std::endl;
  failed = failed || phySize == -1;
  
  return failed;
}
