/*
* Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
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
#include "FillHoles2DFilter.h"
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>

// ITK
#include <itkBinaryFillholeImageFilter.h>
#include <itkPasteImageFilter.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

using BinaryFillholeFilter = itk::BinaryFillholeImageFilter<itkVolumeType>;
using PasteImageFilter = itk::PasteImageFilter <itkVolumeType, itkVolumeType>;

//-----------------------------------------------------------------------------
FillHoles2DFilter::FillHoles2DFilter(InputSList inputs, const Filter::Type &type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler), m_direction(Axis::Z)
{
}

//-----------------------------------------------------------------------------
bool FillHoles2DFilter::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0);
}

//-----------------------------------------------------------------------------
void FillHoles2DFilter::execute()
{
	auto dir = idx(m_direction);

	if (m_inputs.size() != 1)
	{
		auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
		auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

		throw EspinaException(what, details);
	}


	auto input = m_inputs[0];
	auto inputVolume = readLockVolume(input->output());
	if (!inputVolume->isValid())
	{
		auto what = QObject::tr("Invalid input volume");
		auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid input volume");

		throw EspinaException(what, details);
	}

	reportProgress(0);
	if (!canExecute()) return;

	const auto bounds = inputVolume->bounds().bounds();
	auto spacing      = inputVolume->bounds().spacing();
	auto volume       = sparseCopy<itkVolumeType>(inputVolume->itkImage());
	auto sliceBounds  = bounds;

	//Create mask for the filter containing a 3 slices size image filled with SEG_VOXEL_VALUE (255)
	auto maskRegion      = inputVolume->itkImage()->GetLargestPossibleRegion();
	const auto numSlices = maskRegion.GetSize(dir);
	maskRegion.SetSize(dir,3);
	itkVolumeType::Pointer maskImage = itkVolumeType::New();
	maskImage->SetRegions(maskRegion);
	maskImage->Allocate();
	maskImage->FillBuffer(SEG_VOXEL_VALUE);
	auto maskIndex = maskRegion.GetIndex();
	maskIndex.SetElement(dir,maskIndex.GetElement(dir)+1);

	//Variables for progress reporter
	const unsigned int numFilters = 3;
	const auto progressPerFilter  = (double)100/(numFilters*numSlices);
	double progress               = 0;

	PasteImageFilter::Pointer pasteFilter        = nullptr;
	BinaryFillholeFilter::Pointer fillholeFilter = nullptr;
	auto bi0 = dir * 2;
	auto bi1 = bi0 + 1;

	for(auto i = bounds[bi0]; i < bounds[bi1] && canExecute(); i += spacing[dir])
	{
		sliceBounds[bi0] = sliceBounds[bi1] = i;

		auto slice = inputVolume->itkImage(sliceBounds);

		//Copy the current slice over the mask middle slice (like a cheese sandwich)
		pasteFilter = PasteImageFilter::New();
		pasteFilter->SetSourceImage(slice);
		pasteFilter->SetSourceRegion(slice->GetLargestPossibleRegion());
		pasteFilter->SetDestinationImage(maskImage);
		pasteFilter->SetDestinationIndex(maskIndex);
		ITKProgressReporter<PasteImageFilter> pfReporter(this, pasteFilter, progress, progress + progressPerFilter);
		pasteFilter->Update();
		progress += progressPerFilter;

		//Fill the holes of the middle slice (like filling the cheese holes)
		fillholeFilter = BinaryFillholeFilter::New();
		fillholeFilter->SetInput(pasteFilter->GetOutput());
		ITKProgressReporter<BinaryFillholeFilter> fhReporter(this, fillholeFilter, progress, progress + progressPerFilter);
		fillholeFilter->Update();
		progress += progressPerFilter;
		auto fhfOutput = fillholeFilter->GetOutput();

		//Copy back the middle slice to its original place (like taking the cheese out)
		pasteFilter = PasteImageFilter::New();
		pasteFilter->SetSourceImage(fhfOutput);
		auto secondSliceFhfOutputRegion = fhfOutput->GetLargestPossibleRegion();
		secondSliceFhfOutputRegion.SetIndex(dir, 1); //Second slice from the mask
		secondSliceFhfOutputRegion.SetSize(dir, 1); //1 slice of the 3 contained by the mask
		pasteFilter->SetSourceRegion(secondSliceFhfOutputRegion);
		pasteFilter->SetDestinationImage(slice);
		pasteFilter->SetDestinationIndex(slice->GetLargestPossibleRegion().GetIndex());
		ITKProgressReporter<PasteImageFilter> pfReporter2(this, pasteFilter, progress, progress + progressPerFilter);
		pasteFilter->Update();
		progress += progressPerFilter;

		volume->draw(pasteFilter->GetOutput());
	}

	if (!canExecute()) return;
	reportProgress(100);

	if (!m_outputs.contains(0))
	{
		m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
	}

	m_outputs[0]->setData(volume);
	m_outputs[0]->setData(std::make_shared<MarchingCubesMesh>(m_outputs[0].get()));
	m_outputs[0]->setSpacing(spacing);
}

//-----------------------------------------------------------------------------
void FillHoles2DFilter::setDirection(Axis axis)
{
	 m_direction = axis;
}
