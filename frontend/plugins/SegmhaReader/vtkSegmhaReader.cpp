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

typedef itk::Image<unsigned short,3> 			SegmhaImageType;
typedef itk::Image<unsigned  char,3> 			EspinaImageType;
typedef itk::ImageFileReader<SegmhaImageType> 		ImageReaderType;
typedef itk::ImageToVTKImageFilter<SegmhaImageType> 	SegmhaToVTKImageFilterType;
typedef itk::VTKImageToImageFilter<SegmhaImageType> 	VTKImageToImageFilterType;
typedef itk::StatisticsLabelObject<unsigned int, 3> 	LabelObjectType;
typedef itk::LabelMap<LabelObjectType> LabelMapType;
typedef itk::LabelImageToShapeLabelMapFilter< 
	      SegmhaImageType, LabelMapType> 		Image2LabelFilterType;
typedef itk::LabelMapToLabelImageFilter<
	      LabelMapType, EspinaImageType> 		Label2ImageFilterType;
typedef itk::ExtractImageFilter<
	      EspinaImageType, EspinaImageType> 	ExtractFilterType;
typedef itk::ImageToVTKImageFilter<EspinaImageType> 	ImageToVTKImageFilterType;
  


vtkStandardNewMacro(vtkSegmhaReader);

vtkSegmhaReader::SegmentationObject::SegmentationObject(const QString& line)
{
  QStringList elements = line.split(" ");
  
  label 	= elements[1].split("=")[1].toUInt();
  taxonomyId 	= elements[2].split("=")[1].toUInt();
  selected 	= elements[3].split("=")[1].toUInt();
}

vtkSegmhaReader::TaxonomyObject::TaxonomyObject(const QString& line)
{
  QStringList elements = line.split(" ");
  
  name     = new QString();
  *name    = elements[1].split("=")[1].replace("\"","");
  label    = elements[2].split("=")[1].toInt();
  color[0] = elements[4].replace(',',"").toInt();
  color[1] = elements[5].replace(',',"").toInt();
  color[2] = elements[6].toInt();
}

QString& vtkSegmhaReader::TaxonomyObject::toString()
{
  QString *string = new QString(QString("%1 %2 %3 %4 %5;")
  .arg(label)
  .arg(*name)
  .arg(color[0])
  .arg(color[1])
  .arg(color[2]))
  ;
  return *string;
}



//---------------------------------------------------------------------------
vtkSegmhaReader::vtkSegmhaReader()
{
  this->FileName = NULL;
  this->NumSegmentations = 0;
  this->Taxonomy = NULL;
  this->SegTaxonomies = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkSegmhaReader::~vtkSegmhaReader()
{
}


void parseCountingBrick(QString line, int cb[6])
{
  QStringList margins = line.split('=');
  QStringList inclusive = margins[1].split(',');
  QStringList exclusive = margins[2].split(',');
  
  cb[0] = inclusive[0].section('[',-1).toInt();
  cb[1] = inclusive[1].toInt();
  cb[2] = inclusive[2].section(']',0,0).toInt();
  
  cb[3] = exclusive[0].section('[',-1).toInt();
  cb[3] = exclusive[1].toInt();
  cb[4] = exclusive[2].section(']',0,0).toInt();
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
  QString taxonomies;
  QString segTaxonomies;
  while (!(line = stream.readLine()).isNull())
  {
    QString infoType = line.split(":")[0];
    
    if (infoType == "Object")
    {
      SegmentationObject seg(line);
      metaData.push_back(seg);
      segTaxonomies.append(QString::number(seg.taxonomyId)).append(";");
    }
    else if (infoType == "Segment")
    {
      TaxonomyObject tax(line);
      taxonomies.append(tax.toString());
      std::cout << tax.toString().toStdString() << std::endl;
    }
    else if (infoType == "Counting Brick")
    {
      int cb[6];
      parseCountingBrick(line,cb);   
      this->SetCountingBrick(cb);
    }
  }
  metaDataReader.close();
  this->NumSegmentations = metaData.size();
  this->SetSegTaxonomies(segTaxonomies.toUtf8());
//   std::cout << "Total Number of Segmentations: " << NumSegmentations << std::endl;
  this->SetTaxonomy(taxonomies.toUtf8());
//   std::cout << "Total Number of Taxonomies: " << taxonomies.split(";").size() << std::endl;
  
  
  vtkDebugMacro(<< "Reading ITK image from file");
  // Read the original image, whose pixels are indeed labelmap object ids
  imageReader->SetFileName(FileName);
  imageReader->SetImageIO(itk::MetaImageIO::New());
  imageReader->Update();

  vtkDebugMacro(<< "Invert ITK image's slices");
  // EspINA python used an inversed representation of the samples
  SegmhaToVTKImageFilterType::Pointer originalImage =
    SegmhaToVTKImageFilterType::New();
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
    std::cout << "Loading Segmentations " << blockNo << "..." << std::endl;
//     std::cout << "\tLabel: " << QString::number(seg.label).toStdString() << std::endl;
//     std::cout << "\tSegment: " << QString::number(seg.taxonomyId).toStdString() << std::endl;
    LabelMapType *    labelMap = image2label->GetOutput();
//     std::cout << "Number of labels: " << labelMap->GetNumberOfLabelObjects() << std::endl;
    LabelObjectType * object   = labelMap->GetLabelObject(seg.label);
    LabelObjectType::RegionType region = object->GetRegion();
    
    LabelMapType::Pointer tmpLabelMap = 
      LabelMapType::New();
    tmpLabelMap->CopyInformation(labelMap);
    object->SetLabel(255);
    tmpLabelMap->AddLabelObject(object);
    tmpLabelMap->Update();
    
    Label2ImageFilterType::Pointer label2image =
      Label2ImageFilterType::New();
    label2image->SetInput(tmpLabelMap);
    label2image->Update();
  
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
    segImage->CopyInformation(itk2vtk_filter->GetOutput());//WARNING: don't forget!

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