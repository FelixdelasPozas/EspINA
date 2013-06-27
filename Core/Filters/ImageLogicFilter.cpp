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
#include <Core/Model/MarchingCubesMesh.h>
#include <Core/OutputRepresentations/RawVolume.h>
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
: BasicSegmentationFilter(inputs, args, type)
, m_param(m_args)
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate(FilterOutputId oId) const
{
  bool update =SegmentationFilter::needUpdate(oId);

  if (!update && !m_inputs.isEmpty()) //TODO 2012-12-10 Check this update
  {
    Q_ASSERT(m_namedInputs.size()  >= 1);
    Q_ASSERT(m_outputs.size() == 1);

    SegmentationVolumeSPtr outputVolume = segmentationVolume(m_outputs[0]);
    int i = 1;
    while (!update && i < m_inputs.size())
    {
      SegmentationVolumeSPtr inputVolume = segmentationVolume(m_inputs[i]);
      update = outputVolume->timeStamp() < inputVolume->timeStamp();
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
    case SUBTRACTION:
      subtraction();
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

  SegmentationVolumeSPtr firstVolume = segmentationVolume(m_inputs[0]);
  EspinaRegion bb = firstVolume->espinaRegion();
  regions << bb;

  for (int i = 1; i < m_inputs.size(); i++)
  {
    SegmentationVolumeSPtr iVolume = segmentationVolume(m_inputs[i]);
    EspinaRegion region = iVolume->espinaRegion();

    bb = BoundingBox(bb, region);
    regions << region;
  }

  itkVolumeType::SpacingType spacing = firstVolume->toITK()->GetSpacing();
  // TODO FIX. WARNING: This is a hack to prevent rounding problems on the limits
  // of the voxel ranges
  for (int i = 0; i < 3; ++i)
  {
    bb[2*i]   += 0.25*spacing[i];
    bb[2*i+1] -= 0.25*spacing[i];
  }
  for(int r = 0; r < regions.size(); ++r)
  {
    for (int i = 0; i < 3; i++)
    {
      regions[r][2*i]   += 0.25*spacing[i];
      regions[r][2*i+1] -= 0.25*spacing[i];
    }
  }
  RawSegmentationVolumeSPtr volumeRepresentation(new RawSegmentationVolume(bb, spacing));

  for (int i = 0; i < regions.size(); i++)
  {
    SegmentationVolumeSPtr iVolume = segmentationVolume(m_inputs[i]);
    itkVolumeConstIterator it      = iVolume->constIterator(regions[i]);
    itkVolumeIterator      ot      = volumeRepresentation->iterator(regions[i]);

    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() || ot.Value())
        ot.Set(SEG_VOXEL_VALUE);
    }
  }

  SegmentationRepresentationSList repList;
  repList << volumeRepresentation;
  repList << MeshRepresentationSPtr(new MarchingCubesMesh(volumeRepresentation));

  addOutputRepresentations(0, repList);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::subtraction()
{
  // TODO 2012-11-29 Revisar si se puede evitar crear la imagen
  OutputSList          validInputs;
  QList<EspinaRegion> regions;

  SegmentationVolumeSPtr firstVolume = segmentationVolume(m_inputs[0]);
  EspinaRegion outputRegion = firstVolume->espinaRegion();

  validInputs << m_inputs[0];
  regions     << outputRegion;

  for (int i = 1; i < m_inputs.size(); i++)
  {
    SegmentationVolumeSPtr iVolume = segmentationVolume(m_inputs[i]);
    EspinaRegion region = iVolume->espinaRegion();
    if (outputRegion.intersect(region))
    {
      validInputs << m_inputs[i];
      regions     << outputRegion.intersection(region);
    }
  }

  itkVolumeType::SpacingType spacing = firstVolume->toITK()->GetSpacing();
  RawSegmentationVolumeSPtr volumeRepresentation(new RawSegmentationVolume(outputRegion, spacing));

  itk::ImageAlgorithm::Copy(firstVolume->toITK().GetPointer(),
                            volumeRepresentation->toITK().GetPointer(),
                            firstVolume->volumeRegion(),
                            volumeRepresentation->volumeRegion());

  for (int i = 1; i < validInputs.size(); i++)
  {
    SegmentationVolumeSPtr iVolume = segmentationVolume(m_inputs[i]);
    itkVolumeConstIterator it      = iVolume->constIterator(regions[i]);
    itkVolumeIterator      ot      = volumeRepresentation ->iterator     (regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() == SEG_VOXEL_VALUE)
        ot.Set(0);
    }
  }

  SegmentationRepresentationSList repList;
  repList << volumeRepresentation;
  repList << MeshRepresentationSPtr(new MarchingCubesMesh(volumeRepresentation));

  addOutputRepresentations(0, repList);
}