// Testing Counting Region
#include "vtkAdaptiveBoundingRegionFilter.h"

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

int Pipeline(int argc, char **argv)
{
  QDir stackPath(argv[1]);
    
  vtkSmartPointer<vtkMetaImageReader> sample =
    vtkSmartPointer<vtkMetaImageReader>::New();
  QString inputFileName = stackPath.filePath("peque.mhd");
    sample->SetFileName(inputFileName.toUtf8());
  
  // Get the bounding region
  vtkSmartPointer<vtkAdaptiveBoundingRegionFilter> region =
    vtkSmartPointer<vtkAdaptiveBoundingRegionFilter>::New();
  region->SetInputConnection(sample->GetOutputPort());
  region->Update();
  
  // Display the  region
  vtkSmartPointer<vtkPolyDataMapper> regionMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> regionActor =
    vtkSmartPointer<vtkActor>::New();
    
  regionMapper->SetInputConnection(region->GetOutputPort());
  regionActor->SetMapper(regionMapper);
  regionActor->GetProperty()->SetColor(1, 0, 0);

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
   vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  interactor->SetRenderWindow(renderWindow);
  renderWindow->AddRenderer(renderer);
  renderer->AddActor(regionActor);
  renderWindow->Render();
  interactor->Start();
  
  return 0;
}
