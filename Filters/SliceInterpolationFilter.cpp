/*
 * Copyright (C) 2017, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "SliceInterpolationFilter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/SpatialUtils.hxx>
#include <Core/Utils/Histogram.h>
#include <Extensions/SLIC/StackSLIC.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// ITK
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkCropImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkLabelMapToBinaryImageFilter.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SliceInterpolationFilter::SliceInterpolationFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler)
: Filter     {inputs, type, scheduler}
, m_useSLIC  {false}
, m_threshold{0.5}
, m_slic     {nullptr}
{
}

//------------------------------------------------------------------------
bool SliceInterpolationFilter::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0);
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::execute()
{
  if (m_inputs.size() != 2)
  {
    auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
    auto details = QObject::tr("SliceInterpolationFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

    throw EspinaException(what, details);
  }

  reportProgress(0);
  m_errors.clear();

  auto inputSegmentation = m_inputs[0];

  bool validVolume = inputSegmentation->output()->isValid();
  if (!validVolume)
  {
    auto what = QObject::tr("Invalid input volume");
    auto details = QObject::tr("SliceInterpolationFilter::execute(id) -> Invalid input volume");

    throw EspinaException(what, details);
  }

  // Checking user correct input
  auto volume = sparseCopy<itkVolumeType>(readLockVolume(inputSegmentation->output())->itkImage());
  auto segImage = volume->itkImage();

  auto biToSlmFilter = itk::BinaryImageToShapeLabelMapFilter<itkVolumeType>::New();
  biToSlmFilter->SetInput(segImage);
  biToSlmFilter->SetInputForegroundValue(SEG_VOXEL_VALUE);
  biToSlmFilter->SetFullyConnected(true);
  biToSlmFilter->SetNumberOfThreads(1);
  biToSlmFilter->Update();

  auto slmOutput = biToSlmFilter->GetOutput();

  // Checking correct number of pieces
  const auto sliceNumber = slmOutput->GetNumberOfLabelObjects();
  if (sliceNumber < 2)
  {
    m_errors << tr("Wrong number of unconnected slices, must be two or more. Selected segmentation only has one.");
    abort();
    return;
  }

  // Get interpolation direction.
  auto auxSize = itkVolumeType::SizeType();
  auxSize.Fill(0);
  auto objects = slmOutput->GetLabelObjects();
  auto sumOperation = [&auxSize](const SLO::Pointer object) { auxSize += object->GetBoundingBox().GetSize(); };
  std::for_each(objects.begin(), objects.end(), sumOperation);

  Axis direction{Axis::UNDEFINED};
  for (int i = 0; i < itkVolumeType::ImageDimension && direction == Axis::UNDEFINED; i++)
  {
    if (auxSize[i] == sliceNumber) direction = toAxis(i);
  }

  if (direction == Axis::UNDEFINED)
  {
    m_errors << tr("Interpolation direction error. The segmentation must have all the slices in the same orientation with a depth of 1.");
    abort();
    return;
  }

  const auto dirIndex = idx(direction);

  const auto stackRegion = readLockVolume(m_inputs[1]->output())->itkImage()->GetLargestPossibleRegion();
  auto segRegion = segImage->GetLargestPossibleRegion();
  segRegion.Crop(stackRegion);

  auto commonBounds = equivalentBounds<itkVolumeType>(segImage, segRegion);
  segImage = volume->itkImage(commonBounds);

  auto stackImage = readLockVolume(m_inputs[1]->output())->itkImage(commonBounds);
  Core::Utils::Histogram original;
  original.addValues(segImage, stackImage);
  original.update();

  reportProgress(5);

  if (!canExecute()) return;

  //Sorting labels by in ascendant order
  auto lessThan = [dirIndex] (const SLO::Pointer lhs, const SLO::Pointer rhs) { return lhs->GetBoundingBox().GetIndex(dirIndex) < rhs->GetBoundingBox().GetIndex(dirIndex); };
  std::sort(objects.begin(), objects.end(), lessThan);

  stackImage = readLockVolume(m_inputs[1]->output())->itkImage();
  auto stackBounds = readLockVolume(m_inputs[1]->output())->bounds();
  const auto spacing = stackBounds.spacing();
  const auto origin  = stackBounds.origin();

  const auto parts      = objects.size();
  const auto firstSlice = objects.at(0)->GetBoundingBox().GetIndex(dirIndex);
  const auto lastSlice  = objects.at(parts-1)->GetBoundingBox().GetIndex(dirIndex);
  const auto totalSlices = (lastSlice-firstSlice-parts+1) * 2; // up + down
  int currentProgress = 0;

  // Process for each pair of pieces
  unsigned int i = 0;

  while (i < parts - 1)
  {
    if (!canExecute()) return;

    auto sourceSLO = objects.at(i);
    auto targetSLO = objects.at(i+1);
    ++i;

    reportProgress(5 + ((currentProgress * 100)/totalSlices)*0.95);

    const auto sourceRegion = sourceSLO->GetBoundingBox();
    const auto targetRegion = targetSLO->GetBoundingBox();

    // we need to pad the middle region because of the dilate and erode filters. If the dilate 'sticks'
    // to the edge of the image it is not correctly eroded, thus expanding the resulting slice when it's not
    // necessary to.
    auto middleBounds = boundingBox(equivalentBounds<itkVolumeType>(segImage, sourceRegion), equivalentBounds<itkVolumeType>(segImage, targetRegion));
    auto middleRegion = equivalentRegion<itkVolumeType>(segImage, middleBounds);
    auto paddedMiddleRegion = middleRegion;
    paddedMiddleRegion.PadByRadius(20);
    paddedMiddleRegion.SetIndex(dirIndex, middleRegion.GetIndex(dirIndex));
    paddedMiddleRegion.SetSize(dirIndex, middleRegion.GetSize(dirIndex));

    const auto paddedBounds = equivalentBounds<itkVolumeType>(origin, spacing, paddedMiddleRegion);
    auto paddedImageDown = volume->itkImage(paddedBounds);

    auto duplicator = itk::ImageDuplicator<itkVolumeType>::New();
    duplicator->SetInputImage(paddedImageDown);
    duplicator->Update();

    auto paddedImageUp = duplicator->GetOutput();

    itkVolumeType::Pointer paddedStack = nullptr;

    if(m_useSLIC && m_slic)
    {
      try
      {
        paddedStack = m_slic->getImageFromBounds(paddedBounds);
      }
      catch(const EspinaException &e)
      {
        m_errors << QString(e.details());
        abort();
        return;
      }
      catch(const std::exception &e)
      {
        m_errors << QString(e.what());
        abort();
        return;
      }
      catch(...)
      {
        if(!m_slic->errors().isEmpty())
        {
          m_errors << m_slic->errors();
        }
        else
        {
          m_errors << tr("Unspecified error when executing SLIC");
        }

        abort();
        return;
      }
    }
    else
    {
      paddedStack = create_itkImage<itkVolumeType>(paddedBounds, SEG_BG_VALUE, spacing, origin);
    }

    Q_ASSERT(intersect(paddedBounds, stackBounds));
    auto common = intersection(paddedBounds, stackBounds);

    copy_image<itkVolumeType>(stackImage, paddedStack, common);

    Q_ASSERT(paddedMiddleRegion.IsInside(sourceRegion) && paddedMiddleRegion.IsInside(targetRegion));

    long int sliceSize = 1;
    for(auto i: {0,1,2}) { if(dirIndex == i) continue; sliceSize *= paddedMiddleRegion.GetSize(i); };

    RegionType sliceRegion{paddedMiddleRegion};
    sliceRegion.SetIndex(dirIndex, 0);
    sliceRegion.SetSize(dirIndex, 1);

    auto stackSlice = itkVolumeType::New();
    stackSlice->SetRegions(sliceRegion);
    stackSlice->SetOrigin(stackImage->GetOrigin());
    stackSlice->SetSpacing(stackImage->GetSpacing());
    stackSlice->Allocate();

    auto segSlice = itkVolumeType::New();
    segSlice->SetRegions(sliceRegion);
    segSlice->SetOrigin(stackImage->GetOrigin());
    segSlice->SetSpacing(stackImage->GetSpacing());
    segSlice->Allocate();

    if (!canExecute()) return;

    // process down
    for(auto dir = sourceRegion.GetIndex(dirIndex) + 1; dir < targetRegion.GetIndex(dirIndex); ++dir)
    {
      reportProgress(5 + ((currentProgress++ * 100)/totalSlices)*0.95);

      auto stackPointer = paddedStack->GetBufferPointer() + ((dir - sourceRegion.GetIndex(dirIndex)) * sliceSize);
      std::memcpy(stackSlice->GetBufferPointer(), stackPointer, sliceSize);

      auto slicePointer = paddedImageDown->GetBufferPointer() + ((dir-1 - sourceRegion.GetIndex(dirIndex)) * sliceSize);
      std::memcpy(segSlice->GetBufferPointer(), slicePointer, sliceSize);

      auto slice  = processSlice(stackSlice, segSlice, direction, original);

      slicePointer = paddedImageDown->GetBufferPointer() + ((dir - sourceRegion.GetIndex(dirIndex)) * sliceSize);
      std::memcpy(slicePointer, slice->GetBufferPointer(), sliceSize);

      if (!canExecute()) return;
    }

    // process up
    for(auto dir = targetRegion.GetIndex(dirIndex) - 1; dir > sourceRegion.GetIndex(dirIndex); --dir)
    {
      reportProgress(5 + ((currentProgress++ * 100)/totalSlices)*0.95);

      auto stackPointer = paddedStack->GetBufferPointer() + ((dir - sourceRegion.GetIndex(dirIndex)) * sliceSize);
      std::memcpy(stackSlice->GetBufferPointer(), stackPointer, sliceSize);

      auto slicePointer = paddedImageUp->GetBufferPointer() + ((dir+1 - sourceRegion.GetIndex(dirIndex)) * sliceSize);
      std::memcpy(segSlice->GetBufferPointer(), slicePointer, sliceSize);

      auto slice  = processSlice(stackSlice, segSlice, direction, original);

      slicePointer = paddedImageUp->GetBufferPointer() + ((dir - sourceRegion.GetIndex(dirIndex)) * sliceSize);
      std::memcpy(slicePointer, slice->GetBufferPointer(), sliceSize);

      if (!canExecute()) return;
    }

    // mix both up and down.
    auto downIt = itk::ImageRegionIteratorWithIndex<itkVolumeType>(paddedImageDown, paddedMiddleRegion);
    auto upIt = itk::ImageRegionConstIteratorWithIndex<itkVolumeType>(paddedImageUp, paddedMiddleRegion);
    downIt.GoToBegin();
    upIt.GoToBegin();

    while(!downIt.IsAtEnd())
    {
      if((downIt.Value() == SEG_VOXEL_VALUE) && (upIt.Value() == SEG_VOXEL_VALUE))
      {
        downIt.Set(1);
      }
      else
      {
        downIt.Set(0);
      }

      ++downIt;
      ++upIt;
    }

    using StructuringElementType = itk::FlatStructuringElement<3>;
    StructuringElementType::RadiusType radius;
    for(int i: {0,1,2}) { radius.SetElement(i, i == dirIndex ? 0: 4); };

    auto ball = StructuringElementType::Ball(radius);

    auto finalDilate = itk::BinaryDilateImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>::New();
    finalDilate->SetInput(paddedImageDown);
    finalDilate->SetKernel(ball);
    finalDilate->SetForegroundValue(1);
    finalDilate->SetBackgroundValue(0);
    finalDilate->SetNumberOfThreads(1);
    finalDilate->Update();

    auto finalErode = itk::BinaryErodeImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>::New();
    finalErode->SetInput(finalDilate->GetOutput());
    finalErode->SetKernel(ball);
    finalErode->SetForegroundValue(1);
    finalErode->SetBackgroundValue(0);
    finalErode->SetNumberOfThreads(1);
    finalErode->Update();

    auto labelmap = itk::BinaryImageToShapeLabelMapFilter<itkVolumeType>::New();
    labelmap->SetInput(finalErode->GetOutput());
    labelmap->SetInputForegroundValue(1);
    labelmap->SetOutputBackgroundValue(0);
    labelmap->SetFullyConnected(true);
    labelmap->SetNumberOfThreads(1);
    labelmap->Update();

    auto labelmapSeg = labelmap->GetOutput();

    if (!canExecute()) return;

    unsigned int max = 0;
    int biggest = -1;
    for(unsigned int i = 1; i < labelmapSeg->GetNumberOfLabelObjects() + 1; ++i)
    {
      auto pixelNum = labelmapSeg->GetLabelObject(i)->GetNumberOfPixels();
      if(pixelNum > max)
      {
        biggest = static_cast<int>(i);
        max = pixelNum;
      }
    }

    if(biggest != -1)
    {
      SpacingType sloSpacing;
      sloSpacing.SetElement(0, spacing[0]);
      sloSpacing.SetElement(1, spacing[1]);
      sloSpacing.SetElement(2, spacing[2]);

      OriginType sloOrigin;
      sloOrigin.SetElement(0, origin[0]);
      sloOrigin.SetElement(1, origin[1]);
      sloOrigin.SetElement(2, origin[2]);

      SLO::Pointer selected = labelmapSeg->GetLabelObject(biggest);
      auto finalSeg = labelObjectToImage(selected, paddedMiddleRegion, sloSpacing, sloOrigin);
      auto minimalB = minimalBounds<itkVolumeType>(finalSeg, SEG_BG_VALUE);
      auto finalRegion = equivalentRegion<itkVolumeType>(finalSeg, minimalB);

      // we need to undo the padding before writing the results to the volume.
      auto extractor = itk::ExtractImageFilter<itkVolumeType, itkVolumeType>::New();
      extractor->SetInput(finalSeg);
      extractor->SetExtractionRegion(finalRegion);
      extractor->UpdateLargestPossibleRegion();

      expandAndDraw<itkVolumeType>(volume, extractor->GetOutput());
    }
 }

  if (!canExecute()) return;

  reportProgress(100);

  if (!m_outputs.contains(0)) m_outputs[0] = std::make_shared<Output>(this, 0, inputSegmentation->output()->spacing());
  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
}

//------------------------------------------------------------------------
itkVolumeType::Pointer SliceInterpolationFilter::processSlice(const itkVolumeType::Pointer stackSlice,
                                                              const itkVolumeType::Pointer segSlice,
                                                              const Axis direction,
                                                              const Histogram &histogram) const
{
  const auto dirIndex = idx(direction);

  using StructuringElementType = itk::FlatStructuringElement<3>;
  StructuringElementType::RadiusType radius;
  for(int i: {0,1,2}) { radius.SetElement(i, i == dirIndex ? 0: 4); };

  auto ball = StructuringElementType::Ball(radius);

  // Erode
  auto binaryErodeFilter = itk::BinaryErodeImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>::New();
  binaryErodeFilter->SetInput(segSlice);
  binaryErodeFilter->SetKernel(ball);
  binaryErodeFilter->SetForegroundValue(SEG_VOXEL_VALUE);
  binaryErodeFilter->SetBackgroundValue(SEG_BG_VALUE);
  binaryErodeFilter->SetNumberOfThreads(1);
  binaryErodeFilter->Update();
  auto erodeImg = binaryErodeFilter->GetOutput();

  // Dilate
  auto binaryDilateFilter = itk::BinaryDilateImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>::New();
  binaryDilateFilter->SetInput(segSlice);
  binaryDilateFilter->SetKernel(ball);
  binaryDilateFilter->SetForegroundValue(SEG_VOXEL_VALUE);
  binaryDilateFilter->SetBackgroundValue(SEG_BG_VALUE);
  binaryDilateFilter->SetNumberOfThreads(1);
  binaryDilateFilter->Update();
  auto dilateImg = binaryDilateFilter->GetOutput();

  auto dilateBuffer = dilateImg->GetBufferPointer();
  auto erodeBuffer  = erodeImg->GetBufferPointer();
  for(unsigned long i = 0; i < dilateImg->GetLargestPossibleRegion().GetNumberOfPixels(); ++i)
  {
    // creates 'frontier' image
    if(erodeBuffer[i] == SEG_VOXEL_VALUE) dilateBuffer[i] = SEG_BG_VALUE;
  }

  auto gaussian  = itk::DiscreteGaussianImageFilter<itkVolumeType, itkVolumeType>::New();
  gaussian->SetInput(stackSlice);
  gaussian->SetVariance(2.0);
  gaussian->SetFilterDimensionality(2);
  gaussian->SetNumberOfThreads(1);
  gaussian->Update();

  const auto sliceSize = segSlice->GetLargestPossibleRegion().GetNumberOfPixels();

  // Resulting image
  auto mask = itkVolumeType::New();
  mask->SetRegions(stackSlice->GetLargestPossibleRegion());
  mask->Allocate();
  std::memcpy(mask->GetBufferPointer(), binaryErodeFilter->GetOutput()->GetBufferPointer(), sliceSize);

  auto maskBuf = mask->GetBufferPointer();
  auto stackImgBuf = gaussian->GetOutput()->GetBufferPointer();

  auto threshold = histogram.threshold(m_threshold);
  int minor = std::max(0, histogram.medianValue()-threshold);
  int major = std::min(255, histogram.medianValue()+threshold);

  auto isValid = [major, minor](const unsigned char value) { return minor <= value && value <= major; };

  dilateBuffer = binaryDilateFilter->GetOutput()->GetBufferPointer();

  for (SizeValueType i = 0; i < sliceSize; ++i)
  {
    if((dilateBuffer[i] == SEG_VOXEL_VALUE) && isValid(stackImgBuf[i]))
    {
        maskBuf[i] = SEG_VOXEL_VALUE;
    }
  }

  auto finalDilate = itk::BinaryDilateImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>::New();
  finalDilate->SetInput(mask);
  finalDilate->SetKernel(ball);
  finalDilate->SetForegroundValue(SEG_VOXEL_VALUE);
  finalDilate->SetBackgroundValue(SEG_BG_VALUE);
  finalDilate->SetNumberOfThreads(1);
  finalDilate->Update();

  auto finalErode = itk::BinaryErodeImageFilter<itkVolumeType, itkVolumeType, StructuringElementType>::New();
  finalErode->SetInput(finalDilate->GetOutput());
  finalErode->SetKernel(ball);
  finalErode->SetForegroundValue(SEG_VOXEL_VALUE);
  finalErode->SetBackgroundValue(SEG_BG_VALUE);
  finalErode->SetNumberOfThreads(1);
  finalErode->Update();

  std::memcpy(mask->GetBufferPointer(), finalErode->GetOutput()->GetBufferPointer(), sliceSize);

  return mask;
}

//------------------------------------------------------------------------
itkVolumeType::Pointer SliceInterpolationFilter::labelObjectToImage(const SLO::Pointer slObject,
                                                                    const RegionType &region,
                                                                    const SpacingType &spacing,
                                                                    const OriginType &origin)
{
  // Creates a temporal map with the label
  auto map = ShapeLabelMap::New();
  map->SetRegions(region);
  map->AddLabelObject(slObject.GetPointer());

  // Converts the map in a image
  auto slmToBiFilter = itk::LabelMapToBinaryImageFilter<ShapeLabelMap, itkVolumeType>::New();
  slmToBiFilter->SetInput(map);
  slmToBiFilter->SetForegroundValue(SEG_VOXEL_VALUE);
  slmToBiFilter->SetBackgroundValue(SEG_BG_VALUE);
  slmToBiFilter->SetNumberOfThreads(1);
  slmToBiFilter->Update();

  auto image = slmToBiFilter->GetOutput();
  image->SetSpacing(spacing);
  image->SetOrigin(origin);

  return itkVolumeType::Pointer(image);
}

//------------------------------------------------------------------------
const QStringList SliceInterpolationFilter::errors() const
{
  return m_errors;
}
