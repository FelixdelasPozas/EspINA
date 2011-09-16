#include "vtkSegmhaReader.h"

#include <QDebug>

// #include "vtkFileContent.h"

#include <vtkMultiBlockDataSet.h>
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkVertexGlyphFilter.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <itkVTKImageToImageFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkExtractImageFilter.h>


#include <QFile>
#include <QString>
#include <QStringList>

typedef itk::Image<unsigned char,3> 			ImageType;
typedef itk::ImageFileReader<ImageType> 		ImageReaderType;
typedef itk::ImageToVTKImageFilter<ImageType> 		ImageToVTKImageFilterType;
typedef itk::VTKImageToImageFilter<ImageType> 		VTKImageToImageFilterType;
typedef itk::StatisticsLabelObject<unsigned int, 3> 	LabelObjectType;
typedef itk::LabelMap<LabelObjectType> LabelMapType;
typedef itk::LabelImageToShapeLabelMapFilter<ImageType,
	      LabelMapType> 				Image2LabelFilterType;
typedef itk::LabelMapToLabelImageFilter<LabelMapType,
	      ImageType> 				Label2ImageFilterType;
typedef itk::ExtractImageFilter<ImageType,ImageType> 	ExtractFilterType;
  


vtkStandardNewMacro(vtkSegmhaReader);

vtkSegmhaReader::SegmentationObject::SegmentationObject(const QString& line)
{
  QStringList elements = line.split(" ");
  
  label 	= elements[1].split("=")[1].toInt();
  taxonomyId 	= elements[2].split("=")[1].toInt();
  selected 	= elements[3].split("=")[1].toInt();
}


//---------------------------------------------------------------------------
vtkSegmhaReader::vtkSegmhaReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkSegmhaReader::~vtkSegmhaReader()
{
}



//---------------------------------------------------------------------------
int vtkSegmhaReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  
  vtkDebugMacro(<< "Reading segmentation's meta data from file");
  QList<SegmentationObject> metaData;
  
  QFile metaDataReader(FileName);
  metaDataReader.open(QIODevice::ReadOnly);
  QTextStream stream(&metaDataReader);
  
  QString line;
  while (!(line = stream.readLine()).isNull())
  {
    QString infoType = line.split(":")[0];
    
    if (infoType == "Object")
      metaData.push_back(SegmentationObject(line));
    else if (infoType == "Segment")
      std::cout << line.toStdString() << std::endl;
  }
  metaDataReader.close();
  
  NumSegmentations = metaData.size();
  
  vtkDebugMacro(<< "Reading ITK image from file");
  // Read the original image, whose pixels are indeed labelmap object ids
  imageReader->SetFileName(FileName);
  imageReader->SetImageIO(itk::MetaImageIO::New());
  imageReader->Update();

  vtkDebugMacro(<< "Invert ITK image's slices");
  // EspINA python used an inversed representation of the samples
  ImageToVTKImageFilterType::Pointer originalImage =
    ImageToVTKImageFilterType::New();
  originalImage->SetInput(imageReader->GetOutput());
  originalImage->Update();
  
  vtkSmartPointer<vtkImageReslice> reslicer = 
    vtkSmartPointer<vtkImageReslice>::New();
  reslicer->SetInput(originalImage->GetOutput());
  reslicer->SetResliceAxesDirectionCosines(1,0,0,0,-1,0,0,0,-1);
  reslicer->Update();
  
  vtkSmartPointer<vtkImageChangeInformation> infoChanger = 
    vtkSmartPointer<vtkImageChangeInformation>::New();
  infoChanger->SetInput(reslicer->GetOutput());
  infoChanger->SetInformationInput(originalImage->GetOutput());
  infoChanger->Update();
  
  VTKImageToImageFilterType::Pointer vtk2itk_filter = 
    VTKImageToImageFilterType::New();
  vtk2itk_filter->SetInput(infoChanger->GetOutput());
  vtk2itk_filter->Update();
  
  vtkDebugMacro(<< "Converting from ITK to LabelMap");
  // Convert labeled image to label map
  Image2LabelFilterType::Pointer image2label =
    Image2LabelFilterType::New();
  image2label->SetInput(vtk2itk_filter->GetOutput());
  image2label->Update();//TODO: Check if needed
  
  
  // Configure the output multiblock
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT())
  );
  output->SetNumberOfBlocks(NumSegmentations);
  
  vtkDebugMacro(<< "Extract all label objects from LabelMap");
  int blockNo = 0;
  foreach(SegmentationObject seg, metaData)
  {
    LabelMapType *    labelMap = image2label->GetOutput();
    LabelObjectType * object   = labelMap->GetLabelObject(seg.label);
    LabelObjectType::RegionType region = object->GetRegion();
    
    LabelMapType::Pointer tmpLabelMap = 
      LabelMapType::New();
    tmpLabelMap->CopyInformation(labelMap);
    object->SetLabel(255);
    tmpLabelMap->AddLabelObject(object);
    
    Label2ImageFilterType::Pointer label2image =
      Label2ImageFilterType::New();
    label2image->SetInput(tmpLabelMap);
  
    ExtractFilterType::Pointer extract =
      ExtractFilterType::New();
    extract->SetInput(label2image->GetOutput());
    extract->SetExtractionRegion(region);
    extract->Update();
    
    // Convert each object to vtk image
    ImageToVTKImageFilterType::Pointer itk2vtk_filter =
      ImageToVTKImageFilterType::New();
    itk2vtk_filter->SetInput( extract->GetOutput() );
    itk2vtk_filter->Update();
  
    vtkSmartPointer<vtkImageData> segImage = 
      vtkSmartPointer<vtkImageData>::New();
    segImage->DeepCopy( itk2vtk_filter->GetOutput() );
    
    // Without these lines, the output will appear real but will not work as the input to any other filters
    segImage->SetExtent(itk2vtk_filter->GetOutput()->GetExtent());
//     segImage->SetSpacing(imageReader->GetOutput()->gets->GetSpacing());
    segImage->SetWholeExtent(segImage->GetExtent());
    output->SetBlock(blockNo,segImage);
    
    blockNo++;
  }

  return 1;
}

//---------------------------------------------------------------------------
void vtkSegmhaReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
//   os << this->Trace << this->Taxonomy;
}