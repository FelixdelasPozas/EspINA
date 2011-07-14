//Testing ImageLabelMapBlend
#include "vtkImageLabelMapBlend.h"

#include "vtkSmartPointer.h"
#include "vtkMetaImageReader.h"
#include "vtkMetaImageWriter.h"
#include "vtkImageMapper.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include <QString>
#include <QDir>
#include <vtkRenderWindowInteractor.h>

int SingleInput_Stepped(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> bgImage =
    vtkSmartPointer<vtkMetaImageReader>::New();
  bgImage->SetFileName((stackPath.filePath("peque.mha")).toUtf8());

  vtkSmartPointer<vtkMetaImageReader> input1 =
    vtkSmartPointer<vtkMetaImageReader>::New();
  input1->SetFileName((stackPath.filePath("reducedExtentSegmentation.mhd")).toUtf8());
  
//   vtkSmartPointer<vtkMetaImageReader> bgImage =
// //     vtkSmartPointer<vtkMetaImageReader>::New();
//   bgImage->SetFileName((stackPath.filePath("peque.mha")).toUtf8());
  
  
  // Pasarle el filtro que queremos probar
  vtkSmartPointer<vtkImageLabelMapBlend> blender =
    vtkSmartPointer<vtkImageLabelMapBlend>::New();
    
  blender->AddInputConnection(0, bgImage->GetOutputPort());
  
  blender->DebugOn();
  blender->Update();
  
  blender->AddInputConnection(0,input1->GetOutputPort());
  blender->Update();

  vtkSmartPointer<vtkMetaImageWriter> writer =
    vtkSmartPointer<vtkMetaImageWriter>::New();
  std::string outFileName("SeveralInputReducedExtent_out.mhd");
  writer->SetFileName(outFileName.c_str());
  writer->SetInputConnection(blender->GetOutputPort());
//   writer->Write();
  
  // El resto es igual
  vtkImageActor *imageActor = vtkImageActor::New();
  imageActor->SetInput(blender->GetOutput());
  //imageActor->SetInput( imageMapper );
  
  vtkRenderer *ren1= vtkRenderer::New();
  ren1->AddActor( imageActor );
  ren1->SetBackground( 0.1, 0.2, 0.4 );
  
  vtkSmartPointer<vtkRenderWindowInteractor> interactor =
   vtkSmartPointer<vtkRenderWindowInteractor>::New();
   
  vtkRenderWindow *renWin = vtkRenderWindow::New();

  interactor->SetRenderWindow(renWin);
  renWin->AddRenderer( ren1 );
  renWin->SetSize( 600, 600 );
  renWin->Render();
//   interactor->Start();

  return 0;
}
