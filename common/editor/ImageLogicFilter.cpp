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

#include <EspinaRegions.h>
#include <model/EspinaFactory.h>

#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageAlgorithm.h>

#include <vtkImageAlgorithm.h>

#include <QDebug>

const QString ImageLogicFilter::TYPE = "EditorToolBar::ImageLogicFilter";

typedef itk::ImageRegionConstIteratorWithIndex<EspinaVolume> ConstIterator;
typedef itk::ImageRegionIteratorWithIndex<EspinaVolume> Iterator;

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ImageLogicFilter::OPERATION = "Operation";


//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(Filter::NamedInputs inputs,
                                   ModelItem::Arguments args)
: Filter(inputs, args)
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

  if (!update)
  {
    Q_ASSERT(m_inputs.size()  == 1);
    Q_ASSERT(m_outputs.size() == 1);
    Q_ASSERT(m_outputs[0].volume.IsNotNull());

    itk::TimeStamp inputTimeStamp = m_inputs[0]->GetTimeStamp();
    for (int i = 1; i < m_inputs.size(); i++)
    {
      if (inputTimeStamp < m_inputs[i]->GetTimeStamp())
        inputTimeStamp = m_inputs[i]->GetTimeStamp();
    }

    update = m_outputs[0].volume->GetTimeStamp() < inputTimeStamp;
  }

  return update;
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run() //TODO: Parallelize
{
  Q_ASSERT(m_inputs.size() > 1);

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
  QList<EspinaVolume::RegionType> regions;

  EspinaVolume::SpacingType spacing = m_inputs[0]->GetSpacing();
  EspinaVolume::RegionType br = NormalizedRegion(m_inputs[0]);
  regions << br;
  for (int i = 1; i < m_inputs.size(); i++)
  {
    //Q_ASSERT(spacing == m_inputs[i]->GetSpacing());
    EspinaVolume::RegionType nr = NormalizedRegion(m_inputs[i]);
    br = BoundingBoxRegion(br, nr);
    regions << nr;
  }

  EspinaVolume::Pointer volume = EspinaVolume::New();
  volume->SetRegions(br);
  volume->SetSpacing(spacing);
  volume->Allocate();
  volume->FillBuffer(0);

  for (int i = 0; i < regions.size(); i++)
  {
    ConstIterator it(m_inputs[i], VolumeRegion(m_inputs[i], regions[i]));
    Iterator ot(volume, regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() || ot.Value())
        ot.Set(SEG_VOXEL_VALUE);
    }
  }

  m_outputs << FilterOutput(this, 0, volume);
}

void ImageLogicFilter::substraction()
{
  QList<EspinaVolume *> validInputs;
  QList<EspinaVolume::RegionType> regions;

  EspinaVolume::SpacingType spacing = m_inputs[0]->GetSpacing();
  EspinaVolume::RegionType outputRegion = NormalizedRegion(m_inputs[0]);
  validInputs << m_inputs[0];
  regions     << outputRegion;

  for (int i = 1; i < m_inputs.size(); i++)
  {
//     Q_ASSERT(spacing == m_inputs[i]->GetSpacing());
    EspinaVolume::RegionType nr = NormalizedRegion(m_inputs[i]);
    if (nr.Crop(outputRegion))
    {
      validInputs << m_inputs[i];
      regions     << nr;
    }
  }

  EspinaVolume::Pointer volume = EspinaVolume::New();
  volume->SetRegions(outputRegion);
  volume->SetSpacing(spacing);
  volume->Allocate();
  volume->FillBuffer(0);

  itk::ImageAlgorithm::Copy(m_inputs[0], volume.GetPointer(), m_inputs[0]->GetLargestPossibleRegion(), regions[0]);
  for (int i = 1; i < validInputs.size(); i++)
  {
    ConstIterator it(m_inputs[i], VolumeRegion(m_inputs[i], regions[i]));
    Iterator ot(volume, regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() == SEG_VOXEL_VALUE)
        ot.Set(0);
    }
  }

  m_outputs << FilterOutput(this, 0, volume);
}