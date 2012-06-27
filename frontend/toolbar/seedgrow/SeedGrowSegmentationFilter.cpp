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

#include "common/model/EspinaModel.h"

#include <QDebug>
#include <QCryptographicHash>
#include <QLayout>
#include <QMessageBox>
#include <QSpinBox>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

const QString SeedGrowSegmentationFilter::TYPE = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId SeedGrowSegmentationFilter::CHANNEL = ArgumentId("Channel", true);
const ArgumentId SeedGrowSegmentationFilter::SEED = ArgumentId("Seed", true);
const ArgumentId SeedGrowSegmentationFilter::LTHRESHOLD = ArgumentId("LowerThreshold", true);
const ArgumentId SeedGrowSegmentationFilter::UTHRESHOLD = ArgumentId("UpperThreshold", true);
const ArgumentId SeedGrowSegmentationFilter::VOI = ArgumentId("VOI", true);
const ArgumentId SeedGrowSegmentationFilter::CLOSE = ArgumentId("Close", true);

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

SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(Filter::NamedInputs inputs,
                                                       ModelItem::Arguments args)
: m_inputs(inputs)
, m_args(args)
{
  qDebug() << TYPE << "arguments" << args;
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::~SeedGrowSegmentationFilter()
{
  qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::run()
{
  int voi[6];
  m_args.voi(voi);

  QStringList input = m_args[INPUTS].split("_");
  m_input = m_inputs[input[0]]->output(input[1].toUInt());
  qDebug() << "Bound VOI to input extent";
  int inputExtent[6];
  VolumeExtent(m_input, inputExtent);
  for (int i = 0; i < 3; i++)
  {
    int inf = 2*i, sup = 2*i+1;
    voi[inf] = std::max(voi[inf], inputExtent[inf]);
    voi[sup] = std::min(voi[sup], inputExtent[sup]);
  }

  qDebug() << "Apply VOI";
  extractFilter = ExtractType::New();
  ExtractType::InputImageRegionType roi;
  for (int i = 0; i < 3; i++)
  {
    int inf = 2*i, sup = 2*i+1;
    roi.SetIndex(i, voi[inf]);
    roi.SetSize (i, voi[sup] - voi[inf] + 1);
  }
  extractFilter->SetNumberOfThreads(1);
  extractFilter->SetInput(m_input);
  extractFilter->SetExtractionRegion(roi);
  extractFilter->Update();

  qDebug() << "Computing Original Size Connected Threshold";
  ctif = ConnectedThresholdFilterType::New();
  ctif->ReleaseDataFlagOn();
  int aseed[3];
  m_args.seed(aseed);
  EspinaVolume::IndexType index;
  index[0] = aseed[0];
  index[1] = aseed[1];
  index[2] = aseed[2];
  double seedIntensity = m_input->GetPixel(index);
  ctif->SetInput(extractFilter->GetOutput());
  ctif->SetReplaceValue(LABEL_VALUE);
  ctif->SetLower(std::max(seedIntensity - m_args.lowerThreshold(), 0.0));
  ctif->SetUpper(std::min(seedIntensity + m_args.upperThreshold(), 255.0));

  EspinaVolume::IndexType seed; //TODO: Use class seed
  seed[0] = aseed[0]; seed[1] = aseed[1]; seed[2] = aseed[2];
  ctif->AddSeed(seed);

  qDebug() << "Intensity at Seed:" << seedIntensity;
  qDebug() << "Lower Threshold:" << m_args.lowerThreshold();
  qDebug() << "Upper Threshold:" << m_args.upperThreshold();
  qDebug() << "SEED: " << seed[0] << " " << seed[1] << " " << seed[2];

  qDebug() << "Converting from ITK to LabelMap";
  image2label = Image2LabelFilterType::New();
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

  if (m_args.closeValue() > 0)
  {
    qDebug() << "Closing Segmentation";
    StructuringElementType ball;
    ball.SetRadius(4);
    ball.CreateStructuringElement();

    bmcif = bmcifType::New();
    bmcif->SetInput(ctif->GetOutput());
    bmcif->SetKernel(ball);
    bmcif->SetForegroundValue(LABEL_VALUE);
    //   bmcif->ReleaseDataFlagOn();
    bmcif->Update();

    m_volume = bmcif->GetOutput();
  }
  else
    m_volume = ctif->GetOutput();

  emit modified(this);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setLowerThreshold(int th)
{
  if (th < 0)
    return;

  m_args.setLowerThreshold(th);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setUpperThreshold(int th)
{
  if (th < 0)
    return;

  m_args.setUpperThreshold(th);
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
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString SeedGrowSegmentationFilter::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
int SeedGrowSegmentationFilter::numberOutputs() const
{
  return m_volume?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* SeedGrowSegmentationFilter::output(OutputNumber i) const
{
  if (m_volume && i == 0)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//-----------------------------------------------------------------------------
QWidget* SeedGrowSegmentationFilter::createConfigurationWidget()
{
  return new SetupWidget(this);
}