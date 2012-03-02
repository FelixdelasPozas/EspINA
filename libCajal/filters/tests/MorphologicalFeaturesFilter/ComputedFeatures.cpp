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

int ComputedFeatures(int argc, char **argv)
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
  size = features->GetSize();  
  std::cout << "\tSize\t\t" << size << std::endl;
  failed = failed || size != 357;

  double phySize = -1;
  phySize = features->GetPhysicalSize();  
  std::cout << "\tPhysicalSize\t" << phySize << std::endl;
  failed = failed || phySize != 714;
  if (failed)
    return 1;

  double centroid[3] = {-1,-1,1};
  features->GetCentroid(centroid);  
  std::cout << "\tCentroid\t\t" << centroid[0] << " " << centroid[1] << " " << centroid[2] << std::endl;
  failed = failed || round(centroid[0]*1000) != 415521 || round(centroid[1]*1000) != 347717 || round(centroid[2]*100000) != 429692;
  if (failed)
    return 1;
  
  int region[3] = {-1,-1,-1};
  features->GetRegion(region);
  std::cout << "\tRegion\t\t" << region[0] << " " << region[1] << " " << region[2] << std::endl;
  failed = failed || region[0] != 27 || region[1] != 25 || region[2] != 8;
  if (failed)
    return 1;
  
  double bpm[3] = {-1,-1,-1};
  features->GetBinaryPrincipalMoments(bpm);
  std::cout << "\tBinary Princ. Moments\t" << bpm[0] << " " << bpm[1] << " " << bpm[2] << std::endl;
  failed = failed || round(10000*bpm[0]) != 33241 || round(10000*bpm[1]) != 250345 || round(10000*bpm[2]) != 597872;
  if (failed)
    return 1;

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
  failed = failed || round(1e6*bpa[0]) != -478568 || round(1e7*bpa[1]) != 719678 || round(1e6*bpa[2]) != -875096;
  failed = failed || round(1e6*bpa[3]) != -433767 || round(1e6*bpa[4]) != -885908 || round(1e6*bpa[5]) != 164359;
  failed = failed || round(1e6*bpa[6]) != -763426 || round(1e6*bpa[7]) != 458245 || round(1e6*bpa[8]) != 455184;
  if (failed)
    return 1;
  
  double feret = -1;
  feret = features->GetFeretDiameter();  
  std::cout << "\tFeret Diameter\t" << feret << std::endl;
  failed = failed || round(1e3*feret) != 30348;
  if (failed)
    return 1;

  double ess[3] = {-1,-1,-1};
  features->GetEquivalentEllipsoidSize(ess);
  std::cout << "\tEquivalent Ellip. Size\t" << ess[0] << " " << ess[1] << " " << ess[2] << std::endl;
  failed = failed || round(1e5*ess[0]) != 489328 || round(1e4*ess[1]) != 134286 || round(1e4*ess[2]) != 207523;

  return failed;
}
