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


#include "SplitFilter.h"

#include <Core/EspinaVolume.h>

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkImageStencilData.h>

using namespace EspINA;

const QString SplitFilter::TYPE      = "EditorToolBar::SplitFilter";
const QString SplitFilter::INPUTLINK = "Segmentation";

//-----------------------------------------------------------------------------
SplitFilter::SplitFilter(Filter::NamedInputs inputs, ModelItem::Arguments args)
: SegmentationFilter(inputs, args)
, m_stencil(NULL)
{
}

//-----------------------------------------------------------------------------
SplitFilter::~SplitFilter()
{

}

//-----------------------------------------------------------------------------
QVariant SplitFilter::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
bool SplitFilter::needUpdate() const
{
  return Filter::needUpdate();
}

//-----------------------------------------------------------------------------
void SplitFilter::run()
{
  Q_ASSERT(m_inputs.size() == 1);
  EspinaVolume::Pointer input = m_inputs[0];
  Q_ASSERT(m_stencil.GetPointer());

  EspinaRegion region = input->espinaRegion();
//   itkVolumeType::RegionType  region  = input->GetLargestPossibleRegion();
//   itkVolumeType::RegionType  nRegion = NormalizedRegion(input);
  itkVolumeType::SpacingType spacing = input->toITK()->GetSpacing();

  SegmentationVolume::Pointer volumes[2];
  for(int i=0; i < 2; i++)
    volumes[i] = SegmentationVolume::Pointer(new SegmentationVolume(region, spacing));

  itkVolumeConstIterator it = input      ->iterator(region);
  itkVolumeIterator     ot1 = volumes[0] ->iterator(region);
  itkVolumeIterator     ot2 = volumes[1] ->iterator(region);

  it .GoToBegin();
  ot1.GoToBegin();
  ot2.GoToBegin();

  bool isEmpty1 = true;
  bool isEmpty2 = true;

  for(; !it.IsAtEnd(); ++it, ++ot1, ++ot2)
  {
    itkVolumeType::IndexType index = ot1.GetIndex();
    if (m_stencil->IsInside(index[0], index[1], index[2]))
    {
      ot1.Set(it.Value());
      if (isEmpty1)
        isEmpty1 = ot1.Get() != SEG_VOXEL_VALUE;
    }
    else
    {
      ot2.Set(it.Value());
      if (isEmpty2)
        isEmpty2 = ot2.Get() != SEG_VOXEL_VALUE;
    }
  }

  if (!isEmpty1 && !isEmpty2)
  {
    for (int i = 0; i < 2; i++)
    {
      volumes[i]->strechToFitContent();
      createOutput(i, volumes[i]);
    }

    emit modified(this);
  }
}