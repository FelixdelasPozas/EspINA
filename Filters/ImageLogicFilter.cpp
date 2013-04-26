/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "ImageLogicFilter.h"

#include <Core/Model/EspinaFactory.h>
#include <GUI/Representations/SliceRepresentation.h>

#include <itkImageAlgorithm.h>

#include <QDebug>

using namespace EspINA;


//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ImageLogicFilter::OPERATION = "Operation";


//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(NamedInputs inputs,
                                   Arguments   args,
                                   FilterType  type)
: SegmentationFilter(inputs, args, type)
, m_param(m_args)
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}


//-----------------------------------------------------------------------------
void ImageLogicFilter::createDummyOutput(FilterOutputId id, const FilterOutput::OutputTypeName &type)
{

}

//-----------------------------------------------------------------------------
void ImageLogicFilter::createOutputRepresentations(OutputSPtr output)
{
  VolumeOutputTypeSPtr volumeData = outputVolume(output);
  output->addRepresentation(EspinaRepresentationSPtr(new SegmentationSliceRepresentation(volumeData, NULL)));
  //   output->addRepresentation(EspinaRepresentationSPtr(new VolumeReprentation  (volumeOutput(output))));
  //   output->addRepresentation(EspinaRepresentationSPtr(new MeshRepresentation  (meshOutput  (output))));
  //   output->addRepresentation(EspinaRepresentationSPtr(new SmoothRepresentation(meshOutput  (output))));
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate(FilterOutputId oId) const
{
  bool update = Filter::needUpdate(oId);

  if (!update && !m_inputs.isEmpty()) //TODO 2012-12-10 Check this update
  {
    Q_ASSERT(m_namedInputs.size()  >= 1);
    Q_ASSERT(m_outputs.size() == 1);

    int i = 0;
    while (!update && i < m_inputs.size())
    {
      update = m_outputs[0]->data(VolumeOutputType::TYPE)->timeStamp() < m_inputs[i]->data(VolumeOutputType::TYPE)->timeStamp();
      ++i;
    }
  }

  return update;
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run()
{
  run(0);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run(FilterOutputId oId) //TODO: Parallelize
{
  Q_ASSERT(0 == oId);
  Q_ASSERT(m_inputs.size() > 1);

  // NOTE: Updating this filter will result in invalidating previous outputs
  m_outputs.clear();

  switch (m_param.operation())
  {
    case ADDITION:
      addition();
      break;
    case SUBSTRACTION:
      substraction();
      break;
    default:
      Q_ASSERT(false);
  };

  emit modified(this);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::addition()
{
  QList<EspinaRegion> regions;

  SegmentationVolumeTypeSPtr firstVolume = outputSegmentationVolume(m_inputs[0]);
  EspinaRegion bb = firstVolume->espinaRegion();
  regions << bb;

  for (int i = 1; i < m_inputs.size(); i++)
  {
    SegmentationVolumeTypeSPtr iVolume = outputSegmentationVolume(m_inputs[i]);
    EspinaRegion region = iVolume->espinaRegion();

    bb = BoundingBox(bb, region);
    regions << region;
  }

  itkVolumeType::SpacingType spacing = firstVolume->toITK()->GetSpacing();
  SegmentationVolumeTypeSPtr volume(new SegmentationVolumeType(bb, spacing));

  for (int i = 0; i < regions.size(); i++)
  {
    SegmentationVolumeTypeSPtr  iVolume = outputSegmentationVolume(m_inputs[i]);
    itkVolumeConstIterator it = iVolume->constIterator(regions[i]);
    itkVolumeIterator      ot = volume ->iterator (regions[i]);

    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() || ot.Value())
        ot.Set(SEG_VOXEL_VALUE);
    }
  }

  FilterOutput::OutputTypeList dataList;
  dataList << volume;

  createOutput(0, dataList);

  emit modified(this);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::substraction()
{
  // TODO 2012-11-29 Revisar si se puede evitar crear la imagen
  OutputSList          validInputs;
  QList<EspinaRegion> regions;

  SegmentationVolumeTypeSPtr  firstVolume = outputSegmentationVolume(m_inputs[0]);
  EspinaRegion outputRegion = firstVolume->espinaRegion();

  validInputs << m_inputs[0];
  regions     << outputRegion;

  for (int i = 1; i < m_inputs.size(); i++)
  {
    SegmentationVolumeTypeSPtr iVolume = outputSegmentationVolume(m_inputs[i]);
    EspinaRegion region = iVolume->espinaRegion();
    if (outputRegion.intersect(region))
    {
      validInputs << m_inputs[i];
      regions     << outputRegion.intersection(region);
    }
  }

  itkVolumeType::SpacingType spacing = firstVolume->toITK()->GetSpacing();
  SegmentationVolumeTypeSPtr volume(new SegmentationVolumeType(outputRegion, spacing));

  itk::ImageAlgorithm::Copy(firstVolume->toITK().GetPointer(),
                            volume->toITK().GetPointer(),
                            firstVolume->volumeRegion(),
                            volume->volumeRegion());

  for (int i = 1; i < validInputs.size(); i++)
  {
    SegmentationVolumeTypeSPtr  iVolume = outputSegmentationVolume(m_inputs[i]);
    itkVolumeConstIterator it = iVolume->constIterator(regions[i]);
    itkVolumeIterator      ot = volume ->iterator     (regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() == SEG_VOXEL_VALUE)
        ot.Set(0);
    }
  }

  FilterOutput::OutputTypeList dataList;
  dataList << volume;

  createOutput(0, dataList);

  emit modified(this);
}