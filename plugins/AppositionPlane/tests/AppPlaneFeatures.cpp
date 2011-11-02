// Testing Apposition Plane Features
#include "vtkAppositionPlaneFilter.h"
#include "vtkAppositionPlaneFeatures.h"

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
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkFixedPointVolumeRayCastMapper.h>

#include <QDir>

int AppPlaneFeatures(int argc, char **argv)
{
  QDir stackPath(argv[1]);
    
  vtkSmartPointer<vtkMetaImageReader> seg =
    vtkSmartPointer<vtkMetaImageReader>::New();
//   QString inputFileName = stackPath.filePath("object44_OUTPUT.mha");
//   QString inputFileName = stackPath.filePath("reducedSeg1.mhd");
  QString inputFileName = stackPath.filePath("object35.mha");
  seg->SetFileName(inputFileName.toUtf8());
  seg->Update();
    
  vtkSmartPointer<vtkAppositionPlaneFilter> appPlane =
    vtkSmartPointer<vtkAppositionPlaneFilter>::New();
  appPlane->DebugOn();
  appPlane->SetInputConnection(seg->GetOutputPort());
  appPlane->Update();
  
  vtkSmartPointer<vtkAppositionPlaneFeatures> appPlaneFeatures = 
    vtkSmartPointer<vtkAppositionPlaneFeatures>::New();
  appPlaneFeatures->DebugOn();
  appPlaneFeatures->SetInput(appPlane->GetOutput());
  appPlaneFeatures->Update();
  
  std::cout << "Area: " << appPlaneFeatures->GetArea() << std::endl;
  std::cout << "Perimeter: " << appPlaneFeatures->GetPerimeter() << std::endl;
  
  
  // Display the  region
  vtkSmartPointer<vtkPolyDataMapper> planeMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> planeActor =
    vtkSmartPointer<vtkActor>::New();
    
  planeMapper->SetInputConnection(appPlane->GetOutputPort());
  planeActor->SetMapper(planeMapper);
  planeActor->GetProperty()->SetColor(1, 0, 0);
  
  // Create a transfer function mapping scalar value to opacity
  vtkSmartPointer<vtkPiecewiseFunction> oTFun =
  vtkSmartPointer<vtkPiecewiseFunction>::New();
  oTFun->AddSegment(0, 0, 255, 1); 
  
  vtkSmartPointer<vtkColorTransferFunction> cTFun =
  vtkSmartPointer<vtkColorTransferFunction>::New();
  cTFun->AddRGBPoint(   0, 0.2, 1.0, 0.2 );
  cTFun->AddRGBPoint( 255, 0.0, 1.0, 0.0 );
  
  vtkSmartPointer<vtkVolumeProperty> property =
  vtkSmartPointer<vtkVolumeProperty>::New();
  property->SetScalarOpacity(oTFun);
  property->SetColor(cTFun);
  property->SetInterpolationTypeToLinear();
  property->ShadeOn();
  
  vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> mapper =
  vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
  mapper->SetBlendModeToComposite();
  mapper->SetInput(seg->GetOutput());
  
  vtkSmartPointer<vtkVolume> volume =
  vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(mapper);
  volume->SetProperty(property);

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
   vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();

  interactor->SetRenderWindow(renderWindow);
  renderWindow->AddRenderer(renderer);
  renderer->AddActor(planeActor);
//   renderer->AddViewProp(volume);
  renderWindow->Render();
  interactor->Start();
  
  return 0;
}
