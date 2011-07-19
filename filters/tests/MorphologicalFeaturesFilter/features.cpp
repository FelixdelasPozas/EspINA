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
    
  QString inputFileName = stackPath.filePath("reducedSeg2.mhd");
  segImage->SetFileName(inputFileName.toUtf8());
  segImage->Update();
  
  // Testing filter
  vtkSmartPointer<vtkMorphologicalFeaturesFilter> features =
    vtkSmartPointer<vtkMorphologicalFeaturesFilter>::New();
    
  features->AddInputConnection(0,segImage->GetOutputPort());
  features->Update();
//   features->DebugOn();
  
  bool failed = false;
  
  std::cout << "Features Computed: "  << std::endl;
  
  unsigned long size = -1;
  features->GetSize(&size);  
  std::cout << "\tSize\t\t" << size << std::endl;
  failed = failed || size == -1;

  double phySize = -1;
  features->GetPhysicalSize(&phySize);  
  std::cout << "\tPhysicalSize\t" << phySize << std::endl;
  failed = failed || phySize == -1;

  double centroid[3] = {-1,-1,1};
  features->GetCentroid(centroid);  
  std::cout << "\tCentroid\t\t" << centroid[0] << " " << centroid[1] << " " << centroid[2] << std::endl;
  failed = failed || centroid[0] == -1 || centroid[1] == -1 || centroid[2] == -1;
  
  int region[3] = {-1,-1,-1};
  features->GetRegion(region);
  std::cout << "\tRegion\t\t" << region[0] << " " << region[1] << " " << region[2] << std::endl;
  failed = failed || region[0] == -1 || region[1] == -1 || region[2] == -1;
  
  double bpm[3] = {-1,-1,-1};
  features->GetBinaryPrincipalMoments(bpm);
  std::cout << "\tBinary Princ. Moments\t" << bpm[0] << " " << bpm[1] << " " << bpm[2] << std::endl;
  failed = failed || bpm[0] == -1 || bpm[1] == -1 || bpm[2] == -1;

  double bpa[9] = 
  {
    -1,-1,-1,
    -1,-1,-1,
    -1,-1,-1
  };
  features->GetBinaryPrincipalAxes(bpa);
  std::cout << "\tBinary Princ. Axes\t" << bpa[0] << " " << bpa[1] << " " << bpa[2] << std::endl;
  std::cout << "\t\t\t" << bpa[3] << " " << bpa[4] << " " << bpa[5] << std::endl;
  std::cout << "\t\t\t" << bpa[6] << " " << bpa[7] << " " << bpa[8] << std::endl;
  failed = failed || bpa[0] == -1 || bpa[1] == -1 || bpa[2] == -1;
  failed = failed || bpa[3] == -1 || bpa[4] == -1 || bpa[5] == -1;
  failed = failed || bpa[6] == -1 || bpa[7] == -1 || bpa[8] == -1;
  
  double feret = -1;
  features->GetFeretDiameter(&feret);  
  std::cout << "\tFeret Diameter\t" << feret << std::endl;
  failed = failed || feret == -1;

  double ess[3] = {-1,-1,-1};
  features->GetEquivalentEllipsoidSize(ess);
  std::cout << "\tEquivalent Ellip. Size\t" << ess[0] << " " << ess[1] << " " << ess[2] << std::endl;
  failed = failed || ess[0] == -1 || ess[1] == -1 || ess[2] == -1;

  return failed;
}
