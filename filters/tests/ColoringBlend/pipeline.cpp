#include "vtkSmartPointer.h"
#include "vtkMetaImageReader.h"
#include "vtkMetaImageWriter.h"
#include "vtkImageMapper.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkColoringBlend.h"
#include <QString>
#include <QDir>
#include <vtkRenderWindowInteractor.h>

int pipeline(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> bgImage =
    vtkSmartPointer<vtkMetaImageReader>::New();
  bgImage->SetFileName((stackPath.filePath("peque.mha")).toUtf8());
  
  // Pasarle el filtro que queremos probar
  vtkSmartPointer<vtkColoringBlend> blender =
    vtkSmartPointer<vtkColoringBlend>::New();
    
  blender->SetNumberOfThreads(3);
  blender->AddInputConnection(0,bgImage->GetOutputPort());
  
  blender->DebugOn();
  blender->Update();
  
  vtkSmartPointer<vtkMetaImageWriter> writer =
    vtkSmartPointer<vtkMetaImageWriter>::New();
  std::string outFileName("pipeline_out.mhd");
  writer->SetFileName(outFileName.c_str());
  writer->SetInputConnection(blender->GetOutputPort());
//   writer->Write();
  
  // El resto es igual
//   vtkImageActor *imageActor = vtkImageActor::New();
//   imageActor->SetInput(blender->GetOutput());
  //imageActor->SetZSlice(60);
  
//   vtkRenderer *ren1= vtkRenderer::New();
//   ren1->AddActor( imageActor );
//   ren1->SetBackground( 0.1, 0.2, 0.4 );
  
//   vtkSmartPointer<vtkRenderWindowInteractor> interactor =
//    vtkSmartPointer<vtkRenderWindowInteractor>::New();
   
//   vtkRenderWindow *renWin = vtkRenderWindow::New();

//   interactor->SetRenderWindow(renWin);
//   renWin->AddRenderer( ren1 );
//   renWin->SetSize( 600, 600 );
//   renWin->Render();
//   interactor->Start();

  return 0;
}
