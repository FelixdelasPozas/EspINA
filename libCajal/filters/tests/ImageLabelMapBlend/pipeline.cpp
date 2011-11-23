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
#include <vtkRenderWindowInteractor.h>

#include "../../../tests/fileTests.h"

int pipeline(int argc, char **argv)
{
  QDir stackPath(argv[1]);
  
  vtkSmartPointer<vtkMetaImageReader> bgImage =
    vtkSmartPointer<vtkMetaImageReader>::New();
    
  QString inputFileName = stackPath.filePath("peque.mhd");
  bgImage->SetFileName(inputFileName.toUtf8());
  
  // Pasarle el filtro que queremos probar
  vtkSmartPointer<vtkImageLabelMapBlend> blender =
    vtkSmartPointer<vtkImageLabelMapBlend>::New();
    
//   blender->SetNumberOfThreads(3);
  blender->AddInputConnection(0,bgImage->GetOutputPort());
  
  blender->DebugOn();
  blender->Update();
  
  vtkSmartPointer<vtkMetaImageWriter> writer =
    vtkSmartPointer<vtkMetaImageWriter>::New();
  writer->SetFileName("pipeline_out.mhd");
  writer->SetInputConnection(blender->GetOutputPort());
  writer->Write();
  
  
  bool failed = fileDiff(stackPath.filePath("tests/ImageLabelMapBlend/pipeline_out.mhd"),
		  "pipeline_out.mhd") ||
	fileDiff(stackPath.filePath("tests/ImageLabelMapBlend/pipeline_out.zraw"),
		 "pipeline_out.zraw");
	
  // In case the test fails, we may want to analyze the output
  if (!failed)
  {
    remove("pipeline_out.mhd");
    remove("pipeline_out.zraw");
  }
  
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

  return failed;
}
