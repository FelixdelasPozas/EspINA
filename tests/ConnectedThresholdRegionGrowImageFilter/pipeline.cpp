#include "vtkSmartPointer.h"
#include "vtkMetaImageReader.h"
#include "vtkMetaImageWriter.h"
#include "vtkImageMapper.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkConnectedThresholdRegionGrowImageFilter.h"
#include <QString>
#include <QDir>

int pipeline(int argc, char **argv)
{
  vtkSmartPointer<vtkMetaImageReader> reader =
    vtkSmartPointer<vtkMetaImageReader>::New();
  QDir stackPath(argv[1]);
  reader->SetFileName((stackPath.filePath("peque.mha")).toUtf8());
  reader->SetFileName(argv[1]);
  // Pasarle el filtro que queremos probar
  vtkSmartPointer<vtkConnectedThresholdRegionGrowImageFilter> segmentation =
    vtkSmartPointer<vtkConnectedThresholdRegionGrowImageFilter>::New();
    
  segmentation->SetInputConnection(reader->GetOutputPort());
  
  segmentation->Setm_threshold(30);
  segmentation->Setm_seed(76,177,0);
  segmentation->DebugOn();
  segmentation->Update();
  
  vtkSmartPointer<vtkMetaImageWriter> writer =
  vtkSmartPointer<vtkMetaImageWriter>::New();
  std::string outFileName("pipeline_out.mhd");
  writer->SetFileName(outFileName.c_str());
  writer->SetInputConnection(segmentation->GetOutputPort());
  writer->Write();
  //vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
  //writer->SetFileName("Test2.mha");
  //writer->SetRAWFileName("Test2.raw");
  //writer->SetInputConnection(segmentation->GetOutputPort());
  //writer->Write();
  
  // El resto es igual
//   vtkImageActor *imageActor = vtkImageActor::New();
//   imageActor->SetInput(segmentation->GetOutput());
//   //imageActor->SetInput( imageMapper );
//   
//   vtkRenderer *ren1= vtkRenderer::New();
//   ren1->AddActor( imageActor );
//   ren1->SetBackground( 0.1, 0.2, 0.4 );
//   
//   vtkRenderWindow *renWin = vtkRenderWindow::New();
//   renWin->AddRenderer( ren1 );
//   renWin->SetSize( 600, 600 );
//   renWin->Render();

  return 0;
}
