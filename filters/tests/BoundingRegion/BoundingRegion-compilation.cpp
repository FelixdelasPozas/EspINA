#include "vtkSmartPointer.h"
#include "vtkMetaImageReader.h"
#include "vtkImageMapper.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkBoundingRegionFilter.h"

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <cstdio>
#include <vtkRenderWindowInteractor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAppendPolyData.h>
#include <vtkSphereSource.h>


vtkSmartPointer<vtkActor> render_points(vtkPoints * points,vtkRenderer * renderer,double * color, float radius=30)
{
       vtkSmartPointer<vtkAppendPolyData> appender =
                       vtkSmartPointer<vtkAppendPolyData>::New();
       int n_p = points->GetNumberOfPoints();

       for (vtkIdType i = 0; i < n_p; i++) {
               vtkSmartPointer<vtkSphereSource> sphere =
                               vtkSmartPointer<vtkSphereSource>::New();
               sphere->SetCenter(points->GetPoint(i));
               sphere->SetRadius(radius);
               appender->AddInput(sphere->GetOutput());
       }
       vtkSmartPointer<vtkPolyDataMapper> mapper =
                       vtkSmartPointer<vtkPolyDataMapper>::New();
       mapper->SetInput(appender->GetOutput());
       vtkSmartPointer<vtkActor> actor =
                       vtkSmartPointer<vtkActor>::New();
       actor->SetMapper(mapper);
       vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
       prop->SetColor(color);
       renderer->AddActor(actor);
       return actor;
}

int main(int argc, char **argv)
{
  vtkSmartPointer<vtkMetaImageReader> reader =
    vtkSmartPointer<vtkMetaImageReader>::New();

  reader->SetFileName(argv[1]);
  // Pasarle el filtro que queremos probar
  vtkSmartPointer<vtkBoundingRegionFilter> region =
    vtkSmartPointer<vtkBoundingRegionFilter>::New();

  int inclusion[3] = {0,150,2};
  int exclusion[3] = {200,200,0};
  
  region->SetInclusion(inclusion);
  region->SetExclusion(exclusion);

  region->SetInputConnection(reader->GetOutputPort());


  //region->PrintSelf(std::cout, vtkIndent(0));
  // El resto es igual
  vtkSmartPointer<vtkPolyDataMapper> regionMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  regionMapper->SetInputConnection(region->GetOutputPort());
  
  
  vtkPolyData * pd= vtkPolyData::SafeDownCast(region->GetOutput());
  vtkSmartPointer<vtkActor> regionActor = vtkSmartPointer<vtkActor>::New();  
  regionActor->SetMapper(regionMapper);
  regionActor->GetProperty()->SetColor(1,0,0);
  //imageActor->SetInput( imageMapper );
  
  vtkSmartPointer<vtkImageActor> imageActor = vtkSmartPointer<vtkImageActor>::New();
  imageActor->SetInput(reader->GetOutput());
  
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  //renderer->AddActor(regionActor);
  double red[3] = {1.0,0,0};
  //vtkSmartPointer<vtkActor> regionActor = render_points(pd->GetPoints(),renderer,red);
  renderWindow->AddRenderer(renderer);
  renderer->AddActor(regionActor);
  renderWindow->Render();
  renderer->AddActor(imageActor);
  renderWindow->Render();
  interactor->Start();
  
  
  
	 /*
  vtkSmartPointer<vtkRenderer> ren1= vtkSmartPointer<vtkRenderer>::New();
  ren1->AddActor( imageActor );
  //ren1->AddActor( regionActor );
  ren1->SetBackground( 0.1, 0.2, 0.4 );
  
  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer( ren1 );
  renWin->SetSize( 600, 600 );
  renWin->Render();
  
  vtkSmartPointer<vtkRenderWindowInteractor> renWinInter = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renWinInter->SetRenderWindow(renWin);
  
  renWinInter->Initialize();
  renWinInter->Start();
*/
  return 0;
}
