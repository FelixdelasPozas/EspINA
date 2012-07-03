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
#include <EspinaCore.h>

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
const ArgumentId SeedGrowSegmentationFilter::SEED = ArgumentId("Seed", true);
const ArgumentId SeedGrowSegmentationFilter::LTHRESHOLD = ArgumentId("LowerThreshold", true);
const ArgumentId SeedGrowSegmentationFilter::UTHRESHOLD = ArgumentId("UpperThreshold", true);
const ArgumentId SeedGrowSegmentationFilter::VOI = ArgumentId("VOI", true);
const ArgumentId SeedGrowSegmentationFilter::CLOSE = ArgumentId("Close", true);

const unsigned char LABEL_VALUE = 255;


//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::Parameters::Parameters(ModelItem::Arguments &args)
: m_args(args)
{
}

SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(Filter::NamedInputs inputs,
                                                       ModelItem::Arguments args)
: Filter(inputs, args)
, m_needUpdate(false)
, m_param(m_args)
, m_input(NULL)
, m_volume(NULL)
{
//   qDebug() << TYPE << "arguments" << m_args;
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::~SeedGrowSegmentationFilter()
{
  qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::needUpdate() const
{
  return (m_volume == NULL || m_needUpdate);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::run()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  int voi[6];
  m_param.voi(voi);

  Q_ASSERT(m_inputs.size() == 1);
  m_input = m_inputs.first();
  Q_ASSERT(m_input);

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
  EspinaVolume::IndexType seed = m_param.seed();
  double seedIntensity = m_input->GetPixel(seed);
  ctif = ConnectedThresholdFilterType::New();
  ctif->ReleaseDataFlagOn();
  ctif->SetInput(extractFilter->GetOutput());
  ctif->SetReplaceValue(LABEL_VALUE);
  ctif->SetLower(std::max(seedIntensity - m_param.lowerThreshold(), 0.0));
  ctif->SetUpper(std::min(seedIntensity + m_param.upperThreshold(), 255.0));
  ctif->AddSeed(seed);

//   qDebug() << "Intensity at Seed:" << seedIntensity;
//   qDebug() << "Lower Intensity:" << std::max(seedIntensity - m_param.lowerThreshold(), 0);
//   qDebug() << "Upper Intensity:" << std::min(seedIntensity + m_param.upperThreshold(), 255);
//   qDebug() << "SEED: " << seed[0] << " " << seed[1] << " " << seed[2];

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

  if (m_param.closeValue() > 0)
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

  QApplication::restoreOverrideCursor();
  emit modified(this);
  m_needUpdate = false;
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setLowerThreshold(int th)
{
  if (th < 0)
    return;

  m_param.setLowerThreshold(th);
  m_needUpdate = true;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setUpperThreshold(int th)
{
  if (th < 0)
    return;

  m_param.setUpperThreshold(th);
  m_needUpdate = true;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setVOI(int VOI[6])
{
  m_param.setVOI(VOI);
  m_needUpdate = true;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setSeed(int seed[3])
{
  m_param.setSeed(seed);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::seed(int seed[3]) const
{
  EspinaVolume::IndexType index = m_param.seed();
  for(int i=0; i<3; i++)
    seed[i] = index[i];
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
bool SeedGrowSegmentationFilter::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_volume = m_cachedFilter->GetOutput();
    emit modified(this);
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
QWidget* SeedGrowSegmentationFilter::createConfigurationWidget()
{
  return new SetupWidget(this);
}