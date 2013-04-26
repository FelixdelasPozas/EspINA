/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "FillHolesFilter.h"
#include <Core/Model/VolumeOutputType.h>
#include <GUI/Representations/SliceRepresentation.h>

using namespace EspINA;

const QString FillHolesFilter::INPUTLINK = "Input";

//-----------------------------------------------------------------------------
FillHolesFilter::FillHolesFilter(NamedInputs inputs,
                                 Arguments   args,
                                 FilterType  type)
: SegmentationFilter(inputs, args, type)
{
}

//-----------------------------------------------------------------------------
FillHolesFilter::~FillHolesFilter()
{
}

//-----------------------------------------------------------------------------
void FillHolesFilter::createDummyOutput(FilterOutputId id, const FilterOutput::OutputTypeName &type)
{

}

//-----------------------------------------------------------------------------
void FillHolesFilter::createOutputRepresentations(OutputSPtr output)
{
  VolumeOutputTypeSPtr volumeData = outputVolume(output);
  output->addRepresentation(EspinaRepresentationSPtr(new SegmentationSliceRepresentation(volumeData, NULL)));
  //   output->addRepresentation(EspinaRepresentationSPtr(new VolumeReprentation  (volumeOutput(output))));
  //   output->addRepresentation(EspinaRepresentationSPtr(new MeshRepresentation  (meshOutput  (output))));
  //   output->addRepresentation(EspinaRepresentationSPtr(new SmoothRepresentation(meshOutput  (output))));
}

//-----------------------------------------------------------------------------
bool FillHolesFilter::needUpdate(FilterOutputId oId) const
{
  bool update = Filter::needUpdate(oId);

  if (!update)
  {
    Q_ASSERT(m_namedInputs.size()  == 1);
    Q_ASSERT(m_outputs.size() == 1);

    if (!m_inputs.isEmpty())
    {
      Q_ASSERT(m_inputs.size() == 1);
      update = m_outputs[0]->data(VolumeOutputType::TYPE)->timeStamp() < m_inputs[0]->data(VolumeOutputType::TYPE)->timeStamp();
    }
  }

  return update;
}

//-----------------------------------------------------------------------------
void FillHolesFilter::run()
{
  run(0);
}

//-----------------------------------------------------------------------------
void FillHolesFilter::run(FilterOutputId oId)
{
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() == 1);

  SegmentationVolumeTypeSPtr input = outputSegmentationVolume(m_inputs[0]);
  Q_ASSERT(input);

  BinaryFillholeFilter::Pointer filter = BinaryFillholeFilter::New();
  filter->SetInput(input->toITK());
  filter->Update();

  FilterOutput::OutputTypeList dataList;
  dataList << SegmentationVolumeTypeSPtr(new SegmentationVolumeType(filter->GetOutput()));

  createOutput(0, dataList);
}
