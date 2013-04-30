/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "MorphologicalEditionFilter.h"

#include <Core/Model/EspinaFactory.h>
#include <GUI/Representations/SliceRepresentation.h>

#include <QDebug>

using namespace EspINA;

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId MorphologicalEditionFilter::RADIUS = "Radius";
const QString MorphologicalEditionFilter::INPUTLINK = "Input";

const unsigned int LABEL_VALUE = 255;


//-----------------------------------------------------------------------------
MorphologicalEditionFilter::MorphologicalEditionFilter(NamedInputs inputs,
                                                       Arguments   args,
                                                       FilterType  type)
: SegmentationFilter(inputs, args, type)
, m_ignoreCurrentOutputs(false)
, m_isOutputEmpty(true)
, m_params(m_args)
{
}

//-----------------------------------------------------------------------------
MorphologicalEditionFilter::~MorphologicalEditionFilter()
{
}

//-----------------------------------------------------------------------------
void MorphologicalEditionFilter::createDummyOutput(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
{
  if (SegmentationVolume::TYPE == type)
    createOutput(id, RawSegmentationVolumeSPtr(new RawSegmentationVolume()));
  else
    Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void MorphologicalEditionFilter::createOutputRepresentations(SegmentationOutputSPtr output)
{
  SegmentationVolumeSPtr volumeRep = segmentationVolume(output);
  output->addRepresentation(GraphicalRepresentationSPtr(new SegmentationSliceRepresentation(volumeRep, NULL)));
  //   output->addRepresentation(GraphicalRepresentationSPtr(new VolumeReprentation  (volumeOutput(output))));
  //   output->addRepresentation(GraphicalRepresentationSPtr(new MeshRepresentation  (meshOutput  (output))));
  //   output->addRepresentation(GraphicalRepresentationSPtr(new SmoothRepresentation(meshOutput  (output))));
}

//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::needUpdate(FilterOutputId oId) const
{
  bool update =SegmentationFilter::needUpdate(oId);

  if (!update)
  {
    Q_ASSERT(m_namedInputs.size()  == 1);
    Q_ASSERT(m_outputs.size() == 1);

    SegmentationVolumeSPtr outputVolume = segmentationVolume(m_outputs[0]);
    Q_ASSERT(outputVolume.get());
    Q_ASSERT(outputVolume->toITK().IsNotNull());
    if (!m_inputs.isEmpty())
    {
      Q_ASSERT(m_inputs.size() == 1);
      SegmentationVolumeSPtr inputVolume = segmentationVolume(m_inputs[0]);
      update = outputVolume->timeStamp() < inputVolume->timeStamp();
    }
  }

  return update;
}


//-----------------------------------------------------------------------------
bool MorphologicalEditionFilter::fetchSnapshot(FilterOutputId oId)
{
  if (m_ignoreCurrentOutputs)
    return false;

  return SegmentationFilter::fetchSnapshot(oId);
}
