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
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Utils/SpatialUtils.hxx>
#include <Core/Utils/BlockTimer.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// ITK
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkLabelMapToBinaryImageFilter.h>

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
void SliceInterpolationFilter::execute(){//TODO

  BlockTimer timer0("Whole filter execution");

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

  using BinImgToShapeLabMapFilter = itk::BinaryImageToShapeLabelMapFilter<itkVolumeType>;
  auto biToSlmFilter = BinImgToShapeLabMapFilter::New();
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

  reportProgress(0);
  if (!canExecute())
    return;

  //Sorting labels by in ascendent order
  QList<SLO> sloList;
  for (auto slmObj : slmOutput->GetLabelObjects())
  {
    sloList << slmObj;
  }
  auto lessThan = [dir] (const SLO a, const SLO b)
  {
    return a->GetBoundingBox().GetIndex(dir) < b->GetBoundingBox().GetIndex(dir);
  };
  std::sort(sloList.begin(), sloList.end(), lessThan);

  //Process for each pair of pieces
  SLO sloSource, sloTarget;
  itkVolumeType::RegionType sloTargetReg;
  auto stackVolume = readLockVolume(m_inputs.at(1)->output());
  auto maxRegion = equivalentRegion<itkVolumeType>(stackVolume->bounds());
  itkVolumeType::RegionType currentRegion;
  ContourInfo sloSourceInfo, sloTargetInfo, commonInfo;
  {
    BlockTimer timer1("Processing time");
    while (sloList.size() > 1)
    {
      sloSource = sloList.takeFirst();
      sloTarget = sloList.first();
      sloSourceInfo = getContourInfo(image, stackVolume, sloSource, toAxis(dir));
      sloTargetInfo = getContourInfo(image, stackVolume, sloTarget, toAxis(dir));
      //commonInfo = sloSourceInfo && sloTargetInfo;
      commonInfo = sloSourceInfo;
      currentRegion.SetIndex(dir, sloSource->GetBoundingBox().GetIndex(dir));
      currentRegion.SetSize(dir, sloTarget->GetBoundingBox().GetIndex(dir) - sloSource->GetBoundingBox().GetIndex(dir) + 1);
      auto dirAux = (dir + 1) % 3;
      setMaximumRangeSizeBetween2SLO(currentRegion, maxRegion, sloSource, sloTarget, dirAux);
      dirAux = (dirAux + 1) % 3;
      setMaximumRangeSizeBetween2SLO(currentRegion, maxRegion, sloSource, sloTarget, dirAux);
      printRegion(maxRegion);
      printRegion(image->GetLargestPossibleRegion());
      printRegion(currentRegion);

      // Copy stack slice and assign values in a temp image
      auto tempImage = stackVolume->itkImage(equivalentBounds<itkVolumeType>(image, currentRegion));

      auto buffer = tempImage->GetBufferPointer();
      for (unsigned int i = 0; i < currentRegion.GetSize()[0] * currentRegion.GetSize()[1] * currentRegion.GetSize()[2]; ++i)
        buffer[i] = (commonInfo.inHistogramRange(buffer[i])) ? SEG_VOXEL_VALUE : SEG_BG_VALUE;
      //buffer[i] = (commonInfo.inRange(buffer[i])) ? SEG_VOXEL_VALUE : SEG_BG_VALUE;

      // Copy the top piece
      for (unsigned int pID = 0; pID < sloSource->Size(); pID++)
        tempImage->SetPixel(sloSource->GetIndex(pID), SEG_VOXEL_VALUE);

      // Copy the bottom piece
      for (unsigned int pID = 0; pID < sloTarget->Size(); pID++)
        tempImage->SetPixel(sloTarget->GetIndex(pID), SEG_VOXEL_VALUE);

      // Get connected labels
      auto biToSlmFilter2 = BinImgToShapeLabMapFilter::New();
      biToSlmFilter2->SetInput(tempImage);
      biToSlmFilter2->SetInputForegroundValue(SEG_VOXEL_VALUE);
      biToSlmFilter2->SetFullyConnected(true);
      biToSlmFilter2->SetNumberOfThreads(1);
      biToSlmFilter2->Update();

      auto outputLabelmap = biToSlmFilter2->GetOutput();

      // Discard the labels that don't have the whole size in the calculated direction
      QList<int> toRemove;
      SLO sloOutput;
      for (auto obj : biToSlmFilter2->GetOutput()->GetLabelObjects())
        if (obj->GetBoundingBox().GetSize(dir) == currentRegion.GetSize(dir) && obj->HasIndex(sloSource->GetIndex(0)) && obj->HasIndex(sloTarget->GetIndex(0)))
          sloOutput = obj;
        else
          toRemove << obj->GetLabel();

      for (auto i : toRemove)
        outputLabelmap->RemoveLabel(i);

      if (sloOutput == nullptr)
      {
        qDebug() << "There are not connected between 2 pieces";
        m_errorMessage = tr("There are not connected between 2 pieces");
        return;
      }

      // Creating output image
      using LabMapToBinImgFilter = itk::LabelMapToBinaryImageFilter<BinImgToShapeLabMapFilter::OutputImageType, itkVolumeType>;
      auto lmToBiFilter = LabMapToBinImgFilter::New();
      lmToBiFilter->SetInput(outputLabelmap);
      lmToBiFilter->ReleaseDataFlagOn();
      lmToBiFilter->SetNumberOfThreads(1);
      lmToBiFilter->SetBackgroundValue(SEG_BG_VALUE);
      lmToBiFilter->SetForegroundValue(SEG_VOXEL_VALUE);
      lmToBiFilter->Update();

      auto outputImg = lmToBiFilter->GetOutput();

      expandAndDraw<itkVolumeType>(volume, outputImg, equivalentBounds<itkVolumeType>(outputImg));
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
SliceInterpolationFilter::ContourInfo SliceInterpolationFilter::getContourInfo(itkVolumeType::Pointer image, Output::ReadLockData<DefaultVolumetricData> stackVolume, SLO slObject, Axis direction){
  // Obtaining average, minimum and maximum from the label pixels value of the stack
  auto stackImage = stackVolume->itkImage(equivalentBounds<itkVolumeType>(image, slObject->GetBoundingBox()));
  itkVolumeType::IndexType index;
  itkVolumeType::PixelType value;
  itkVolumeType::PixelType max = SEG_BG_VALUE;
  itkVolumeType::PixelType min = SEG_VOXEL_VALUE;
  double average = 0;
  unsigned int borderPX = 0;
  auto histogram = std::make_shared<Histogram>(256);

  for (unsigned int pxId = 0; pxId < slObject->Size(); pxId++)
  {
    index = slObject->GetIndex(pxId);
    if (belongToContour(index, slObject, direction))
    {
      value = stackImage->GetPixel(index);
      ++(*histogram)[value];
      if (value > max)
        max = value;
      if (value < min)
        min = value;
      average += value;
      borderPX++;
    }
  }
  average /= borderPX;

  itkVolumeType::PixelType maxHist = SEG_BG_VALUE;
  itkVolumeType::PixelType minHist = SEG_VOXEL_VALUE;
  QList<unsigned char> posList;
  for(unsigned int i = 0; i < histogram->size(); ++i)
    if ((*histogram)[i] != 0) posList << i;

  std::sort(posList.begin(), posList.end(),
      [histogram](unsigned char a,unsigned char b) {return (*histogram)[a] > (*histogram)[b];});

  unsigned char pos;
  unsigned long minHistOcurrences;
  for(auto i = 0; i < 30 && i < posList.size(); ++i){
    pos = posList.at(i);
    minHistOcurrences = (*histogram)[pos];
    if (pos > maxHist)
      maxHist = pos;
    if (pos < minHist)
      minHist = pos;
  }

  return ContourInfo(maxHist, minHist, 2.0, histogram, minHistOcurrences);

}

//------------------------------------------------------------------------
bool SliceInterpolationFilter::belongToContour(itkVolumeType::IndexType index, SLO slObject, Axis direction){
  bool result = false;
  QList<itkVolumeType::OffsetType> offsetList;
  if(direction != Axis::X) offsetList << itkVolumeType::OffsetType({1, 0, 0}) << itkVolumeType::OffsetType({-1, 0, 0});
  if(direction != Axis::Y) offsetList << itkVolumeType::OffsetType({0, 1, 0}) << itkVolumeType::OffsetType({0, -1, 0});
  if(direction != Axis::Z) offsetList << itkVolumeType::OffsetType({0, 0, 1}) << itkVolumeType::OffsetType({0, 0, -1});
  for (auto offset : offsetList)
  {
    result = !slObject->HasIndex(index + offset);
    if (result) break;
  }

  return result;
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::setMaximumRangeSizeBetween2SLO(itkVolumeType::RegionType& region, const itkVolumeType::RegionType& maxRegion, const SLO sloSrc, const SLO sloTar, const int direction, const int extraOffset)
{
  auto srcOrigin = sloSrc->GetBoundingBox().GetIndex(direction);
  auto tarOrigin = sloTar->GetBoundingBox().GetIndex(direction);
  auto minIndex = (srcOrigin < tarOrigin) ? srcOrigin : tarOrigin;
  if (minIndex < 0) minIndex = 0;
  auto srcEnd = srcOrigin + sloSrc->GetBoundingBox().GetSize(direction);
  auto tarEnd = tarOrigin + sloTar->GetBoundingBox().GetSize(direction);
  auto maxSize = (srcEnd > tarEnd) ? srcEnd - minIndex : tarEnd - minIndex;
  auto maxRegionSize = maxRegion.GetSize(direction);
  if (minIndex + maxSize > maxRegionSize) maxSize = maxRegionSize - minIndex;

  minIndex -= extraOffset;
  maxSize += 2 * extraOffset;

  region.SetIndex(direction, minIndex);
  region.SetSize(direction, maxSize);
}

//------------------------------------------------------------------------
SliceInterpolationFilter::ContourInfo::ContourInfo()
    : m_max { 0 }
    , m_min { 0 }
    , m_average { 0 }
    , m_histogram { nullptr }
    , m_minHistOcurrences { 0 }
{
}

//------------------------------------------------------------------------
SliceInterpolationFilter::ContourInfo::ContourInfo(itkVolumeType::PixelType max, itkVolumeType::PixelType min, double average, HistogramSptr histogram, unsigned long minHistOcurrences)
    : m_max(max), m_min(min), m_average(average), m_histogram(histogram), m_minHistOcurrences(minHistOcurrences)
{
}

//------------------------------------------------------------------------
const SliceInterpolationFilter::ContourInfo SliceInterpolationFilter::ContourInfo::operator &&(const ContourInfo & other)
{
  itkVolumeType::PixelType max = (m_max > other.max()) ? m_max : other.max();
  itkVolumeType::PixelType min = (m_min < other.min()) ? m_min : other.min();
  double average = (m_average + other.average())/2;

  auto histogram = new Histogram(256);
  for (auto i = 0;i<256;++i)
    (*histogram)[i] = m_histogram->at(i) + other.histogram()->at(i);

  auto mho = m_minHistOcurrences + other.minHistOcurrences();

  return ContourInfo(max, min, average, std::make_shared<Histogram>(histogram), mho);
}

//------------------------------------------------------------------------
const itkVolumeType::PixelType SliceInterpolationFilter::ContourInfo::max() const
{
  return m_max;
}

//------------------------------------------------------------------------
const itkVolumeType::PixelType SliceInterpolationFilter::ContourInfo::min() const
{
  return m_min;
}

//------------------------------------------------------------------------
const double SliceInterpolationFilter::ContourInfo::average() const
{
  return m_average;
}

//------------------------------------------------------------------------
SliceInterpolationFilter::HistogramSptr SliceInterpolationFilter::ContourInfo::histogram() const
{
  return m_histogram;
}

//------------------------------------------------------------------------
unsigned long SliceInterpolationFilter::ContourInfo::minHistOcurrences() const
{
  return m_minHistOcurrences;
}

//------------------------------------------------------------------------
const bool SliceInterpolationFilter::ContourInfo::inRange(const unsigned char value) const
{
  return (value < m_max) && (value > m_min);
}

//------------------------------------------------------------------------
const bool SliceInterpolationFilter::ContourInfo::inHistogramRange(const unsigned char value) const
{
  return (*m_histogram)[value] >= m_minHistOcurrences;
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::ContourInfo::print(std::ostream& os) const
{
  os << "[ min " << static_cast<int>(min()) << " max: " << static_cast<int>(max()) << " med: " << average() << " threshold: " << (average() - static_cast<int>(min())) << "]\n";
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::printRegion(itkVolumeType::RegionType region)
{
  std::cout << "A region:" << std::endl << region << " " << std::endl;
}

//------------------------------------------------------------------------
void SliceInterpolationFilter::printImageInZ(const itkVolumeType::Pointer image, const itkVolumeType::OffsetValueType offsetInZ)
{
  const unsigned int Z = 2;
  auto region = image->GetLargestPossibleRegion();
  region.SetIndex(Z, region.GetIndex(Z) + offsetInZ);
  region.SetSize(Z, 1);
  auto it = itk::ImageRegionIteratorWithIndex<itkVolumeType>(image, region);
  auto index = region.GetIndex();
  std::cout << "---Slice " << region.GetIndex(Z) << "---" << std::endl;
  bool inSlice = false;
  unsigned int number = 0;
  unsigned char histogram[256];
  for (unsigned int i = region.GetIndex(0); i < region.GetIndex(0) + region.GetSize(0); ++i)
  {
    for (unsigned int j = region.GetIndex(1); j < region.GetIndex(1) + region.GetSize(1); ++j)
    {
      index[0] = i;
      index[1] = j;
      it.SetIndex(index);

      switch(it.Value())
      {
        case SEG_VOXEL_VALUE:
          if(!inSlice)
          {
            inSlice = true;
            ++number;
            ++histogram[it.Value()];
          }
          else
          {
            if(number < 10)
              ++number;
          }
          break;
        case SEG_BG_VALUE:
          if(inSlice)
          {
            inSlice = false;
            number = 0;
          }
          break;
        default:
          break;
      }

      std::cout << ((it.Value() == SEG_BG_VALUE) ? '0' : '1');
      if (j == region.GetIndex(1) + region.GetSize(1) - 1)
        std::cout << std::endl;
    }
  }
}
