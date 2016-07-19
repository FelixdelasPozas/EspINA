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
#include <itkLabelStatisticsImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>//TODO

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

using LabelStatisticsImageFilter = itk::LabelStatisticsImageFilter<itkVolumeType,itkVolumeType>;
using BinaryFillholeFilter = itk::BinaryFillholeImageFilter<itkVolumeType>;//TODO

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

	//inputVolume->bounds().bounds();
	//labelImageFilter  (ITK)

	//TODO
	BinaryFillholeFilter::Pointer filter = BinaryFillholeFilter::New();

	ITKProgressReporter<BinaryFillholeFilter> reporter(this, filter);

	filter->SetInput(inputVolume->itkImage());
	filter->Update();

	reportProgress(100);
	if (!canExecute()) return;

	auto output  = filter->GetOutput();
	auto spacing = input->output()->spacing();

	auto volume = sparseCopy<itkVolumeType>(output);

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
