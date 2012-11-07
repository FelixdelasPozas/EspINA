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
  //NOTE: check stencil pointer
  return m_outputs.size() < 2 || m_outputs[0].IsNull() || m_outputs[1].IsNull();
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

  for(int i=0; i < 2; i++)
  {
    m_outputs[i] = EspinaVolume::New();
    m_outputs[i]->SetRegions(nRegion);
    m_outputs[i]->SetSpacing(spacing);
    m_outputs[i]->Allocate();
    m_outputs[i]->FillBuffer(0);
  }

  ConstIterator it(input, region);
  Iterator     ot1(m_outputs[0], nRegion);
  Iterator     ot2(m_outputs[1], nRegion);

  it .GoToBegin();
  ot1.GoToBegin();
  ot2.GoToBegin();

  for(; !it.IsAtEnd(); ++it, ++ot1, ++ot2)
  {
    EspinaVolume::IndexType index = it.GetIndex();
    if (m_stencil->IsInside(index[0], index[1], index[2]))
      ot1.Set(it.Value());
    else
      ot2.Set(it.Value());
  }

  for (int i = 0; i < 2; i++)
    m_outputs[i] = strechToFiContent(m_outputs[i]);

  emit modified(this);
}

//-----------------------------------------------------------------------------
bool SplitFilter::prefetchFilter()
{
  // TODO 2012-11-07 Save/Restore blocks (like in segmhaImporter)
  for (int i = 0; i < 2; i++)
  {
    QString tmpFile = QString("%1_%2.mhd").arg(id()).arg(i);
    EspinaVolumeReader::Pointer tmpReader = tmpFileReader(tmpFile);
    if (tmpReader.IsNull())
      return false;

    EspinaVolume::Pointer tmpVolume = tmpReader->GetOutput();
    if (tmpVolume.IsNull())
      return false;

    m_outputs[i] = tmpVolume;
  }

  emit modified(this);
  return true;
}
