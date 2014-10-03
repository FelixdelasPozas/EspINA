/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "SeedGrowSegmentationFilter.h"
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.hxx>

// C++
#include <unistd.h>

// ITK
#include <itkImage.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>

using namespace ESPINA;

using ExtractFilterType      = itk::ExtractImageFilter<itkVolumeType, itkVolumeType>;
using ConnectedFilterType    = itk::ConnectedThresholdImageFilter<itkVolumeType, itkVolumeType>;
using LabelObjectType        = itk::StatisticsLabelObject<unsigned int, 3>;
using LabelMapType           = itk::LabelMap<LabelObjectType>;
using Image2LabelFilterType  = itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType>;
using StructuringElementType = itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3>;
using ClosingFilterType      = itk::BinaryMorphologicalClosingImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>;

//------------------------------------------------------------------------
SeedGrowSegmentationFilter::SeedGrowSegmentationFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Filter       {inputs, type, scheduler}
, m_lowerTh    {0}
, m_prevLowerTh{m_lowerTh}
, m_upperTh    {0}
, m_prevUpperTh{m_upperTh}
, m_seed       {{0,0,0}}
, m_prevSeed   {m_seed}
, m_radius     {0}
, m_prevRadius {m_radius}
, m_touchesROI {false}
{
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::restoreState(const State& state)
{
  for (auto token : state.split(';'))
  {
    QStringList tokens = token.split('=');
    if (tokens.size() != 2)
      continue;

    if ("Seed" == tokens[0])
    {
      QStringList seed = tokens[1].split(",");
      for(int i = 0; i < 3; ++i)
      {
        m_prevSeed[i] = m_seed[i] = seed[i].toDouble();
      }
    } else if ("LowerThreshold" == tokens[0])
    {
      m_prevLowerTh = m_lowerTh = tokens[1].toInt();
    } else if ("UpperThreshold" == tokens[0])
    {
      m_prevUpperTh = m_upperTh = tokens[1].toInt();
    } else if ("ClosingRadius" == tokens[0] || "Close" == tokens[0])
    {
      m_prevRadius  = m_radius  = tokens[1].toInt();
    }
  }
}

//------------------------------------------------------------------------
State SeedGrowSegmentationFilter::state() const
{
  State state = QString("Seed=%1;LowerThreshold=%2;UpperThreshold=%3;ClosingRadius=%4")
                .arg(QString("%1,%2,%3").arg(m_seed[0]).arg(m_seed[1]).arg(m_seed[2]))
                .arg(m_lowerTh)
                .arg(m_upperTh)
                .arg(m_radius);
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
void SeedGrowSegmentationFilter::setROI(const ROISPtr roi)
{
  m_ROI = roi;
  m_touchesROI = false;
}

//------------------------------------------------------------------------
ROISPtr SeedGrowSegmentationFilter::roi() const
{
	return m_ROI;
}

//------------------------------------------------------------------------
void SeedGrowSegmentationFilter::setClosingRadius(int radius)
{
  m_radius = radius;
}

//------------------------------------------------------------------------
int SeedGrowSegmentationFilter::closingRadius() const
{
  return m_radius;
}

//------------------------------------------------------------------------
Snapshot SeedGrowSegmentationFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::needUpdate() const
{
  return needUpdate(0);
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

  auto input = volumetricData(m_inputs[0]->output());
  if (!input) throw Invalid_Input_Data_Exception();

  Q_ASSERT(contains(input->bounds(), m_seed));

  emit progress(0);
  if (!canExecute()) return;

  Bounds seedBounds;
  for (int i = 0; i < 3; ++i)
  {
    seedBounds[2*i] = seedBounds[2*i+1] = m_seed[i];
  }
  seedBounds.setUpperInclusion(true);

  itkVolumeType::Pointer   seedVoxel     = input->itkImage(seedBounds);
  itkVolumeType::IndexType seedIndex     = seedVoxel->GetLargestPossibleRegion().GetIndex();
  itkVolumeType::ValueType seedIntensity = seedVoxel->GetPixel(seedIndex);

  int  outValue      = 0;
  auto extractFilter = ExtractFilterType::New();

  if(m_ROI != nullptr)
  {
    auto image = input->itkImage();
    auto intersectionBounds = intersection(input->bounds(), m_ROI->bounds());
    auto extractRegion      = equivalentRegion<itkVolumeType>(input->itkImage(), intersectionBounds);

    extractFilter->SetNumberOfThreads(1);
    extractFilter->SetInput(image);
    extractFilter->SetExtractionRegion(extractRegion);
    extractFilter->ReleaseDataFlagOn();
    extractFilter->Update();

    outValue = seedIntensity + m_upperTh + 1;

    if (outValue > 255)
    {
      outValue = seedIntensity - m_lowerTh - 1;
    }

    if (outValue >= 0)
    {
      m_ROI->applyROI<itkVolumeType>(extractFilter->GetOutput(), outValue);
    }
  }

  emit progress(25);
  if (!canExecute()) return;

  auto connectedFilter = ConnectedFilterType::New();

  ITKProgressReporter<ConnectedFilterType> seedProgress(this, connectedFilter, 25, 50);

  if(m_ROI != nullptr)
    connectedFilter->SetInput(extractFilter->GetOutput());
  else
    connectedFilter->SetInput(input->itkImage());

  connectedFilter->SetReplaceValue(SEG_VOXEL_VALUE);
  connectedFilter->SetLower(std::max(seedIntensity - m_lowerTh, 0));
  connectedFilter->SetUpper(std::min(seedIntensity + m_upperTh, 255));
  connectedFilter->AddSeed(seedIndex);
  connectedFilter->SetNumberOfThreads(1);
  connectedFilter->ReleaseDataFlagOn();
  connectedFilter->Update();

  emit progress(50);
  if (!canExecute()) return;

  itkVolumeType::Pointer output = connectedFilter->GetOutput();

  if(m_radius > 0)
  {
    auto closingFilter = ClosingFilterType::New();

    ITKProgressReporter<ClosingFilterType> seedProgress(this, closingFilter, 50, 75);

     //     qDebug() << "Closing Segmentation";
    StructuringElementType ball;
    ball.SetRadius(m_radius);
    ball.CreateStructuringElement();

    closingFilter->SetInput(output);
    closingFilter->SetKernel(ball);
    closingFilter->SetForegroundValue(SEG_VOXEL_VALUE);
    closingFilter->ReleaseDataFlagOn();
    closingFilter->Update();

    output = closingFilter->GetOutput();
  }

  emit progress(75);
  if (!canExecute()) return;

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = OutputSPtr(new Output(this, 0));
  }

  Bounds bounds = minimalBounds<itkVolumeType>(output, SEG_BG_VALUE);
  NmVector3 spacing = m_inputs[0]->output()->spacing();

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

  if(m_ROI != nullptr)
  	m_touchesROI = computeTouchesROIValue();

  emit progress(100);
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

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationFilter::computeTouchesROIValue() const
{
  auto volume = volumetricData(m_outputs[0]);
  auto spacing = volume->spacing();
  auto boundsSeg = volume->bounds();

  if(m_ROI->isRectangular())
  {
    auto boundsROI = m_ROI->bounds();
    for (int i = 0, j = 1; i < 6; i += 2, j += 2)
    {
      if (areEqual(boundsSeg[i], boundsROI[i], spacing[i/2]) || areEqual(boundsSeg[j], boundsROI[j], spacing[i/2]))
      {
        return true;
        break;
      }
    }
  }
  else
  {
    // the mask algorithm requires at least a voxel more to function correctly.
    Bounds extendedBounds{boundsSeg[0]-spacing[0], boundsSeg[1]+spacing[0],
                          boundsSeg[2]-spacing[1], boundsSeg[3]+spacing[1],
                          boundsSeg[4]-spacing[2], boundsSeg[5]+spacing[2]};

    // later the bounds are constrained to the channel bounds.
    auto input = volumetricData(m_inputs[0]->output());
    auto validBounds = intersection(extendedBounds, input->bounds(), spacing);

    // correct intersection bounds.
    auto intersectionBounds = intersection(validBounds, m_ROI->bounds(), spacing);
    auto roiImage = m_ROI->itkImage(intersectionBounds);

    auto region = roiImage->GetLargestPossibleRegion();
    auto regionIndex = region.GetIndex();
    auto regionLimit = regionIndex;

    auto roiMask = m_ROI->itkImage(intersectionBounds);
    auto pointer = roiMask->GetBufferPointer();
    memset(pointer, 0, region.GetSize(0)*region.GetSize(1)*region.GetSize(2));

    regionLimit.SetElement(0, regionIndex.GetElement(0) + region.GetSize(0) - 1);
    regionLimit.SetElement(1, regionIndex.GetElement(1) + region.GetSize(1) - 1);
    regionLimit.SetElement(2, regionIndex.GetElement(2) + region.GetSize(2) - 1);

    // creates an image of the roi only with the frontier values.
    itk::ImageRegionIteratorWithIndex<itk::Image<unsigned char, 3>> roiIt(roiImage, region);
    itk::ImageRegionIteratorWithIndex<itk::Image<unsigned char, 3>> maskIt(roiMask, region);

    // process Z direction
    unsigned char valueBefore = SEG_BG_VALUE;
    for(auto x = regionIndex.GetElement(0); x <= regionLimit.GetElement(0); ++x)
    {
      for(auto y = regionIndex.GetElement(1); y <= regionLimit.GetElement(1); ++y)
      {
        for(auto z = regionIndex.GetElement(2); z <= regionLimit.GetElement(2); ++z)
        {
          itkVolumeType::IndexType index{x,y,z};
          roiIt.SetIndex(index);
          maskIt.SetIndex(index);
          auto value = roiIt.Value();

          if(index.GetElement(2) == regionIndex.GetElement(2))
          {
            maskIt.Set(value);
          }
          else
          {
            if(value != valueBefore)
            {
              if(value == SEG_BG_VALUE)
              {
                index.SetElement(2,z-1);
                maskIt.SetIndex(index);
                maskIt.Set(SEG_VOXEL_VALUE);
                index.SetElement(2,z);
                maskIt.SetIndex(index);
              }
              else
                maskIt.Set(SEG_VOXEL_VALUE);
            }

            if(index.GetElement(2) == regionLimit.GetElement(2))
              maskIt.Set(value);
          }

          valueBefore = value;
        }
      }
    }

    // process Y axis
    valueBefore = SEG_BG_VALUE;
    for(auto x = regionIndex.GetElement(0); x <= regionLimit.GetElement(0); ++x)
    {
      for(auto z = regionIndex.GetElement(2); z <= regionLimit.GetElement(2); ++z)
      {
        for(auto y = regionIndex.GetElement(1); y <= regionLimit.GetElement(1); ++y)
        {
          itkVolumeType::IndexType index{x,y,z};
          roiIt.SetIndex(index);
          maskIt.SetIndex(index);
          auto value = roiIt.Value();

          if(index.GetElement(1) == regionIndex.GetElement(1))
          {
            maskIt.Set(value);
          }
          else
          {
            if(value != valueBefore)
            {
              if(value == SEG_BG_VALUE)
              {
                index.SetElement(1,y-1);
                maskIt.SetIndex(index);
                maskIt.Set(SEG_VOXEL_VALUE);
                index.SetElement(1,y);
                maskIt.SetIndex(index);
              }
              else
                maskIt.Set(SEG_VOXEL_VALUE);
            }

            if(index.GetElement(1) == regionLimit.GetElement(1))
              maskIt.Set(value);
          }

          valueBefore = value;
        }
      }
    }

    // process X axis
    valueBefore = SEG_BG_VALUE;
    for(auto y = regionIndex.GetElement(1); y <= regionLimit.GetElement(1); ++y)
    {
      for(auto z = regionIndex.GetElement(2); z <= regionLimit.GetElement(2); ++z)
      {
        for(auto x = regionIndex.GetElement(0); x <= regionLimit.GetElement(0); ++x)
        {
          itkVolumeType::IndexType index{x,y,z};
          roiIt.SetIndex(index);
          maskIt.SetIndex(index);
          auto value = roiIt.Value();

          if(index.GetElement(0) == regionIndex.GetElement(0))
          {
            maskIt.Set(value);
          }
          else
          {
            if(value != valueBefore)
            {
              if(value == SEG_BG_VALUE)
              {
                index.SetElement(0,x-1);
                maskIt.SetIndex(index);
                maskIt.Set(SEG_VOXEL_VALUE);
                index.SetElement(0,x);
                maskIt.SetIndex(index);
              }
              else
                maskIt.Set(SEG_VOXEL_VALUE);
            }

            if(index.GetElement(0) == regionLimit.GetElement(0))
              maskIt.Set(value);
          }

          valueBefore = value;
        }
      }
    }

    roiMask->Update();
    auto segImage = volume->itkImage();
    auto segRegion = segImage->GetLargestPossibleRegion();

    itk::ImageRegionIteratorWithIndex<itk::Image<unsigned char, 3>> imageIt(segImage, segRegion);
    itk::ImageRegionIteratorWithIndex<itk::Image<unsigned char, 3>> roiCheckIt(roiMask, segRegion);
    imageIt.GoToBegin();
    roiCheckIt.GoToBegin();
    while(!imageIt.IsAtEnd())
    {
      if((roiCheckIt.Value() == SEG_VOXEL_VALUE) && (imageIt.Value() == SEG_VOXEL_VALUE))
        return true;

      ++roiCheckIt;
      ++imageIt;
    }
  }

  return false;
}
