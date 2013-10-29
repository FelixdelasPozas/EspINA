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

#include "Core/Model/EspinaModel.h"
#include <Core/Model/MarchingCubesMesh.h>
#include <Core/OutputRepresentations/VolumeProxy.h>
#include <Core/OutputRepresentations/MeshProxy.h>
#include <Core/OutputRepresentations/RawVolume.h>
#include <GUI/Representations/SliceRepresentation.h>

#include <QDebug>

#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkImageData.h>

using namespace EspINA;

const QString SeedGrowSegmentationFilter::INPUTLINK = "Input";

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

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(NamedInputs inputs,
                                                       Arguments   args,
                                                       FilterType  type)
: BasicSegmentationFilter(inputs, args, type)
, m_ignoreCurrentOutputs   (false)
, m_param           (m_args)
{
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationFilter::~SeedGrowSegmentationFilter()
{
}

//-----------------------------------------------------------------------------
QVariant SeedGrowSegmentationFilter::data(int role) const
{
  if (Qt::ToolTipRole == role)
  {
    QString tooltip;

    if (!m_outputs[0]->isEdited() && isTouchingVOI())
      tooltip = condition(":/espina/voi.svg", "Touch VOI");

    return tooltip;
  } else
    return EspINA::Filter::data(role);
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::needUpdate(FilterOutputId oId) const
{
  return SegmentationFilter::needUpdate(oId);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::run()
{
  run(0);
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::run(FilterOutputId oId)
{
  //qDebug() << "Run SGS";
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() == 1);

  ChannelVolumeSPtr input = channelVolume(m_inputs[0]);
  Q_ASSERT(input);

  int voi[6];
  m_param.voi(voi);

//   qDebug() << "Bound VOI to input extent";
  int inputExtent[6];
  input->extent(inputExtent);
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
  voiFilter->SetInput(input->toITK());
  voiFilter->SetExtractionRegion(roi);
  voiFilter->ReleaseDataFlagOn();
  voiFilter->Update();

//   qDebug() << "Computing Original Size Connected Threshold";
  itkVolumeType::IndexType seed = m_param.seed();
  double seedIntensity = input->toITK()->GetPixel(seed);
  ctif = ConnectedThresholdFilterType::New();
  ctif->SetInput(voiFilter->GetOutput());
  ctif->SetReplaceValue(LABEL_VALUE);
  ctif->SetLower(std::max(seedIntensity - m_param.lowerThreshold(), 0.0));
  ctif->SetUpper(std::min(seedIntensity + m_param.upperThreshold(), 255.0));
  ctif->AddSeed(seed);
  ctif->SetNumberOfThreads(4);
  ctif->ReleaseDataFlagOn();
  ctif->Update();

//   qDebug() << "Intensity at Seed:" << seedIntensity;
//   qDebug() << "Lower Intensity:" << std::max(seedIntensity - m_param.lowerThreshold(), 0.0);
//   qDebug() << "Upper Intensity:" << std::min(seedIntensity + m_param.upperThreshold(), 255.0);
//   qDebug() << "SEED: " << seed[0] << " " << seed[1] << " " << seed[2];
  RawSegmentationVolumeSPtr volumeRepresentation(new RawSegmentationVolume(ctif->GetOutput()));
  volumeRepresentation->fitToContent();

  if (m_param.closeValue() > 0)
  {
//     qDebug() << "Closing Segmentation";
    StructuringElementType ball;
    ball.SetRadius(4);
    ball.CreateStructuringElement();

    bmcif = bmcifType::New();
    bmcif->SetInput(volumeRepresentation->toITK());
    bmcif->SetKernel(ball);
    bmcif->SetForegroundValue(LABEL_VALUE);
    bmcif->ReleaseDataFlagOn();
    bmcif->Update();

    volumeRepresentation->setVolume(bmcif->GetOutput());
  }

  SegmentationRepresentationSList repList;
  repList << volumeRepresentation;
  repList << MeshRepresentationSPtr(new MarchingCubesMesh(volumeRepresentation));

  addOutputRepresentations(0, repList);

  m_ignoreCurrentOutputs = false;

  m_outputs[0]->updateModificationTime();

  emit modified(this);
}


//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setLowerThreshold(int th, bool ignoreUpdate)
{
  if (th < 0)
    return;

  m_param.setLowerThreshold(th);
  m_ignoreCurrentOutputs = !ignoreUpdate;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setUpperThreshold(int th, bool ignoreUpdate)
{
  if (th < 0)
    return;

  m_param.setUpperThreshold(th);
  m_ignoreCurrentOutputs = !ignoreUpdate;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setVOI(int VOI[6], bool ignoreUpdate)
{
  m_param.setVOI(VOI);
  m_ignoreCurrentOutputs = !ignoreUpdate;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::isTouchingVOI() const
{
  int segExtent[6];
  SegmentationVolumeSPtr outputVolume =segmentationVolume(m_outputs[0]);
  outputVolume->extent(segExtent);

  int voiExtent[6];
  m_param.voi(voiExtent);

  bool incompleteSeg = false;
  for (int i=0, j=1; i<6; i+=2, j+=2)
  {
    if (segExtent[i] <= voiExtent[i] || voiExtent[j] <= segExtent[j])
      incompleteSeg = true;
  }

  return incompleteSeg;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setSeed(itkVolumeType::IndexType seed, bool ignoreUpdate)
{
  m_param.setSeed(seed);
  m_ignoreCurrentOutputs = !ignoreUpdate;
}

//-----------------------------------------------------------------------------
itkVolumeType::IndexType SeedGrowSegmentationFilter::seed() const
{
  return m_param.seed();
}