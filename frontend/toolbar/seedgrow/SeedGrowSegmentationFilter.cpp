/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "SeedGrowSegmentationFilter.h"
#include "SetupWidget.h"

// EspinaModel
#include "common/model/Channel.h"
#include "common/model/EspinaModel.h"
#include "common/model/Segmentation.h"
#include "common/processing/pqFilter.h"
#include "common/processing/pqData.h"
#include "common/cache/CachedObjectBuilder.h"

#include <QSpinBox>
#include <QLayout>
#include <QMessageBox>
#include <QCryptographicHash>

#include <common/model/EspinaFactory.h>
#include <vtkAlgorithm.h>
#include <vtkImageData.h>
#include <vtkAlgorithmOutput.h>
#include <itkImage.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkExtractImageFilter.h>

#include <QDebug>

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId SeedGrowSegmentationFilter::CHANNEL = ArgumentId("Channel", true);
const ArgumentId SeedGrowSegmentationFilter::SEED = ArgumentId("Seed", true);
const ArgumentId SeedGrowSegmentationFilter::LTHRESHOLD = ArgumentId("LowerThreshold", true);
const ArgumentId SeedGrowSegmentationFilter::UTHRESHOLD = ArgumentId("UpperThreshold", true);
const ArgumentId SeedGrowSegmentationFilter::VOI = ArgumentId("VOI", true);

const unsigned char LABEL_VALUE = 255;


//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SArguments::SArguments(const ModelItem::Arguments args)
: Arguments(args)
{
  QStringList seed = args[SEED].split(",");
  m_seed[0] = seed[0].toInt();
  m_seed[1] = seed[1].toInt();
  m_seed[2] = seed[2].toInt();

  m_threshold[0] = args[LTHRESHOLD].toInt();
  m_threshold[1] = args[UTHRESHOLD].toInt();

  QStringList voi = args[VOI].split(",");
  m_VOI[0] = voi[0].toInt();
  m_VOI[1] = voi[1].toInt();
  m_VOI[2] = voi[2].toInt();
  m_VOI[3] = voi[3].toInt();
  m_VOI[4] = voi[4].toInt();
  m_VOI[5] = voi[5].toInt();
}


//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(SelectableItem* input, int seed[3], int threshold[2], int VOI[6])
: m_seg(NULL)
, m_segImg(vtkImageData::New())
, m_filterOutput(NULL)
{
  m_args.setInput(input->id());
  m_args.setSeed(seed);
  m_args.setLowerThreshold(threshold[0]);
  m_args.setUpperThreshold(threshold[1]);
  m_args.setVOI(VOI);

  extract = NULL;
  grow = NULL;
  close = NULL;
  segFilter = NULL;

  run();
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(ModelItem::Arguments args)
: m_args(args)
, m_seg(NULL)
, m_segImg(vtkImageData::New())
, m_filterOutput(NULL)
{
//   qDebug() << args;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  extract = NULL;
  grow = NULL;
  close = NULL;
  segFilter = NULL;

  Q_ASSERT(false);

//   QString segId = id() + "_0";
//   if ((segFilter = cob->loadFile(segId)) == NULL)
//   {
//     int VOI[6];
//     m_args.voi(VOI);
// 
//     //   const int W = 200;
//     //   int VOI[6] = {seed[0].toInt() - W, seed[0].toInt() + W,
//     //                 seed[1].toInt() - W, seed[1].toInt() + W,
//     //                 seed[2].toInt() - W, seed[2].toInt() + W};
// 
//     // //   VOI[0] = VOI[2] = 0;
//     // //   //VOI[1] = 698;
//     // //   VOI[1] = 2264;
//     // //   //VOI[3] = 535;
//     // //   VOI[3] = 2104;
//     //   VOI[4] = 0;
//     //   VOI[5] = 0;
// 
// 
//     for (int i = 0; i < 1; i++)
//     {
//       if (grow)
// 	cob->removeFilter(grow);
//       if (extract)
// 	cob->removeFilter(extract);
// 
//       pqFilter::Arguments extractArgs;
//       extractArgs << pqFilter::Argument("Input",pqFilter::Argument::INPUT, args[CHANNEL]);
//       QString VolumeArg = QString("%1,%2,%3,%4,%5,%6").arg(VOI[0]).arg(VOI[1]).arg(VOI[2]).arg(VOI[3]).arg(VOI[4]).arg(VOI[5]);
//       extractArgs << pqFilter::Argument("VOI",pqFilter::Argument::INTVECT, VolumeArg);
//       extract = cob->createFilter("filters","ExtractGrid", extractArgs);
//       //     qDebug() << "Extract Args:" << extractArgs;
//       Q_ASSERT(extract->getNumberOfData() == 1);
// 
//       // Hacer el grow
//       pqFilter::Arguments growArgs;
//       growArgs << pqFilter::Argument("Input",    pqFilter::Argument::INPUT,   extract->data(0).id());
//       growArgs << pqFilter::Argument("Seed",     pqFilter::Argument::INTVECT, args[SEED]);
//       growArgs << pqFilter::Argument("Threshold",pqFilter::Argument::INTVECT, args[THRESHOLD]);
//       //     qDebug() << "Grow Args:" << growArgs;
// 
//       grow = cob->createFilter("filters","SeedGrowSegmentationFilter", growArgs);
// 
//       Q_ASSERT(grow->getNumberOfData() == 1);
// 
//       vtkImageData *volume = vtkImageData::SafeDownCast(grow->algorithm()->GetOutputPort());
//       int segExtent[6];
//       volume->GetExtent(segExtent);
// 
//       segFilter = grow;
// 
//       if (memcmp(segExtent, VOI, 6*sizeof(int)) == 0)
// 	break;
//       else
// 	memcpy(VOI,segExtent,6*sizeof(int));
//       //   qDebug() << bounds [0] << bounds [1] << bounds [2] << bounds [3] << bounds [4] << bounds [5];
//     }
//
//  }

  Q_ASSERT(segFilter);
  if (!m_seg)
    m_seg = EspinaFactory::instance()->createSegmentation(this, 0);
//   else
//     setSegmentationData(m_seg, segFilter->data(0));
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::~SeedGrowSegmentationFilter()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  qDebug() << "Destroying" << SGSF;

  if (grow)
    cob->removeFilter(grow);
  if (extract)
    cob->removeFilter(extract);;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::run()
{
  int voi[6];
  m_args.voi(voi);

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  pqFilter *channelReader = cob->getFilter(m_args[CHANNEL]);
  vtkImageData *input = channelReader->algorithm()->GetOutput();
  input->Update();

  qDebug() << "Bound VOI to input extent";
  int inputExtent[6];
  input->GetExtent(inputExtent);
  for (int i = 0; i < 3; i++)
  {
    int inf = 2*i, sup = 2*i+1;
    voi[inf] = std::max(voi[inf], inputExtent[inf]);
    voi[sup] = std::min(voi[sup], inputExtent[sup]);
  }

  typedef itk::Image<unsigned char, 3> SegmentationData;

  qDebug() << "Converting from VTK To ITK";
  typedef itk::VTKImageToImageFilter<SegmentationData> vtk2itkType;
  vtk2itkType::Pointer vtk2itk_filter = vtk2itkType::New();
  vtk2itk_filter->ReleaseDataFlagOn();
  vtk2itk_filter->SetInput(input);
  vtk2itk_filter->Update();

  qDebug() << "Apply VOI";
  typedef itk::ExtractImageFilter<SegmentationData, SegmentationData> extractType;
  extractType::Pointer extractFilter = extractType::New();
  extractType::InputImageRegionType roi;
  for (int i = 0; i < 3; i++)
  {
    int inf = 2*i, sup = 2*i+1;
    roi.SetIndex(i, voi[inf]);
    roi.SetSize (i, voi[sup] - voi[inf] + 1);
  }
  extractFilter->SetInput(vtk2itk_filter->GetOutput());
  extractFilter->SetExtractionRegion(roi);
  extractFilter->Update();

  qDebug() << "Computing Original Size Connected Threshold";
  typedef itk::ConnectedThresholdImageFilter<SegmentationData, SegmentationData> ConnectedThresholdFilterType;
  ConnectedThresholdFilterType::Pointer ctif = ConnectedThresholdFilterType::New();
  ctif->ReleaseDataFlagOn();
  int aseed[3];
  m_args.seed(aseed);
  double seedIntensity = input->GetScalarComponentAsDouble(aseed[0], aseed[1], aseed[2], 0);
  ctif->SetInput(extractFilter->GetOutput());
  ctif->SetReplaceValue(LABEL_VALUE);
  ctif->SetLower(std::max(seedIntensity - m_args.lowerThreshold(), 0.0));
  ctif->SetUpper(std::min(seedIntensity + m_args.upperThreshold(), 255.0));

  SegmentationData::IndexType seed; //TODO: Use class seed
  seed[0] = aseed[0]; seed[1] = aseed[1]; seed[2] = aseed[2];
  ctif->AddSeed(seed);

  qDebug() << "Intensity at Seed:" << seedIntensity;
  qDebug() << "Lower Threshold:" << m_args.lowerThreshold();
  qDebug() << "Upper Threshold:" << m_args.upperThreshold();
  qDebug() << "SEED: " << seed[0] << " " << seed[1] << " " << seed[2];

  qDebug() << "Converting from ITK to LabelMap";

  // Convert label image to label map
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<SegmentationData, LabelMapType> Image2LabelFilterType;

  Image2LabelFilterType::Pointer image2label = Image2LabelFilterType::New();
  image2label->ReleaseDataFlagOn();
  image2label->SetInput(ctif->GetOutput());
  image2label->Update();//TODO: Check if needed

//   qDebug() << "Getting Segmentation Region";
//   // Get the roi of the object
//   LabelMapType *    labelMap = image2label->GetOutput();
//   LabelObjectType * object   = labelMap->GetLabelObject(LABEL_VALUE);
//   LabelObjectType::RegionType objectROI = object->GetBoundingBox();
// //   objectROI.Print(std::cout);
//   SegExtent[0] = objectROI.GetIndex(0);
//   SegExtent[1] = SegExtent[0] + objectROI.GetSize(0) - 1;
//   SegExtent[2] = objectROI.GetIndex(1);
//   SegExtent[3] = SegExtent[2] + objectROI.GetSize(1) - 1;
//   SegExtent[4] = objectROI.GetIndex(2);
//   SegExtent[5] = SegExtent[4] + objectROI.GetSize(2) - 1;

//   vtkDebugMacro(<< "Extracting Segmentation Region");
  
//   typedef itk::ExtractImageFilter<InputImageType,InputImageType> ExtractFilterType;
//   ExtractFilterType::Pointer extract = ExtractFilterType::New();
  
//   extract->SetInput(ctif->GetOutput());
//   extract->SetExtractionRegion(objectROI);
//   extract->Update();
  //typedef itk::ImageFileWriter< OutputImageType >  WriterType;
  //WriterType::Pointer writer = WriterType::New();
  //writer->SetFileName("Test.mha");
  //writer->SetInput(extract->GetOutput());
  //writer->Write();

  qDebug() << "Converting from ITK to VTK";
  ctif->Update();

  typedef itk::ImageToVTKImageFilter<SegmentationData> itk2vtkFilterType;
  itk2vtkFilterType::Pointer itk2vtk_filter = itk2vtkFilterType::New();
  itk2vtk_filter->ReleaseDataFlagOn();

  itk2vtk_filter->SetInput(ctif->GetOutput() );
  itk2vtk_filter->Update();

  m_segImg->DeepCopy(itk2vtk_filter->GetOutput());

  m_filterOutput = m_segImg->GetProducerPort();

  if (!m_seg)
    m_seg = EspinaFactory::instance()->createSegmentation(this, 0);
//   else
//     setSegmentationData(m_seg, segFilter->data(0));

  emit modified(this);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setInput(pqData data)
{
//   Q_ASSERT(extract);
//   extract->pipelineSource()->updatePipeline();
//   vtkSMProperty *p = extract->pipelineSource()->getProxy()->GetProperty("Input");
//   Q_ASSERT(p);
//   vtkSMInputProperty *input = vtkSMInputProperty::SafeDownCast(p);
//   input->SetProxy(0,data.pipelineSource()->getProxy());
//   extract->pipelineSource()->getProxy()->UpdateVTKObjects();
//   extract->pipelineSource()->updatePipeline();
  emit modified(this);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setThreshold(int th)
{
  if (th < 0)
    return;

  m_args.setLowerThreshold(th);

// //   if (!grow)
//     run();
// 
// //   Q_ASSERT(grow);
// //   vtkSMPropertyHelper(grow->pipelineSource()->getProxy(),"Threshold").Set(&th, 1);
// //   grow->pipelineSource()->getProxy()->UpdateVTKObjects();
// // //   grow->pipelineSource()->updatePipeline();
//   emit modified(this);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setVOI(int VOI[6])
{
  m_args.setVOI(VOI);
/*
  run();

  emit modified(this);*/
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setSeed(int seed[3])
{
  m_args.setSeed(seed);
//   Q_ASSERT(grow);
//   vtkSMPropertyHelper(grow->pipelineSource()->getProxy(),"Seed").Set(seed,3);
//   grow->pipelineSource()->getProxy()->UpdateVTKObjects();
// //   grow->pipelineSource()->updatePipeline();
//   emit modified(this);
}


//-----------------------------------------------------------------------------
QString SeedGrowSegmentationFilter::id() const
{
  return m_args.hash();
}

//-----------------------------------------------------------------------------
QVariant SeedGrowSegmentationFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return SGSF;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString SeedGrowSegmentationFilter::serialize() const
{
  return m_args.serialize();
//   QString args;
//   args.append(argument("Channel", m_input));
//   QString seedArg = QString("%1,%2,%3").arg(m_seed[0]).arg(m_seed[1]).arg(m_seed[2]);
//   args.append(argument("Seed", seedArg));
//   args.append(argument("Threshold", QString::number(m_threshold)));
//   QString VolumeArg = QString("%1,%2,%3,%4,%5,%6").arg(m_VOI[0]).arg(m_VOI[1]).arg(m_VOI[2]).arg(m_VOI[3]).arg(m_VOI[4]).arg(m_VOI[5]);
//   args.append(argument("VOI", VolumeArg));
//   return args;
}


//-----------------------------------------------------------------------------
pqData SeedGrowSegmentationFilter::preview()
{
  Q_ASSERT(grow);
  return grow->data(0);
}

//-----------------------------------------------------------------------------
int SeedGrowSegmentationFilter::numProducts() const
{
  return m_filterOutput?1:0;
}

//-----------------------------------------------------------------------------
Segmentation *SeedGrowSegmentationFilter::product(int index) const
{
  if (index == 0)
    return m_seg;

  Q_ASSERT(index == 0);
}

//-----------------------------------------------------------------------------
QWidget* SeedGrowSegmentationFilter::createConfigurationWidget()
{
  return new SetupWidget(this);
}

vtkAlgorithmOutput* SeedGrowSegmentationFilter::output(unsigned int outputNb)
{
  if (m_filterOutput && outputNb == 0)
    return m_filterOutput;

  Q_ASSERT(false);
  return NULL;
}

// //-----------------------------------------------------------------------------
// SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(EspinaProduct* input, IVOI* voi, ITraceNode::Arguments& args)
// : m_applyFilter(NULL)
// , m_grow(NULL)
// , m_restoreFilter(NULL)
// , m_finalFilter(NULL)
// {
//   type = FILTER;
//   ProcessingTrace* trace = ProcessingTrace::instance();
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
// 
//   //m_args = QString("%1=%2;").arg("Sample").arg(input->label());
//   m_args = ESPinaModel_ARG("Sample", input->getArgument("Id"));
//   foreach(QString argName, args.keys())
//   {
//     m_args.append(ESPinaModel_ARG(argName, args[argName]));
//   }
//   
//   vtkProduct voiOutput(input->creator(),input->portNumber());
//   //! Executes VOI
//   if (voi)
//   {
//     m_applyFilter = voi->applyVOI(input);
//     if (m_applyFilter)
//     {
//       voiOutput = m_applyFilter->product(0);
//       m_args.append(ESPinaModel_ARG("ApplyVOI", "["+m_applyFilter->getFilterArguments() + "]"));
//     }
//   }
// 
//   //! Execute Grow Filter
//   vtkFilter::Arguments growArgs;
//   growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
//   growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
//   QStringList seed = args["Seed"].split(",");
//   m_seed[0] = seed[0].toInt();
//   m_seed[1] = seed[1].toInt();
//   m_seed[2] = seed[2].toInt();
//   
//   growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
//   m_threshold = args["Threshold"].toInt();
//   //growArgs.push_back(vtkFilter::Argument(QString("ProductPorts"),vtkFilter::INTVECT, "0"));
//   m_grow = cob->createFilter("filters","SeedGrowSegmentationFilter",growArgs);
//   
//   //! Create segmenations. SeedGrowSegmentationFilter has only 1 output
//   assert(m_grow->numProducts() == 1);
//   
//   m_finalFilter = m_grow;
//   
//   vtkProduct growOutput = m_grow->product(0);
//   //! Restore possible VOI transformation
//   if (voi)
//   {
//     m_restoreFilter = voi->restoreVOITransormation(&growOutput);
//     if (m_restoreFilter)
//     {
//       growOutput = m_restoreFilter->product(0);
//       m_finalFilter = m_restoreFilter;
//       //TODO Anadir args
//       
//     }
//   }
// 
//   assert(m_finalFilter->numProducts() == 1);
//   m_numSeg = m_finalFilter->numProducts();
//   
//   //WARNING: taking address of temporary => &m_finalFilter->product(0) ==> Need review
//   Segmentation *seg = EspinaModelFactory::instance()->CreateSegmentation(this, &m_finalFilter->product(0));
//   
//   if (voi)
//   {
//     int extent[6];
//     seg->creator()->pipelineSource()->updatePipeline();
//     seg->creator()->pipelineSource()->getProxy()->UpdatePropertyInformation();
//     seg->outputPort()->getDataInformation()->GetExtent(extent);
//     QStringList voiArgs = m_applyFilter->getFilterArguments().split(';');
//     QStringList bounds = voiArgs[2].section('=',-1).split(',');
//     for (int i=0; i < 6; i++)
//     {
// //       std::cout << extent[i] << " - " << bounds[i].toInt() << std::endl;
//       if (extent[i] == bounds[i].toInt())
//       {
// 	QString title("Seed Grow Segmentation Filter Information");
// 	QString text("New segmentation may be incomplete due to VOI restriction.");
// 	
// 	QApplication::setOverrideCursor(Qt::ArrowCursor);
// 	QApplication::setOverrideCursor(Qt::ArrowCursor);
// 	QMessageBox *msgBox = new QMessageBox(QMessageBox::Information,title,text);
// 	msgBox->show();// using exec make views loose focus
// 	QMessageBox::connect(msgBox,SIGNAL(accepted()),msgBox,SLOT(deleteLater()));
// 	QApplication::restoreOverrideCursor();
// 	QApplication::restoreOverrideCursor();
// 	break;
//       }
//     }
//   }
//   
//   // Trace EspinaFilter
//   trace->addNode(this);
//   // Connect input
//   trace->connect(input,this,"Sample");
//   // Trace Segmentation
//   trace->addNode(seg);
//   // Trace connection
//   trace->connect(this, seg,"Segmentation");
//   
//   EspinaModel::instance()->addSegmentation(seg);
// }
// 
// 
// //-----------------------------------------------------------------------------
// SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(ITraceNode::Arguments& args)
// : m_applyFilter(NULL)
// , m_grow(NULL)
// , m_restoreFilter(NULL)
// , m_finalFilter(NULL)
// {
//   foreach(QString key, args.keys())
//   {
//     if( key == "ApplyVOI" )
//       m_args.append(ESPinaModel_ARG(key, "["+ args[key] + "]"));
//     else
//       m_args.append(ESPinaModel_ARG(key, args[key]));
//   }
//   type = FILTER;
//   ProcessingTrace* trace = ProcessingTrace::instance();
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
// 
//   vtkProduct input(args["Sample"]);
// 
//   vtkProduct voiOutput(input.creator(),input.portNumber());
//   //! Executes VOI
//   if (args.contains("ApplyVOI") )
//   {
//     ITraceNode::Arguments voiArgs = ITraceNode::parseArgs(args["ApplyVOI"]);
//     m_applyFilter = EspinaPluginManager::instance()->createFilter(voiArgs["Type"],voiArgs);
// //     m_applyFilter = trace->getRegistredPlugin(voiArgs["Type"])->createFilter(voiArgs["Type"],voiArgs); // 
//     if (m_applyFilter)
//     {
//       voiOutput = m_applyFilter->product(0);
//       //m_args.append("ApplyVOI=" + applyFilter->getFileArguments());
//       //m_args.append(ESPinaModel_ARG("ApplyVOI", "["+m_applyFilter->getFilterArguments() + "]"));
//     }
//   }
// 
//   //! Execute Grow Filter
//   vtkFilter::Arguments growArgs;
//   growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
//   growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
//   QStringList seed = args["Seed"].split(",");
//   m_seed[0] = seed[0].toInt();
//   m_seed[1] = seed[1].toInt();
//   m_seed[2] = seed[2].toInt();
//   
//   growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
//   m_threshold = args["Threshold"].toInt();
//   //growArgs.push_back(vtkFilter::Argument(QString("ProductPorts"),vtkFilter::INTVECT, "0"));
//   // Disk cache. If the .seg contains .mhd files now it try to load them
// //   Cache::Index id = cob->generateId("filter", "SeedGrowSegmentationFilter", growArgs);
// //   m_grow = cob->getFilter(id);
// //   if( !m_grow )
//   m_grow = cob->createFilter("filters","SeedGrowSegmentationFilter",growArgs);
//   
//   //! Create segmenations. SeedGrowSegmentationFilter has only 1 output
//   assert(m_grow->numProducts() == 1);
// 
//   m_finalFilter = m_grow;
// 
//   vtkProduct growOutput = m_grow->product(0);
//   //! Restore possible VOI transformation
//   if (args.contains("RestoreVOI"))
//   {
//     
//     //TODO: Restore
// //     m_restoreFilter = voi->restoreVOITransormation(&growOutput);
// //     if (m_restoreFilter)
// //     {
// //       growOutput = m_restoreFilter->product(0);
// //       m_finalFilter = m_restoreFilter;
// //       //Anadir args
// //     }
//   }
// 
//   assert(m_finalFilter->numProducts() == 1);
//   m_numSeg = m_finalFilter->numProducts();
// 
//   Segmentation *seg = EspinaModelFactory::instance()->CreateSegmentation(this, &m_finalFilter->product(0));
// 
//   // Trace EspinaFilter
//   trace->addNode(this);
//   // Connect input
//   trace->connect(args["Sample"],this,"Sample");
//   // Trace Segmentation
//   trace->addNode(seg);
//   // Trace connection
//   trace->connect(this, seg,"Segmentation");
// 
//   EspinaModel::instance()->addSegmentation(seg);
// }

// //-----------------------------------------------------------------------------
// void SeedGrowSegmentationFilter::removeProduct(vtkProduct *product)
// {
//   m_numSeg = 0;
// }
// 