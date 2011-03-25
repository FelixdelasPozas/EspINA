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

#include <vtkDenseArray.h>


int main(int argc, char **argv)
{
  vtkSmartPointer<vtkMetaImageReader> sample =
    vtkSmartPointer<vtkMetaImageReader>::New();
  sample->SetFileName(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> seg =
    vtkSmartPointer<vtkMetaImageReader>::New();
  seg->SetFileName(argv[2]);

  std::cout << argv[2] << std::endl;
  // Get the bounding region
  vtkSmartPointer<vtkBoundingRegionFilter> region =
    vtkSmartPointer<vtkBoundingRegionFilter>::New();
  region->SetInputConnection(sample->GetOutputPort());

  vtkSmartPointer<vtkCountingRegionFilter> counting =
    vtkSmartPointer<vtkCountingRegionFilter>::New();
  counting->SetInputConnection(0,seg->GetOutputPort());
  counting->SetInputConnection(1,region->GetOutputPort());
  counting->Update();

  // Display the  region
  vtkSmartPointer<vtkPolyDataMapper> regionMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> regionActor =
    vtkSmartPointer<vtkActor>::New();
  regionMapper->SetInputConnection(region->GetOutputPort());
  regionActor->SetMapper(regionMapper);
  regionActor->GetProperty()->SetColor(1, 0, 0);

  // Display valid images
  vtkSmartPointer<vtkImageActor> segActor =
    vtkSmartPointer<vtkImageActor>::New();
  segActor->SetInput(seg->GetOutput(0));
  
  vtkArrayData *res = vtkArrayData::SafeDownCast(counting->GetOutput());
  vtkDenseArray<int> *discarted = vtkDenseArray<int>::SafeDownCast(res->GetArray(0));
  std::cout << "Discarted: " << counting->GetDiscarted() << std::endl;
  /*
  res = vtkArrayData::SafeDownCast(counting->GetOutput());
  discarted = vtkDenseArray<int>::SafeDownCast(res->GetArray(0));
  std::cout << "Discarted: " << discarted->GetValue(0) << std::endl;
  */

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  //vtkSmartPointer<vtkRenderWindowInteractor> interactor =
  //  vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  //interactor->SetRenderWindow(renderWindow);
  renderWindow->AddRenderer(renderer);
  renderer->AddActor(regionActor);
  renderWindow->Render(); // Centers on region instead of image
  renderer->AddActor(segActor);
  //renderer->AddActor(discartedActor);
  renderWindow->Render();
  //interactor->Start();

  return 0;
}
