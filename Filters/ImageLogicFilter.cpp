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

#include <itkImageAlgorithm.h>

#include <QDebug>

using namespace EspINA;

const QString ImageLogicFilter::TYPE = "EditorToolBar::ImageLogicFilter";

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ImageLogicFilter::OPERATION = "Operation";


//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(Filter::NamedInputs inputs,
                                   ModelItem::Arguments args)
: SegmentationFilter(inputs, args)
, m_param(m_args)
{
}

//-----------------------------------------------------------------------------
ImageLogicFilter::~ImageLogicFilter()
{
}

//-----------------------------------------------------------------------------
QVariant ImageLogicFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate() const
{
  bool update = Filter::needUpdate();

  if (!update && !m_inputs.isEmpty()) //TODO 2012-12-10 Check this update
  {
    Q_ASSERT(m_namedInputs.size()  >= 1);
    Q_ASSERT(m_outputs.size() == 1);
    Q_ASSERT(m_outputs[0].volume.get());

    itk::TimeStamp inputTimeStamp = m_inputs[0]->toITK()->GetTimeStamp();
    for (int i = 1; i < m_inputs.size(); i++)
    {
      if (inputTimeStamp < m_inputs[i]->toITK()->GetTimeStamp())
        inputTimeStamp = m_inputs[i]->toITK()->GetTimeStamp();
    }

    update = m_outputs[0].volume->toITK()->GetTimeStamp() < inputTimeStamp;
  }

  return update;
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run() //TODO: Parallelize
{
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

  EspinaRegion bb = m_inputs[0]->espinaRegion();
  regions << bb;

  for (int i = 1; i < m_inputs.size(); i++)
  {
    EspinaRegion region = m_inputs[i]->espinaRegion();

    bb = BoundingBox(bb, region);
    regions << region;
  }

  itkVolumeType::SpacingType spacing = m_inputs[0]->toITK()->GetSpacing();
  SegmentationVolume::Pointer volume(new SegmentationVolume(bb, spacing));

  for (int i = 0; i < regions.size(); i++)
  {
    itkVolumeConstIterator it = m_inputs[i]->constIterator(regions[i]);
    itkVolumeIterator      ot = volume     ->iterator     (regions[i]);

    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() || ot.Value())
        ot.Set(SEG_VOXEL_VALUE);
    }
  }

  m_outputs[0] = Output(this, 0, volume);
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::substraction()
{
  // TODO 2012-11-29 Revisar si se puede evitar crear la imagen
  QList<EspinaVolume::Pointer> validInputs;
  QList<EspinaRegion> regions;

  EspinaRegion outputRegion = m_inputs[0]->espinaRegion();

  validInputs << m_inputs[0];
  regions     << outputRegion;

  for (int i = 1; i < m_inputs.size(); i++)
  {
    EspinaRegion region = m_inputs[i]->espinaRegion();
    if (outputRegion.intersect(region))
    {
      validInputs << m_inputs[i];
      regions     << outputRegion.intersection(region);
    }
  }

  itkVolumeType::SpacingType spacing = m_inputs[0]->toITK()->GetSpacing();
  SegmentationVolume::Pointer volume(new SegmentationVolume(outputRegion, spacing));

  itk::ImageAlgorithm::Copy(m_inputs[0]->toITK().GetPointer(),
                            volume->toITK().GetPointer(),
                            m_inputs[0]->volumeRegion(),
                            volume->volumeRegion());

  for (int i = 1; i < validInputs.size(); i++)
  {
    itkVolumeConstIterator it = m_inputs[i]->constIterator(regions[i]);
    itkVolumeIterator      ot = volume     ->iterator     (regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() == SEG_VOXEL_VALUE)
        ot.Set(0);
    }
  }

  m_outputs[0] = Output(this, 0, volume);
}