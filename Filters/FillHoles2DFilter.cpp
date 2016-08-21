/*
 * FillHoles2DFilter.cpp
 *
 *  Created on: 12 de jul. de 2016
 *      Author: heavy
 */

// ESPINA
#include "FillHoles2DFilter.h"
#include "Utils/ItkProgressReporter.h"
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/Volumetric/SparseVolume.hxx>
#include <Core/Analysis/Data/Mesh/MarchingCubesMesh.h>

// ITK
#include <itkBinaryFillholeImageFilter.h>//TODO
#include <itkPasteImageFilter.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

using BinaryFillholeFilter = itk::BinaryFillholeImageFilter<itkVolumeType>;
using PasteImageFilter = itk::PasteImageFilter <itkVolumeType, itkVolumeType>;

//-----------------------------------------------------------------------------
FillHoles2DFilter::FillHoles2DFilter(InputSList inputs, Filter::Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
{
}

//-----------------------------------------------------------------------------
FillHoles2DFilter::~FillHoles2DFilter()
{
}

//-----------------------------------------------------------------------------
void FillHoles2DFilter::restoreState(const State& state)
{
}

//-----------------------------------------------------------------------------
State FillHoles2DFilter::state() const
{
  return State();
}

//-----------------------------------------------------------------------------
Snapshot FillHoles2DFilter::saveFilterSnapshot() const
{
  return Snapshot();
}

//-----------------------------------------------------------------------------
bool FillHoles2DFilter::needUpdate() const
{
  return m_outputs.isEmpty();
}

//-----------------------------------------------------------------------------
bool FillHoles2DFilter::needUpdate(Output::Id id) const
{
  if (id != 0)
  {
    auto what    = QObject::tr("Invalid output id, id: %1").arg(id);
    auto details = QObject::tr("FillHolesFilter::needUpdate(id) -> Invalid output id, id: %1").arg(id);

    throw EspinaException(what, details);
  }

  return m_outputs.isEmpty() || !validOutput(id);
}

//-----------------------------------------------------------------------------
void ESPINA::FillHoles2DFilter::execute(Output::Id id) {

	Q_ASSERT(0 == id);

	if (m_inputs.size() != 1) {
		auto what = QObject::tr("Invalid number of inputs, number: %1").arg(m_inputs.size());
		auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid number of inputs, number: %1").arg(m_inputs.size());

		throw EspinaException(what, details);
	}

	auto input = m_inputs[0];
	auto inputVolume = readLockVolume(input->output());
	if (!inputVolume->isValid()) {
		auto what = QObject::tr("Invalid input volume");
		auto details = QObject::tr("FillHolesFilter::execute(id) -> Invalid input volume");

		throw EspinaException(what, details);
	}

	reportProgress(0);
	if (!canExecute()) return;

	const auto bounds = inputVolume->bounds().bounds();
	auto spacing = inputVolume->bounds().spacing();
	auto volume = sparseCopy<itkVolumeType>(inputVolume->itkImage());
	auto sliceBounds = bounds;

	//Create mask for the filter containing a 3 slices size image filled with SEG_VOXEL_VALUE (255)
	auto maskRegion = inputVolume->itkImage()->GetLargestPossibleRegion();
	maskRegion.SetSize(2,3);
	itkVolumeType::Pointer maskImage = itkVolumeType::New();
	maskImage->SetRegions(maskRegion);
	maskImage->Allocate();
	maskImage->FillBuffer(SEG_VOXEL_VALUE);

	PasteImageFilter::Pointer pasteFilter;
	BinaryFillholeFilter::Pointer fillholeFilter;
	for(auto i = bounds[4]; i < bounds[5] && canExecute();i += spacing[2])
	{
		sliceBounds[4] = sliceBounds[5] = i;

		auto slice = inputVolume->itkImage(sliceBounds);

		//Copy the current slice over the mask middle slice (like a cheese sandwich)
		pasteFilter = PasteImageFilter::New();
		pasteFilter->SetSourceImage(slice);
		pasteFilter->SetSourceRegion(slice->GetLargestPossibleRegion());
		pasteFilter->SetDestinationImage(maskImage);
		auto maskIndex = maskRegion.GetIndex();
		maskIndex.SetElement(2,maskIndex.GetElement(2)+1);
		pasteFilter->SetDestinationIndex(maskIndex);
		pasteFilter->Update();

		//Fill the holes of the middle slice (like filling the cheese holes)
		fillholeFilter = BinaryFillholeFilter::New();
		fillholeFilter->SetInput(pasteFilter->GetOutput());
		fillholeFilter->Update();
		auto fhfOutput = fillholeFilter->GetOutput();

		//Copy back the middle slice to its original place (like taking the cheese out)
		pasteFilter = PasteImageFilter::New();
		pasteFilter->SetSourceImage(fhfOutput);
		auto secondSliceFhfOutputRegion = fhfOutput->GetLargestPossibleRegion();
		secondSliceFhfOutputRegion.SetIndex(2,1); //Second slice from the mask
		secondSliceFhfOutputRegion.SetSize(2,1); //1 slice of the 3 contained by the mask
		pasteFilter->SetSourceRegion(secondSliceFhfOutputRegion);
		pasteFilter->SetDestinationImage(slice);
		pasteFilter->SetDestinationIndex(slice->GetLargestPossibleRegion().GetIndex());
		pasteFilter->Update();

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
bool FillHoles2DFilter::areEditedRegionsInvalidated()
{
  // TODO
  return false;
}
