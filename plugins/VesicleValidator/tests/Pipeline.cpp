// Testing Counting Region
#include "vtkCrossSource.h"

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

int Pipeline(int argc, char **argv)
{
  vtkSmartPointer<vtkCrossSource> cross =
    vtkSmartPointer<vtkCrossSource>::New();
  cross->SetCenter(3,3,0);
  cross->Update();
  
  // Display the  region
  vtkSmartPointer<vtkImageActor> crossActor =
    vtkSmartPointer<vtkImageActor>::New();
    
  crossActor->SetInput(cross->GetOutput());

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
   vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(crossActor);

  interactor->SetRenderWindow(renderWindow);
  renderWindow->AddRenderer(renderer);
  renderWindow->Render();
  interactor->Start();
  
  return 0;
}
