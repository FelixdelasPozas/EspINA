#include "vtkSmartPointer.h"
#include "vtkMetaImageReader.h"
#include "vtkImageMapper.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <cstdio>
#include <vtkRenderWindowInteractor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAppendPolyData.h>
#include <vtkSphereSource.h>

#include "vtkBoundingRegionFilter.h"
#include "vtkCountingRegionFilter.h"
#include <vtkImageData.h>
#include <vtkInformation.h>


int main(int argc, char **argv)
{
  vtkSmartPointer<vtkMetaImageReader> reader =
    vtkSmartPointer<vtkMetaImageReader>::New();

  reader->SetFileName(argv[1]);

  // Get the bounding region
  vtkSmartPointer<vtkBoundingRegionFilter> region =
    vtkSmartPointer<vtkBoundingRegionFilter>::New();
  region->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkCountingRegionFilter> counting =
    vtkSmartPointer<vtkCountingRegionFilter>::New();
  counting->SetInputConnection(0,reader->GetOutputPort());
  counting->SetInputConnection(1,region->GetOutputPort());

  // Display the  region
  vtkSmartPointer<vtkPolyDataMapper> regionMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> regionActor =
    vtkSmartPointer<vtkActor>::New();
  regionMapper->SetInputConnection(region->GetOutputPort());
  regionActor->SetMapper(regionMapper);
  regionActor->GetProperty()->SetColor(1, 0, 0);

  // Display valid images
  vtkSmartPointer<vtkImageActor> validActor =
    vtkSmartPointer<vtkImageActor>::New();
  validActor->SetInput(counting->GetOutput(0));
  std::cout << "Number of valid inputs: " << counting->GetOutputPortInformation(0)->GetNumberOfKeys() << std::endl;

  vtkSmartPointer<vtkImageActor> discartedActor =
    vtkSmartPointer<vtkImageActor>::New();
  discartedActor->SetInput(counting->GetOutput(1));
  std::cout << "Number of discarted inputs: " << counting->GetOutputPortInformation(1)->GetNumberOfKeys() << std::endl;

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  interactor->SetRenderWindow(renderWindow);
  renderWindow->AddRenderer(renderer);
  renderer->AddActor(regionActor);
  renderWindow->Render(); // Centers on region instead of image
  renderer->AddActor(validActor);
  //renderer->AddActor(discartedActor);
  renderWindow->Render();
  interactor->Start();

  return 0;
}
