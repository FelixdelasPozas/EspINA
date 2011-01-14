#include "vtkSmartPointer.h"
#include "vtkMetaImageReader.h"
#include "vtkImageMapper.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkConnectedThresholdImageFilter.h"
#include "vtkConnectedThresholdRegionGrowImageFilter.h"


#include <cstdio>

int main(int argc, char **argv)
{
  //TODO: Cambiar esto por un reader de peque.mha
  vtkSmartPointer<vtkMetaImageReader> reader =
    vtkSmartPointer<vtkMetaImageReader>::New();
    
  //reader.GetPointer()->SetFileName("/home/jfernandez/workspace/bbp_workflow/data_experiments/Espina_files/peque.mha");
  reader->SetFileName("/home/sets/Serie2_electronico/02 JPGs_Stack_affine_crop_239-353/stacks/peque.mha");
  // Pasarle el filtro que queremos probar
  vtkSmartPointer<vtkConnectedThresholdImageFilter> segmentation =
    vtkSmartPointer<vtkConnectedThresholdImageFilter>::New();
    
  segmentation->SetInputConnection(reader->GetOutputPort());
  
  segmentation->Setm_threshold(30);
  segmentation->Setm_seed(0,0,0);
  segmentation->Update();
  
  // Es posible que haya que cambiar el mapper
  //vtkImageMapper *imageMapper = vtkImageMapper::New();
  //imageMapper->SetInputConnection( reader->GetOutputPort() );
  
  // El resto es igual
  vtkImageActor *imageActor = vtkImageActor::New();
  imageActor->SetInput(segmentation->GetOutput());
  //imageActor->SetInput( imageMapper );
  
  vtkRenderer *ren1= vtkRenderer::New();
  ren1->AddActor( imageActor );
  ren1->SetBackground( 0.1, 0.2, 0.4 );
  
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer( ren1 );
  renWin->SetSize( 600, 600 );
  renWin->Render();

  char c;
  c = getchar();

  return 0;
}
