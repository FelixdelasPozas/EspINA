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

#include "EspinaCore.h"
#include "EspinaRegions.h"
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
const ArgumentId SeedGrowSegmentationFilter::SEED       = "Seed";
const ArgumentId SeedGrowSegmentationFilter::LTHRESHOLD = "LowerThreshold";
const ArgumentId SeedGrowSegmentationFilter::UTHRESHOLD = "UpperThreshold";
const ArgumentId SeedGrowSegmentationFilter::VOI        = "VOI";
const ArgumentId SeedGrowSegmentationFilter::CLOSE      = "Close";

const unsigned char LABEL_VALUE = 255;


//----------------------------------------------------------------------------
SeedGrowSegmentationFilter::Parameters::Parameters(ModelItem::Arguments &args)
: m_args(args)
{
}

SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(Filter::NamedInputs inputs,
                                                       ModelItem::Arguments args)
: Filter       (inputs, args)
, m_needUpdate (false)
, m_param      (m_args)
, m_input      (NULL)
{
//   qDebug() << TYPE << "arguments" << m_args;
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::~SeedGrowSegmentationFilter()
{
//   qDebug() << "Destroying" << TYPE;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::markAsModified()
{
  if (bmcif.IsNull())
    extractFilter->Modified();
  else
    bmcif->Modified();
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::needUpdate() const
{
  return (!m_outputs.contains(0) || m_needUpdate);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::releaseDataFlagOn()
{
  extractFilter->ReleaseDataFlagOn();
  if (bmcif.IsNotNull())
    bmcif->ReleaseDataFlagOn();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::releaseDataFlagOff()
{
  extractFilter->ReleaseDataFlagOn();
  if (bmcif.IsNotNull())
  {
    extractFilter->ReleaseDataFlagOn();
    bmcif->ReleaseDataFlagOff();
  }
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

//   qDebug() << "Bound VOI to input extent";
  int inputExtent[6];
  VolumeExtent(m_input, inputExtent);
  for (int i = 0; i < 3; i++)
  {
    int inf = 2*i, sup = 2*i+1;
    voi[inf] = std::max(voi[inf], inputExtent[inf]);
    voi[sup] = std::min(voi[sup], inputExtent[sup]);
  }

//   qDebug() << "Apply VOI";
  voiFilter = ExtractType::New();
  ExtractType::InputImageRegionType roi;
  for (int i = 0; i < 3; i++)
  {
    int inf = 2*i, sup = 2*i+1;
    roi.SetIndex(i, voi[inf]);
    roi.SetSize (i, voi[sup] - voi[inf] + 1);
  }
  voiFilter->SetNumberOfThreads(1);
  voiFilter->SetInput(m_input);
  voiFilter->SetExtractionRegion(roi);
  voiFilter->Update();

//   qDebug() << "Computing Original Size Connected Threshold";
  EspinaVolume::IndexType seed = m_param.seed();
  double seedIntensity = m_input->GetPixel(seed);
  ctif = ConnectedThresholdFilterType::New();
  ctif->SetInput(voiFilter->GetOutput());
  ctif->SetReplaceValue(LABEL_VALUE);
  ctif->SetLower(std::max(seedIntensity - m_param.lowerThreshold(), 0.0));
  ctif->SetUpper(std::min(seedIntensity + m_param.upperThreshold(), 255.0));
  ctif->AddSeed(seed);
  ctif->Update();

//   qDebug() << "Intensity at Seed:" << seedIntensity;
//   qDebug() << "Lower Intensity:" << std::max(seedIntensity - m_param.lowerThreshold(), 0.0);
//   qDebug() << "Upper Intensity:" << std::min(seedIntensity + m_param.upperThreshold(), 255.0);
//   qDebug() << "SEED: " << seed[0] << " " << seed[1] << " " << seed[2];

//   qDebug() << "Converting from ITK to LabelMap";
  image2label = Image2LabelFilterType::New();
  image2label->ReleaseDataFlagOn();
  image2label->SetInput(ctif->GetOutput());
  image2label->Update();//TODO: Check if needed

//   qDebug() << "Getting Segmentation Region";
  // Get the roi of the object
  LabelMapType *    labelMap = image2label->GetOutput();
  LabelObjectType * object   = labelMap->GetLabelObject(LABEL_VALUE);
  LabelObjectType::RegionType objectROI = object->GetBoundingBox();
//   vtkDebugMacro(<< "Extracting Segmentation Region");
  extractFilter = ExtractType::New();
  extractFilter->SetInput(ctif->GetOutput());
  extractFilter->SetExtractionRegion(objectROI);
  extractFilter->Update();
  //typedef itk::ImageFileWriter< OutputImageType >  WriterType;
  //WriterType::Pointer writer = WriterType::New();
  //writer->SetFileName("Test.mha");
  //writer->SetInput(extract->GetOutput());
  //writer->Write();

  if (m_param.closeValue() > 0)
  {
//     qDebug() << "Closing Segmentation";
    StructuringElementType ball;
    ball.SetRadius(4);
    ball.CreateStructuringElement();

    bmcif = bmcifType::New();
    bmcif->SetInput(extractFilter->GetOutput());
    bmcif->SetKernel(ball);
    bmcif->SetForegroundValue(LABEL_VALUE);
    bmcif->Update();

    m_outputs[0] = bmcif->GetOutput();
  }
  else
    m_outputs[0] = extractFilter->GetOutput();

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
void SeedGrowSegmentationFilter::setSeed(EspinaVolume::IndexType seed)
{
  m_param.setSeed(seed);
}

//-----------------------------------------------------------------------------
EspinaVolume::IndexType SeedGrowSegmentationFilter::seed() const
{
  return m_param.seed();
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
bool SeedGrowSegmentationFilter::prefetchFilter()
{
  if (m_needUpdate)
    return false;

  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_outputs[0] = m_cachedFilter->GetOutput();
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