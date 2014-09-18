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
#include "ImageLogicFilter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Utils/BinaryMask.hxx>

// ITK
#include <itkImageRegionConstIterator.h>

using namespace ESPINA;

//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(InputSList inputs, Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
, m_operation  {Operation::NOSIGN}
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::setOperation(Operation op)
{
  m_operation = op;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate(Output::Id oId) const
{
  return m_outputs.empty() || m_inputs.empty();
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::execute()
{
  execute(0);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::execute(Output::Id oId)
{
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() > 1);

  // NOTE: Updating this filter will result in invalidating previous outputs
  invalidateEditedRegions();
  m_outputs.clear();

  switch (m_operation)
  {
    case Operation::ADDITION:
      addition();
      break;
    case Operation::SUBTRACTION:
      subtraction();
      break;
    default:
      Q_ASSERT(false);
      break;
  };

  emit progress(100);
  if (!canExecute()) return;
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::addition()
{
  auto firstVolume = volumetricData(m_inputs[0]->output());
  Bounds boundingBounds = firstVolume->bounds();

  emit progress(0);
  if (!canExecute()) return;

  for(auto input: m_inputs)
  {
    auto inputVolume = volumetricData(input->output());
    boundingBounds = boundingBox(inputVolume->bounds(), boundingBounds, firstVolume->spacing());
  }

  emit progress(50);
  if (!canExecute()) return;

  auto outputVolume = new SparseVolume<itkVolumeType>{boundingBounds, firstVolume->spacing()};

  for(auto input: m_inputs)
  {
    auto inputImage = volumetricData(input->output())->itkImage();
    auto region = inputImage->GetLargestPossibleRegion();
    auto inputBounds = equivalentBounds<itkVolumeType>(inputImage, region);
    auto mask = BinaryMaskSPtr<unsigned char>{new BinaryMask<unsigned char>{inputBounds, input->output()->spacing()}};
    mask->setForegroundValue(SEG_VOXEL_VALUE);

    BinaryMask<unsigned char>::region_iterator mit(mask.get(), inputBounds);
    itk::ImageRegionConstIterator<itkVolumeType> it(inputImage, region);

    mit.goToBegin();
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      if(it.Get() == SEG_VOXEL_VALUE)
        mit.Set();

      ++it;
      ++mit;
    }

    outputVolume->draw(mask, mask->foregroundValue());
  }

  m_outputs[0] = OutputSPtr{new Output(this, 0)};
  m_outputs[0]->setData(DataSPtr{outputVolume});
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::subtraction()
{
  auto firstVolume = volumetricData(m_inputs[0]->output());
  auto bounds = firstVolume->bounds();
  auto spacing = firstVolume->spacing();

  auto outputVolume = new SparseVolume<itkVolumeType>{bounds, spacing};
  outputVolume->draw(firstVolume->itkImage());

  for(auto i = 1; i < m_inputs.size(); ++i)
  {
    if(intersect(bounds, m_inputs[i]->output()->bounds(), spacing))
    {
      auto intersectionBounds = intersection(m_inputs[0]->output()->bounds(), m_inputs[i]->output()->bounds(), spacing);
      auto inputImage = volumetricData(m_inputs[i]->output())->itkImage(intersectionBounds);
      auto region = inputImage->GetLargestPossibleRegion();
      auto inputBounds = equivalentBounds<itkVolumeType>(inputImage, region);
      auto mask = BinaryMaskSPtr<unsigned char>{new BinaryMask<unsigned char>{inputBounds, m_inputs[i]->output()->spacing()}};
      mask->setForegroundValue(SEG_BG_VALUE);

      BinaryMask<unsigned char>::region_iterator mit(mask.get(), inputBounds);
      itk::ImageRegionConstIterator<itkVolumeType> it(inputImage, region);

      mit.goToBegin();
      it.GoToBegin();

      while(!it.IsAtEnd())
      {
        if(it.Get() == SEG_VOXEL_VALUE)
          mit.Set();

        ++it;
        ++mit;
      }

      outputVolume->draw(mask, mask->foregroundValue());
    }

    emit progress((100/m_inputs.size())*i);
    if (!canExecute()) return;
  }

  m_outputs[0] = OutputSPtr{new Output(this, 0)};
  m_outputs[0]->setData(DataSPtr{outputVolume});
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::restoreState(const State& state)
{
  if(state.compare("Operation=ADDITION"))
    m_operation = Operation::ADDITION;
  else
    if(state.compare("Operation=SUBTRACTION"))
      m_operation = Operation::SUBTRACTION;
    else
      Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
State ImageLogicFilter::state() const
{
  State state;
  if(m_operation == Operation::ADDITION)
    state = State("Operation=ADDITION");
  else
    if(m_operation == Operation::SUBTRACTION)
      state = State("Operation=SUBTRACTION");
    else
      Q_ASSERT(false);

  return state;
}

//-----------------------------------------------------------------------------
Snapshot ImageLogicFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate() const
{
  return m_outputs.empty();
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::ignoreStorageContent() const
{
  return false;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::invalidateEditedRegions()
{
  return false;
}
