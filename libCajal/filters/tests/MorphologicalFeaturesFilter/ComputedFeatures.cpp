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
  failed = failed || floor((centroid[0]*1000)+0.5) != 415521 || floor((centroid[1]*1000)+0.5) != 347717 || floor((centroid[2]*100000)+0.5) != 429692;
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
  failed = failed || floor((10000*bpm[0])+0.5) != 33241 || floor((10000*bpm[1])+0.5) != 250345 || floor((10000*bpm[2])+0.5) != 597872;
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
  failed = failed || floor((1e6*bpa[0])+0.5) != -478568 || floor((1e7*bpa[1])+0.5) != 719678 || floor((1e6*bpa[2])+0.5) != -875096;
  failed = failed || floor((1e6*bpa[3])+0.5) != -433767 || floor((1e6*bpa[4])+0.5) != -885908 || floor((1e6*bpa[5])+0.5) != 164359;
  failed = failed || floor((1e6*bpa[6])+0.5) != -763426 || floor((1e6*bpa[7])+0.5) != 458245 || floor((1e6*bpa[8])+0.5) != 455184;
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
  failed = failed || floor((1e5*ess[0])+0.5) != 489328 || floor((1e4*ess[1])+0.5) != 134286 || floor((1e4*ess[2])+0.5) != 207523;

  return failed;
}
