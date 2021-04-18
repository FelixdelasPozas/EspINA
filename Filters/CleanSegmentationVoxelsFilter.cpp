/*
 File: CleanSegmentationVoxelsFilter.cpp
 Created on: 25/07/2019
 Author: Felix de las Pozas Alvarez

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
#include "CleanSegmentationVoxelsFilter.h"
#include <Core/Types.h>
#include <Core/Utils/ITKProgressReporter.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Analysis/Output.h>

// ITK
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkImage.h>


using namespace ESPINA;
using namespace ESPINA::Core::Utils;

//-----------------------------------------------------------------------------
CleanSegmentationVoxelsFilter::CleanSegmentationVoxelsFilter(InputSList inputs, const Filter::Type& type, SchedulerSPtr scheduler)
: Filter{inputs, type, scheduler}
, m_removedVoxelsNum{0}
{
}

//-----------------------------------------------------------------------------
const QStringList CleanSegmentationVoxelsFilter::errors() const
{
  return m_errors;
}

//-----------------------------------------------------------------------------
bool CleanSegmentationVoxelsFilter::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0);
}

//-----------------------------------------------------------------------------
void CleanSegmentationVoxelsFilter::execute()
{
  if (m_inputs.size() != 1)
  {
    auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
    auto details = QObject::tr("CleanSegmentationVoxelsFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

    throw EspinaException(what, details);
  }

  reportProgress(0);
  m_errors.clear();
  m_removedVoxelsNum = 0;

  auto inputSegmentation = m_inputs[0];

  bool validVolume = inputSegmentation->output()->isValid();
  if (!validVolume)
  {
    auto what = QObject::tr("Invalid input volume");
    auto details = QObject::tr("CleanSegmentationVoxelsFilter::execute(id) -> Invalid input volume");

    throw EspinaException(what, details);
  }

  if(!hasVolumetricData(inputSegmentation->output()))
  {
    auto what = QObject::tr("Segmentation doesn't have volumetric data.");
    auto details = QObject::tr("CleanSegmentationVoxelsFilter::execute(id) -> ") + what;

    throw EspinaException(what, details);
  }

  auto volume = sparseCopy<itkVolumeType>(readLockVolume(inputSegmentation->output())->itkImage());

  auto image = volume->itkImage();
  auto region = image->GetLargestPossibleRegion();

  auto biToSlmFilter = itk::BinaryImageToShapeLabelMapFilter<itkVolumeType>::New();
  biToSlmFilter->SetInput(image);
  biToSlmFilter->SetInputForegroundValue(SEG_VOXEL_VALUE);
  biToSlmFilter->SetFullyConnected(false);
  biToSlmFilter->SetComputeFeretDiameter(false);
  biToSlmFilter->SetComputePerimeter(false);
  biToSlmFilter->SetNumberOfThreads(1);

  ITKProgressReporter<itk::BinaryImageToShapeLabelMapFilter<itkVolumeType>> reporter(this, biToSlmFilter, 0, 75);

  try
  {
    biToSlmFilter->Update();

    auto slmOutput = biToSlmFilter->GetOutput();

    unsigned int max = 0;
    unsigned long long maxIndex = 0;

    const auto groupsNumber = slmOutput->GetNumberOfLabelObjects();

    // if only one group the segmentation is already clean.
    if(groupsNumber < 2) return;

    // identify bigger group.
    for(int idx = 0; idx < groupsNumber; ++idx)
    {
      auto group = slmOutput->GetNthLabelObject(idx);
      auto voxelsNum = group->GetNumberOfPixels();

      if(max < voxelsNum)
      {
        max = voxelsNum;
        maxIndex = idx;
      }
    }

    if(maxIndex == -1)
    {
      m_errors << tr("Unable to determine label with maximum number of voxels.");
      return;
    }

    const float P_CONSTANT = 40./groupsNumber;
    for(int idx = 0; idx < groupsNumber; ++idx)
    {
      reportProgress(50 + (idx * P_CONSTANT));
      if(idx == maxIndex) continue;

      auto group = slmOutput->GetNthLabelObject(idx);
      const auto voxelsNum = group->GetNumberOfPixels();
      for(unsigned long long v = 0; v < voxelsNum; ++v)
      {
        volume->draw(group->GetIndex(v), SEG_BG_VALUE);
      }

      m_removedVoxelsNum += voxelsNum;
    }

  }
  catch(itk::ExceptionObject &e)
  {
    m_errors << QString::fromStdString(e.what()) << QString::fromStdString(e.GetLocation());
    return;
  }

  if(m_removedVoxelsNum > 0)
  {
    fitToContents<itkVolumeType>(volume, SEG_BG_VALUE);
    if (!m_outputs.contains(0)) m_outputs[0] = std::make_shared<Output>(this, 0, inputSegmentation->output()->spacing());
    m_outputs[0]->setData(volume);
    m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
  }

  reportProgress(100);
}
