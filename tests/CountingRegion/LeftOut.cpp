// Testing Counting Region
#include "vtkRectangularBoundingRegionFilter.h"
#include "vtkCountingRegionFilter.h"

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
#include <vtkDenseArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include "../../../tests/fileTests.h"

int LeftOut(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> seg =
    vtkSmartPointer<vtkMetaImageReader>::New();
    
  QString segFileName = stackPath.filePath("reducedSeg1.mhd");
    seg->SetFileName(segFileName.toUtf8());
    
  // Get the bounding region 1
  vtkSmartPointer<vtkRectangularBoundingRegionFilter> region1 =
    vtkSmartPointer<vtkRectangularBoundingRegionFilter>::New();
  region1->SetInclusion(150,535,114);
  region1->SetExclusion(698,0,0);

  vtkSmartPointer<vtkCountingRegionFilter> counting =
    vtkSmartPointer<vtkCountingRegionFilter>::New();
  counting->SetInputConnection(0,seg->GetOutputPort());
  counting->AddInputConnection(0,region1->GetOutputPort());
  counting->Update();

  int discarted = counting->GetDiscarted();
  bool failed = discarted != 1;
  
//   // Display the  region
//   vtkSmartPointer<vtkPolyDataMapper> regionMapper1 =
//     vtkSmartPointer<vtkPolyDataMapper>::New();
//   vtkSmartPointer<vtkActor> regionActor =
//     vtkSmartPointer<vtkActor>::New();
//   regionMapper1->SetInputConnection(region1->GetOutputPort());
//   regionActor->SetMapper(regionMapper1);
// //   regionActor->GetProperty()->SetColor(1, 0, 0);
// 
//   // Display Segmentation
//   vtkSmartPointer<vtkImageActor> segActor =
//     vtkSmartPointer<vtkImageActor>::New();
//   segActor->SetInput(seg->GetOutput(0));
//   
//   vtkSmartPointer<vtkRenderWindow> renderWindow =
//     vtkSmartPointer<vtkRenderWindow>::New();
//   vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//    vtkSmartPointer<vtkRenderWindowInteractor>::New();
//   vtkSmartPointer<vtkRenderer> renderer =
//     vtkSmartPointer<vtkRenderer>::New();
// 
//   interactor->SetRenderWindow(renderWindow);
//   renderWindow->AddRenderer(renderer);
//   renderer->AddActor(regionActor);
//   renderWindow->Render(); // Centers on region instead of image
//   renderer->AddActor(segActor);
// //   //renderer->AddActor(discartedActor);
//   renderWindow->Render();
//   interactor->Start();

  return failed;
}
