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
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>
#include <unistd.h>

using namespace EspINA;

SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(OutputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
, m_lowerTh(0)
, m_prevLowerTh(m_lowerTh)
, m_upperTh(0)
, m_prevUpperTh(m_upperTh)
, m_seed({0,0,0})
, m_prevSeed(m_seed)
, m_radius(0)
, m_prevRadius(m_radius)
, m_usesROI(false)
{
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::restoreState(const State& state)
{
  if (!state.isEmpty())
  {
    QStringList params = state.split(";");

    QString     seedParam = params[0].split("=")[1];
    QStringList seed      = seedParam.split(","); 
    for(int i = 0; i < 3; ++i)
    {
      m_prevSeed[i] = m_seed[i] = seed[i].toDouble();
    }
    m_prevLowerTh = m_lowerTh = params[1].split("=")[1].toInt();
    m_prevUpperTh = m_upperTh = params[2].split("=")[1].toInt();
    m_prevRadius  = m_radius  = params[3].split("=")[1].toInt();
    m_usesROI = params[4].split("=")[1].toInt();
  }
}

//------------------------------------------------------------------------
State SeedGrowSegmentationFilter::state() const
{
  State state = QString("Seed=%1;LowerTh=%2;UpperTh=%3;ClosingRadius=%4;ROI=%5")
                .arg(QString("%1,%2,%3").arg(m_seed[0]).arg(m_seed[1]).arg(m_seed[2]))
                .arg(m_lowerTh)
                .arg(m_upperTh)
                .arg(m_radius)
                .arg(m_usesROI);
  return state;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setLowerThreshold(int th)
{
  m_lowerTh = th;
}

//------------------------------------------------------------------------
int SeedGrowSegmentationFilter::lowerThreshold() const
{
  return m_lowerTh;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setUpperThreshold(int th)
{
  m_upperTh = th;
}

//------------------------------------------------------------------------
int SeedGrowSegmentationFilter::upperThreshold() const
{
  return m_upperTh;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setSeed(const NmVector3& seed)
{
  m_seed = seed;
}

//------------------------------------------------------------------------
NmVector3 SeedGrowSegmentationFilter::seed() const
{
  return m_seed;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setROI(const Bounds& bounds)
{

}

//------------------------------------------------------------------------
template<typename T>
void SeedGrowSegmentationFilter::setROI(const BinaryMask<T>& mask)
{

}

//------------------------------------------------------------------------
template<typename T>
EspINA::BinaryMask<T> SeedGrowSegmentationFilter::roi() const
{

}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setClosingRadius(int radius)
{
  m_radius = radius;
}

//------------------------------------------------------------------------
int SeedGrowSegmentationFilter::closingRadius()
{
  return m_radius;;
}

//------------------------------------------------------------------------
Snapshot SeedGrowSegmentationFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::needUpdate() const
{
  return m_outputs.isEmpty();
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::needUpdate(Output::Id id) const
{
  if (id != 0) throw Undefined_Output_Exception();

  return m_outputs.isEmpty() || !validOutput(id) || ignoreStorageContent();
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::execute()
{
  execute(0);
}

//----------------------------------------------------------------------------
void SeedGrowSegmentationFilter::execute(Output::Id id)
{
  if (m_inputs.size() != 1) throw Invalid_Number_Of_Inputs_Exception();

  auto input = volumetricData(m_inputs[0]);
  if (!input) throw Invalid_Input_Data_Exception();

//   int voi[6];
//   m_param.voi(voi);
// 
// //   qDebug() << "Bound VOI to input extent";
//   int inputExtent[6];
//   input->extent(inputExtent);
//   for (int i = 0; i < 3; i++)
//   {
//     int inf = 2*i, sup = 2*i+1;
//     voi[inf] = std::max(voi[inf], inputExtent[inf]);
//     voi[sup] = std::min(voi[sup], inputExtent[sup]);
//   }
// 
// //   qDebug() << "Apply VOI";
//   voiFilter = ExtractType::New();
//   ExtractType::InputImageRegionType roi;
//   for (int i = 0; i < 3; i++)
//   {
//     int inf = 2*i, sup = 2*i+1;
//     roi.SetIndex(i, voi[inf]);
//     roi.SetSize (i, voi[sup] - voi[inf] + 1);
//   }
// 
//   voiFilter->SetNumberOfThreads(1);
//   voiFilter->SetInput(input->toITK());
//   voiFilter->SetExtractionRegion(roi);
//   voiFilter->ReleaseDataFlagOn();
//   voiFilter->Update();

  emit progress(25);
  if (!canExecute()) return;

  Q_ASSERT(contains(input->bounds(), m_seed));

  Bounds seedBounds;
  for (int i = 0; i < 3; ++i)
  {
    seedBounds[2*i] = seedBounds[2*i+1] = m_seed[i];
  }
  seedBounds.setUpperInclusion(true);

  itkVolumeType::Pointer   seedVoxel     = input->itkImage(seedBounds);
  itkVolumeType::IndexType seedIndex     = seedVoxel->GetLargestPossibleRegion().GetIndex();
  itkVolumeType::ValueType seedIntensity = seedVoxel->GetPixel(seedIndex);

  auto connectedFilter = ConnectedFilterType::New();

  connectedFilter->SetInput(input->itkImage());
  connectedFilter->SetReplaceValue(SEG_VOXEL_VALUE);
  connectedFilter->SetLower(std::max(seedIntensity - m_lowerTh, 0));
  connectedFilter->SetUpper(std::min(seedIntensity + m_upperTh, 255));
  connectedFilter->AddSeed(seedIndex);
  connectedFilter->SetNumberOfThreads(1);
  connectedFilter->ReleaseDataFlagOn();
  connectedFilter->Update();

  emit progress(75);
  if (!canExecute()) return;

//   qDebug() << "Intensity at Seed:" << seedIntensity;
//   qDebug() << "Lower Intensity:" << std::max(seedIntensity - m_param.lowerThreshold(), 0.0);
//   qDebug() << "Upper Intensity:" << std::min(seedIntensity + m_param.upperThreshold(), 255.0);
//   qDebug() << "SEED: " << seed[0] << " " << seed[1] << " " << seed[2];

//   RawSegmentationVolumeSPtr volumeRepresentation(new RawSegmentationVolume(ctif->GetOutput()));
//   volumeRepresentation->fitToContent();
// 
//   if (m_param.closeValue() > 0)
//   {
// //     qDebug() << "Closing Segmentation";
//     StructuringElementType ball;
//     ball.SetRadius(4);
//     ball.CreateStructuringElement();
// 
//     bmcif = bmcifType::New();
//     bmcif->SetInput(volumeRepresentation->toITK());
//     bmcif->SetKernel(ball);
//     bmcif->SetForegroundValue(LABEL_VALUE);
//     bmcif->ReleaseDataFlagOn();
//     bmcif->Update();
// 
//     volumeRepresentation->setVolume(bmcif->GetOutput());
//   }

  emit progress(100);
  if (!canExecute()) return;

  if (m_outputs.isEmpty())
  {
    m_outputs << OutputSPtr(new Output(this, 0));
  }

  itkVolumeType::Pointer output = connectedFilter->GetOutput();

  Bounds bounds = minimalBounds(output);

  NmVector3 spacing = m_inputs[0]->spacing();

  DefaultVolumetricDataSPtr volume{new SparseVolume<itkVolumeType>(bounds, spacing)};
  volume->draw(output, bounds);

  MeshDataSPtr mesh{new MarchingCubesMesh<itkVolumeType>(volume)};

  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(mesh);

  m_outputs[0]->setSpacing(spacing);

  m_prevLowerTh = m_lowerTh;
  m_prevUpperTh = m_upperTh;
  m_prevSeed    = m_seed;
  m_prevRadius  = m_radius;
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::ignoreStorageContent() const
{
  return m_lowerTh != m_prevLowerTh
      || m_upperTh != m_prevUpperTh
      || m_seed    != m_prevSeed
      || m_radius  != m_prevRadius;
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::invalidateEditedRegions()
{
  return false;
}

//----------------------------------------------------------------------------
Bounds SeedGrowSegmentationFilter::minimalBounds(itkVolumeType::Pointer image) const
{
  Bounds bounds;

  itk::ImageRegionConstIterator<itkVolumeType> it(image, image->GetLargestPossibleRegion());
  auto spacing = image->GetSpacing();

  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    if (it.Get())
    {
      auto index   = it.GetIndex();
      Bounds voxelBounds;
      for (int i = 0; i < 3; ++i)
      {
        voxelBounds[2*i]   = (index[i] * spacing[i]) - spacing[i]/2;
        voxelBounds[2*i+1] = ((index[i]+1) * spacing[i]) - spacing[i]/2;
      }

      if (!bounds.areValid())
        bounds = voxelBounds;
      else
        bounds = boundingBox(bounds, voxelBounds);
    }
    ++it;
  }
  
  return bounds;
}


// //-----------------------------------------------------------------------------
// bool SeedGrowSegmentationFilter::isTouchingVOI() const
// {
//   int segExtent[6];
//   SegmentationVolumeSPtr outputVolume =segmentationVolume(m_outputs[0]);
//   outputVolume->extent(segExtent);
// 
//   int voiExtent[6];
//   m_param.voi(voiExtent);
// 
//   bool incompleteSeg = false;
//   for (int i=0, j=1; i<6; i+=2, j+=2)
//   {
//     if (segExtent[i] <= voiExtent[i] || voiExtent[j] <= segExtent[j])
//       incompleteSeg = true;
//   }
// 
//   return incompleteSeg;
// }
