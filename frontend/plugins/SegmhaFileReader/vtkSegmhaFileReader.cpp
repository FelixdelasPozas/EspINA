#include "vtkSegmhaFileReader.h"

#include <QDebug>

// #include "vtkFileContent.h"

#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkVertexGlyphFilter.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkExtractImageFilter.h>


#include <QFile>
#include <QString>
#include <QStringList>


vtkStandardNewMacro(vtkSegmhaFileReader);

vtkSegmhaFileReader::SegmentationObject::SegmentationObject(const QString& line)
{
  QStringList elements = line.split(" ");
  label = elements[1].split("=")[1].toInt();
  taxonomyId = elements[2].split("=")[1].toInt();
  selected = elements[3].split("=")[1].toInt();
  std::cout << line.toStdString() << std::endl;
}


//---------------------------------------------------------------------------
vtkSegmhaFileReader::vtkSegmhaFileReader()
{
  this->FileName = NULL;
//   this->Trace = NULL;
//   this->Taxonomy = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkSegmhaFileReader::~vtkSegmhaFileReader()
{
  //delete [] Taxonomy;
  //delete [] Trace;
}



//---------------------------------------------------------------------------
int vtkSegmhaFileReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  typedef itk::Image<unsigned char,3> ImageType;
  typedef itk::ImageFileReader<ImageType> ImageFileReaderType;
  
  ImageFileReaderType::Pointer imageReader = ImageFileReaderType::New();
  
  vtkDebugMacro(<< "Reading segmentation's meta data from file");
  QFile metaDataReader(FileName);
  metaDataReader.open(QIODevice::ReadOnly);
  
  QTextStream stream(&metaDataReader);

  QList<SegmentationObject> metaData;
  
  QString line;
  while (!(line = stream.readLine()).isNull())
  {
    QString infoType = line.split(":")[0];
    
    if (infoType == "Object")
      metaData.push_back(SegmentationObject(line));
    else if (infoType == "Segment")
      std::cout << line.toStdString() << std::endl;
  }
  
  
  vtkDebugMacro(<< "Reading ITK image from file");
  // Read the original image, whose pixels are indeed labelmap object ids
  imageReader->SetFileName(FileName);
  imageReader->SetImageIO(itk::MetaImageIO::New());
  imageReader->Update();
  
  vtkDebugMacro(<< "Converting from ITK to LabelMap");
  // Convert labeled image to label map
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<ImageType, LabelMapType> Image2LabelFilterType;
  
  
  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
  image2label->SetInput(imageReader->GetOutput());
  image2label->Update();//TODO: Check if needed
  
  NumSegmentations = metaData.size();
  
  //vtkFileContent* outData = vtkFileContent::SafeDownCast(outInfo->Get(vtkFileContent::DATA_OBJECT()));
  
  vtkDebugMacro(<< "Extract all label objects from LabelMap");
  typedef itk::LabelMapToLabelImageFilter<LabelMapType, ImageType> Label2ImageFilterType;
  typedef itk::ExtractImageFilter<ImageType,ImageType> ExtractFilterType;
  typedef itk::ImageToVTKImageFilter<ImageType> itk2vtkFilterType;
  
  int lastOutput = 0;
  foreach(SegmentationObject seg, metaData)
  {
    LabelMapType *    labelMap = image2label->GetOutput();
    LabelObjectType * object   = labelMap->GetLabelObject(seg.label);
    LabelObjectType::RegionType region = object->GetRegion();
    
    LabelMapType::Pointer tmpLabelMap = LabelMapType::New();
    tmpLabelMap->CopyInformation(labelMap);
    tmpLabelMap->AddLabelObject(object);
    
    Label2ImageFilterType::Pointer label2image = Label2ImageFilterType::New();
    label2image->SetInput(tmpLabelMap);
  
    ExtractFilterType::Pointer extract = ExtractFilterType::New();
    extract->SetInput(label2image->GetOutput());
    extract->SetExtractionRegion(region);
    extract->Update();
    
    // Convert each object to vtk image
    itk2vtkFilterType::Pointer itk2vtk_filter = itk2vtkFilterType::New();
    itk2vtk_filter->SetInput( extract->GetOutput() );
    itk2vtk_filter->Update();
  
    vtkInformation* outInfo = outputVector->GetInformationObject(lastOutput++);
    vtkImageData *output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    output->DeepCopy( itk2vtk_filter->GetOutput() );
  }

//   QString TraceContent, TaxonomyContent;
//   QTextStream TraceStream(&TraceContent), TaxonomyStream(&TaxonomyContent);
  
//   if(IOEspinaFile::loadFile(this->GetFileName(), TraceStream, TaxonomyStream))
//   {
//     this->SetTaxonomy(TaxonomyStream.string()->toUtf8());
//     this->SetTrace(TraceStream.string()->toUtf8());
//   }
  /*
  std::string readed;
  std::ifstream file(this->GetFileName());

  char buffer[256];
  while( !file.eof() ){
      file.getline(buffer, 256);
      readed.append(buffer);
      readed.append("\n");
  }
  file.close();
//   // get the info object
//   vtkInformation *outInfo = outputVector->GetInformationObject(0);
// 
//   // get the ouptut
//    vtkFileContent *outData = vtkFileContent::SafeDownCast(
//             outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Here is where you would read the data from the file. In this example,
  // we simply create a point.
  //std::cout << readed.c_str() << std::endl;
  this->SetContent(readed.c_str());

  /*
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(0.0, 0.0, 0.0);
  polydata->SetPoints(points);

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter =
    vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputConnection(polydata->GetProducerPort());
  glyphFilter->Update();
  */
  //output->ShallowCopy(output);

  return 1;
}

//---------------------------------------------------------------------------
void vtkSegmhaFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
//   os << this->Trace << this->Taxonomy;
}