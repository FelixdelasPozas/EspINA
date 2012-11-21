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

#include <EspinaVolume.h>
#include <EspinaRegions.h>

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkImageStencilData.h>

const QString SplitFilter::TYPE      = "EditorToolBar::SplitFilter";
const QString SplitFilter::INPUTLINK = "Segmentation";

typedef itk::ImageRegionIteratorWithIndex<EspinaVolume>      Iterator;
typedef itk::ImageRegionConstIteratorWithIndex<EspinaVolume> ConstIterator;

//-----------------------------------------------------------------------------
SplitFilter::SplitFilter(Filter::NamedInputs inputs, ModelItem::Arguments args)
: Filter(inputs, args)
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
  EspinaVolume *input = m_inputs[0];

  Q_ASSERT(m_stencil.GetPointer());

  EspinaVolume::RegionType  region  = input->GetLargestPossibleRegion();
  EspinaVolume::RegionType  nRegion = NormalizedRegion(input);
  EspinaVolume::SpacingType spacing = input->GetSpacing();

  EspinaVolume::Pointer volumes[2];
  for(int i=0; i < 2; i++)
  {
    volumes[i] = EspinaVolume::New();
    volumes[i]->SetRegions(nRegion);
    volumes[i]->SetSpacing(spacing);
    volumes[i]->Allocate();
    volumes[i]->FillBuffer(0);
  }

  ConstIterator it(input, region);
  Iterator     ot1(volumes[0], nRegion);
  Iterator     ot2(volumes[1], nRegion);

  it .GoToBegin();
  ot1.GoToBegin();
  ot2.GoToBegin();

  bool isEmpty1 = true;
  bool isEmpty2 = true;

  for(; !it.IsAtEnd(); ++it, ++ot1, ++ot2)
  {
    EspinaVolume::IndexType index = ot1.GetIndex();
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

  m_outputs.clear();

  if (!isEmpty1 && !isEmpty2)
  {
    for (int i = 0; i < 2; i++)
      m_outputs << FilterOutput(this, i, strechToFitContent(volumes[i]));

    emit modified(this);
  }
}