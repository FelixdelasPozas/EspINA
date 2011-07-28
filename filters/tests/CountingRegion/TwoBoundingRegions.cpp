// Testing Counting Region
#include "vtkAdaptiveBoundingRegionFilter.h"
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

int TwoBoundingRegions(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> sample =
    vtkSmartPointer<vtkMetaImageReader>::New();
  QString inputFileName = stackPath.filePath("peque.mhd");
    sample->SetFileName(inputFileName.toUtf8());
  
  vtkSmartPointer<vtkMetaImageReader> seg =
    vtkSmartPointer<vtkMetaImageReader>::New();
    
  QString segFileName = stackPath.filePath("reducedSeg1.mhd");
    seg->SetFileName(segFileName.toUtf8());
    
  // Get the bounding region 1
  vtkSmartPointer<vtkAdaptiveBoundingRegionFilter> region1 =
    vtkSmartPointer<vtkAdaptiveBoundingRegionFilter>::New();
  region1->SetInputConnection(sample->GetOutputPort());
  region1->SetInclusion(50,50,30);
  region1->SetExclusion(50,50,0);

  // Get the bounding region 2
  vtkSmartPointer<vtkAdaptiveBoundingRegionFilter> region2 =
    vtkSmartPointer<vtkAdaptiveBoundingRegionFilter>::New();
  region2->SetInputConnection(sample->GetOutputPort());
  region2->SetInclusion(50,50,114);
  region2->SetExclusion(50,50,80);

  vtkSmartPointer<vtkCountingRegionFilter> counting =
    vtkSmartPointer<vtkCountingRegionFilter>::New();
  counting->SetInputConnection(0,seg->GetOutputPort());
  counting->AddInputConnection(0,region1->GetOutputPort());
  counting->AddInputConnection(0,region2->GetOutputPort());
  counting->Update();

//   // Display the  region
//   vtkSmartPointer<vtkPolyDataMapper> regionMapper1 =
//     vtkSmartPointer<vtkPolyDataMapper>::New();
//   vtkSmartPointer<vtkActor> regionActor =
//     vtkSmartPointer<vtkActor>::New();
//   regionMapper1->SetInputConnection(region1->GetOutputPort());
//   regionActor->SetMapper(regionMapper1);
//   regionActor->GetProperty()->SetColor(1, 0, 0);

  // Display regions
//   vtkSmartPointer<vtkImageActor> segActor =
//     vtkSmartPointer<vtkImageActor>::New();
//   segActor->SetInput(seg->GetOutput(0));
  
  std::cout << "Discarted: " << counting->GetDiscarted() << std::endl;

//   vtkSmartPointer<vtkRenderWindow> renderWindow =
//     vtkSmartPointer<vtkRenderWindow>::New();
//   vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//    vtkSmartPointer<vtkRenderWindowInteractor>::New();
//   vtkSmartPointer<vtkRenderer> renderer =
//     vtkSmartPointer<vtkRenderer>::New();

//   interactor->SetRenderWindow(renderWindow);
//   renderWindow->AddRenderer(renderer);
//   renderer->AddActor(regionActor);
//   renderWindow->Render(); // Centers on region instead of image
//   renderer->AddActor(segActor);
//   //renderer->AddActor(discartedActor);
//   renderWindow->Render();
//   interactor->Start();

  return 0;
}
