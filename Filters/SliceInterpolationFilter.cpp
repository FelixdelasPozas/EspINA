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
//#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/SpatialUtils.hxx>
#include <Core/Utils/BlockTimer.hxx>
#include <GUI/Model/Utils/QueryAdapter.h>

// QT
#include <QQueue>
#include <QDebug>
#include <QList>
#include <QtCore>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//------------------------------------------------------------------------
SliceInterpolationFilter::SliceInterpolationFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler)
    : Filter(inputs, type, scheduler), m_errorMessage("")
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
  BlockTimer<> timer0("Whole filter execution");

  qDebug() << "Number of inputs: " << m_inputs.size();
  if (m_inputs.size() != 2)
  {
    auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
    auto details = QObject::tr("SliceInterpolationFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

    throw EspinaException(what, details);
  }

  auto input = m_inputs[0];
  auto inputVolume = readLockVolume(input->output());

  qDebug() << "Valid readLockVolume: " << inputVolume->isValid();
  if (!inputVolume->isValid())
  {
    auto what = QObject::tr("Invalid input volume");
    auto details = QObject::tr("SliceInterpolationFilter::execute(id) -> Invalid input volume");

    throw EspinaException(what, details);
  }

  // Checking user correct input
  auto spacing = inputVolume->bounds().spacing();
  auto volume = sparseCopy<itkVolumeType>(inputVolume->itkImage());

  auto image = volume->itkImage();

  auto biToSlmFilter = BinaryImageToShapeLabelMapFilter::New();
  biToSlmFilter->SetInput(image);
  biToSlmFilter->SetInputForegroundValue(SEG_VOXEL_VALUE);
  biToSlmFilter->SetFullyConnected(true);
  biToSlmFilter->SetNumberOfThreads(1);
  biToSlmFilter->Update();

  auto slmOutput = biToSlmFilter->GetOutput();

  // Checking correct number of pieces
  const auto slmOutputSize = slmOutput->GetNumberOfLabelObjects();
  if (slmOutputSize < 2)
  {
    qDebug() << "Wrong number of pieces: must be 2 or more";
    m_errorMessage = "Wrong number of pieces: must be 2 or more";
    return;
  }

  // Checking correct direction
  auto auxSize = itkVolumeType::SizeType();
  auxSize.Fill(0); // Ensure 0's initialization
  for (auto slmObj : slmOutput->GetLabelObjects())
    auxSize += slmObj->GetBoundingBox().GetSize();
  auto dir = -1;
  for (int i = 0; i < itkVolumeType::ImageDimension && dir == -1; i++)
    if (auxSize[i] == slmOutputSize)
      dir = i;
  if (dir == -1)
  {
    qDebug() << "Direction error: you should have all the pieces in the same coordinate (with a size of 1)";
    m_errorMessage = tr("Direction error: you should have all the pieces in the same coordinate (with a size of 1)");
    return;
  }
  auto direction = toAxis(dir);

  reportProgress(0);
  if (!canExecute())
    return;

  //Sorting labels by in ascendent order
  QList<SLOSptr> sloList;
  for (auto slmObj : slmOutput->GetLabelObjects())
  {
    sloList << slmObj;
  }
  auto lessThan = [dir] (const SLOSptr a, const SLOSptr b)
  {
    return a->GetBoundingBox().GetIndex(dir) < b->GetBoundingBox().GetIndex(dir);
  };
  std::sort(sloList.begin(), sloList.end(), lessThan);

  // Process for each pair of pieces
  SLOSptr sloSource, sloTarget;
  RegionType sloTargetReg;
  auto stackVolume = readLockVolume(m_inputs.at(1)->output());
  auto maxRegion = equivalentRegion<itkVolumeType>(stackVolume->bounds());
  RegionType currentRegion, srcRegion, tarRegion;
  itkVolumeType::IndexValueType currentSizeOffset;
  itkVolumeType::Pointer stackImg, auxImg, prevMask;
  SizeValueType bufferSize;
  unsigned char* auxImgBuf, *prevMaskBuf, *stackImgBuf;
  ContourInfo cInfo;
  {
    BlockTimer<> timer1("Processing time");
    while (sloList.size() > 1)
    {
      sloSource = sloList.takeFirst();
      sloTarget = sloList.first();

      srcRegion = sloSource->GetBoundingBox();
      tarRegion = sloTarget->GetBoundingBox();

      currentSizeOffset = 2 * (tarRegion.GetIndex(dir) - srcRegion.GetIndex(dir) + 1);
      currentRegion = calculateRoi(maxRegion, srcRegion, tarRegion, direction, currentSizeOffset);
      currentRegion.SetIndex(dir, srcRegion.GetIndex(dir));
      currentRegion.SetSize(dir, 1);

      stackImg = stackVolume->itkImage(equivalentBounds<itkVolumeType>(image, currentRegion));
      bufferSize = currentRegion.GetSize(0) * currentRegion.GetSize(1) * currentRegion.GetSize(2);

      auto sloImg = sloToImage(sloSource, currentRegion);

      cInfo = getContourInfo(stackImg, sloImg, direction, bufferSize);

      while (currentRegion.GetIndex(dir) < tarRegion.GetIndex(dir))
      { //TODO change algorithm
        prevMask = cInfo.getContourMask();
        prevMaskBuf = prevMask->GetBufferPointer();

        currentRegion.SetIndex(dir, currentRegion.GetIndex(dir) + 1);
        stackImg = stackVolume->itkImage(equivalentBounds<itkVolumeType>(image, currentRegion));
        stackImgBuf = stackImg->GetBufferPointer();
        auxImg = itkVolumeType::New();
        auxImg->SetRegions(currentRegion);
        auxImg->Allocate();
        auxImgBuf = auxImg->GetBufferPointer();

        for (SizeValueType i = 0; i < bufferSize; ++i)
        {
          switch (prevMaskBuf[i])
          {
            case 255:
              auxImgBuf[i] = SEG_VOXEL_VALUE;
              break;
            case 2:
              auxImgBuf[i] = (cInfo.getInlandMode() - cInfo.getBeachMode() > cInfo.getInlandMode() - stackImgBuf[i]) ? SEG_VOXEL_VALUE : SEG_BG_VALUE;
              break;
            case 3:
              auxImgBuf[i] = (cInfo.getInlandMode() - cInfo.getCoastMode() > cInfo.getInlandMode() - stackImgBuf[i]) ? SEG_VOXEL_VALUE : SEG_BG_VALUE;
              break;
            default:
              auxImgBuf[i] = SEG_BG_VALUE;
              break;
          }
        }

        expandAndDraw<itkVolumeType>(volume, auxImg, equivalentBounds<itkVolumeType>(auxImg));
      }

    }
  }

  if (!canExecute())
    return;
  reportProgress(100);

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  m_outputs[0]->setData(volume);
  m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
  m_outputs[0]->setSpacing(spacing);
}

//------------------------------------------------------------------------
SliceInterpolationFilter::ContourInfo SliceInterpolationFilter::getContourInfo(const itkVolumeType::Pointer stackImg, const itkVolumeType::Pointer continentImg,
                                                                               const Axis direction, const SizeValueType bufferSize) const
{
  const auto RADIUS = 5; //TODO

  /* inland = erode(continent);
   * beach = continent - inland;
   * dilate = dilate(continent);
   * coast = dilate - sloImage
   * sea = 255 - coast
   */
  StructuringElementType ball;
  ball.SetRadius(RADIUS);
  ball.CreateStructuringElement();

  // Erode
  auto binaryErodeFilter = BinaryErodeFilter::New();
  binaryErodeFilter->SetInput(continentImg);
  binaryErodeFilter->SetKernel(ball);
  binaryErodeFilter->SetNumberOfThreads(1);
  binaryErodeFilter->DebugOn();
  binaryErodeFilter->Update();
  auto inlandImg = binaryErodeFilter->GetOutput();

  // Dilate
  auto binaryDilateFilter = BinaryDilateFilter::New();
  binaryDilateFilter->SetInput(continentImg);
  binaryDilateFilter->SetKernel(ball);
  binaryDilateFilter->SetNumberOfThreads(1);
  binaryDilateFilter->DebugOn();
  binaryDilateFilter->Update();
  auto dilateImg = binaryDilateFilter->GetOutput();

  // Return image
  auto mask = itkVolumeType::New();
  mask->SetRegions(continentImg->GetLargestPossibleRegion());
  mask->Allocate();

  // Buffers
  auto stackImgBuf = stackImg->GetBufferPointer();
  auto continentImgBuf = continentImg->GetBufferPointer();
  auto inlandImgBuf = inlandImg->GetBufferPointer();
  auto dilateImgBuf = dilateImg->GetBufferPointer();
  auto maskBuf = mask->GetBufferPointer();

  // Histograms
  auto inlandHist = Histogram(256);
  auto beachHist = Histogram(256);
  auto coastHist = Histogram(256);
  auto seaHist = Histogram(256);

  // Filling return mask with: inland = 255, beach = 2, coast = 1  and sea = 0
  for (SizeValueType i = 0; i < bufferSize; ++i)
  {
    if (inlandImgBuf[i])
    {
      maskBuf[i] = 255;
      ++inlandHist[stackImgBuf[i]];
    }
    else if (continentImgBuf[i] - inlandImgBuf[i])
    {
      maskBuf[i] = 2;
      ++beachHist[stackImgBuf[i]];
    }
    else if (dilateImgBuf[i])
    {
      maskBuf[i] = 1;
      ++coastHist[stackImgBuf[i]];
    }
    else
    {
      maskBuf[i] = 0;
      ++seaHist[stackImgBuf[i]];
    }
  }

  PixelCounterType inlandMode = 0, beachMode = 0, coastMode = 0, seaMode = 0;
  for (unsigned int i = 0; i < 256; ++i)
  {
    if (inlandHist[i] > inlandHist[inlandMode])
      inlandMode = i;
    if (beachHist[i] > beachHist[beachMode])
      beachMode = i;
    if (coastHist[i] > coastHist[coastMode])
      coastMode = i;
    if (seaHist[i] > seaHist[seaMode])
      seaMode = i;
  }

  return ContourInfo(inlandMode, beachMode, coastMode, seaMode, mask);
}

//------------------------------------------------------------------------
SliceInterpolationFilter::RegionType SliceInterpolationFilter::calculateRoi(const RegionType& maxRegion, const RegionType& srcRegion,
                                                                            const RegionType& tarRegion, const Axis direction, const int extraOffset)
{
  auto dir = idx(direction);

  auto region = RegionType();

  auto dirAux = dir;
  for (unsigned char i = 0; i < 2; ++i)
  {
    dirAux = (dirAux + 1) % 3;
    auto srcOrigin = srcRegion.GetIndex(dirAux);
    auto tarOrigin = tarRegion.GetIndex(dirAux);

    auto minIndex = (srcOrigin < tarOrigin) ? srcOrigin : tarOrigin;
    minIndex -= extraOffset;
    if (minIndex < 0)
      minIndex = 0;

    auto srcEnd = srcOrigin + srcRegion.GetSize(dirAux);
    auto tarEnd = tarOrigin + tarRegion.GetSize(dirAux);
    auto maxSize = (srcEnd > tarEnd) ? srcEnd - minIndex : tarEnd - minIndex;

    auto maxRegionSize = maxRegion.GetSize(dirAux);
    maxSize += 2 * extraOffset;
    if (minIndex + maxSize > maxRegionSize)
      maxSize = maxRegionSize - minIndex;

    region.SetIndex(dirAux, minIndex);
    region.SetSize(dirAux, maxSize);
  }

  return region;
}

//------------------------------------------------------------------------
SliceInterpolationFilter::ContourInfo::ContourInfo()
    : m_inland_mode { 0 }, m_beach_mode { 0 }, m_coast_mode { 0 }, m_sea_mode { 0 }, m_contour_mask { nullptr }
{
}

//------------------------------------------------------------------------
SliceInterpolationFilter::ContourInfo::ContourInfo(PixelCounterType inlandMode, PixelCounterType beachMode, PixelCounterType coastMode,
                                                   PixelCounterType seaMode, itkVolumeType::Pointer contourMask)
    : m_inland_mode { inlandMode }, m_beach_mode { beachMode }, m_coast_mode { coastMode }, m_sea_mode { seaMode }, m_contour_mask { contourMask }
{
}

//------------------------------------------------------------------------
SliceInterpolationFilter::PixelCounterType SliceInterpolationFilter::ContourInfo::getInlandMode() const
{
  return m_inland_mode;
}

//------------------------------------------------------------------------
SliceInterpolationFilter::PixelCounterType SliceInterpolationFilter::ContourInfo::getBeachMode() const
{
  return m_beach_mode;
}

//------------------------------------------------------------------------
SliceInterpolationFilter::PixelCounterType SliceInterpolationFilter::ContourInfo::getCoastMode() const
{
  return m_coast_mode;
}

//------------------------------------------------------------------------
SliceInterpolationFilter::PixelCounterType SliceInterpolationFilter::ContourInfo::getSeaMode() const
{
  return m_sea_mode;
}

//------------------------------------------------------------------------
itkVolumeType::Pointer SliceInterpolationFilter::ContourInfo::getContourMask() const
{
  return m_contour_mask;
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::ContourInfo::setContourMask(const itkVolumeType::Pointer image)
{
  m_contour_mask = image->Clone();
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::ContourInfo::print(std::ostream& os) const
{
  os << "ContourInfo [ " << m_sea_mode << ", " << m_coast_mode << ", " << m_beach_mode << ", " << m_inland_mode << " ]\n";
}

//------------------------------------------------------------------------
itkVolumeType::Pointer SliceInterpolationFilter::sloToImage(const SLOSptr slObject, RegionType region)
{
  // Creates a temporal map with the label
  auto map = ShapeLabelMap::New();
  map->SetRegions(region);
  map->AddLabelObject(slObject.GetPointer());

  // Converts the map in a image
  auto slmToBiFilter = ShapeLabelMapToBinaryImageFilter::New();
  slmToBiFilter->SetInput(map);
  slmToBiFilter->SetForegroundValue(SEG_VOXEL_VALUE);
  slmToBiFilter->SetBackgroundValue(SEG_BG_VALUE);
  slmToBiFilter->SetNumberOfThreads(1);
  slmToBiFilter->Update();

  return itkVolumeType::Pointer(slmToBiFilter->GetOutput());
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::printRegion(const RegionType region) const
{
  std::cout << "A region:" << std::endl << region << " " << std::endl;
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::printImageInZ(const itkVolumeType::Pointer image, const itkVolumeType::OffsetValueType offsetInZ) const
{
  const unsigned int Z = 2;
  auto region = image->GetLargestPossibleRegion();
  region.SetIndex(Z, region.GetIndex(Z) + offsetInZ);
  region.SetSize(Z, 1);
  auto it = itk::ImageRegionIteratorWithIndex<itkVolumeType>(image, region);
  auto index = region.GetIndex();
  std::cout << "---Slice " << region.GetIndex(Z) << "---" << std::endl;
  for (unsigned int i = region.GetIndex(0); i < region.GetIndex(0) + region.GetSize(0); ++i)
  {
    for (unsigned int j = region.GetIndex(1); j < region.GetIndex(1) + region.GetSize(1); ++j)
    {
      index[0] = i;
      index[1] = j;
      it.SetIndex(index);
      switch (it.Value())
      {
        case SEG_VOXEL_VALUE:
          std::cout << "I";
          break;
        case 2:
          std::cout << "P";
          break;
        case 1:
          std::cout << "C";
          break;
        case SEG_BG_VALUE:
          std::cout << "S";
          break;
        default:
          std::cout << "?";
          break;
      }

      if (j == region.GetIndex(1) + region.GetSize(1) - 1)
        std::cout << std::endl;
    }
  }
  std::cout << std::endl << std::endl;

}
