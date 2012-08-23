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
#include <vtkImageAlgorithm.h>

#include <QApplication>
#include <QDebug>
#include <itkImageAlgorithm.h>

const QString ImageLogicFilter::TYPE = "EditorToolBar::ImageLogicFilter";

//-----------------------------------------------------------------------------
typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ImageLogicFilter::OPERATION = "Operation";


//-----------------------------------------------------------------------------
ImageLogicFilter::ImageLogicFilter(Filter::NamedInputs inputs,
                                   ModelItem::Arguments args)
: Filter(inputs, args)
, m_param(m_args)
// , m_pad1(PadFilterType::New())
// , m_pad2(PadFilterType::New())
// , m_orFilter(OrFilterType::New())
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
bool ImageLogicFilter::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_outputs[0] = m_cachedFilter->GetOutput();
    emit modified(this);
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
bool ImageLogicFilter::needUpdate() const
{
  return m_outputs[0].IsNull();
}

//-----------------------------------------------------------------------------
void ImageLogicFilter::run() //TODO: Parallelize
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  Q_ASSERT(m_inputs.size() > 1);

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
  QApplication::restoreOverrideCursor();
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
    Q_ASSERT(spacing == m_inputs[i]->GetSpacing());
    EspinaVolume::RegionType nr = NormalizedRegion(m_inputs[i]);
    br = BoundingBoxRegion(br, nr);
    regions << nr;
  }

  m_outputs[0] = EspinaVolume::New();
  m_outputs[0]->SetRegions(br);
  m_outputs[0]->SetSpacing(spacing);
  m_outputs[0]->Allocate();
  m_outputs[0]->FillBuffer(0);

  for (int i = 0; i < regions.size(); i++)
  {
    itk::ImageRegionConstIteratorWithIndex<EspinaVolume> it(m_inputs[i], regions[i]);
    itk::ImageRegionIteratorWithIndex<EspinaVolume> ot(m_outputs[0], regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      ot.Set(it.Value()+ot.Value());
    }
  }
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
    Q_ASSERT(spacing == m_inputs[i]->GetSpacing());
    EspinaVolume::RegionType nr = NormalizedRegion(m_inputs[i]);
    if (nr.Crop(outputRegion))
    {
      validInputs << m_inputs[i];
      regions     << nr;
    }
  }

  m_outputs[0] = EspinaVolume::New();
  m_outputs[0]->SetRegions(outputRegion);
  m_outputs[0]->SetSpacing(spacing);
  m_outputs[0]->Allocate();
  m_outputs[0]->FillBuffer(0);

  itk::ImageAlgorithm::Copy(m_inputs[0], m_outputs[0].GetPointer(), m_inputs[0]->GetLargestPossibleRegion(), regions[0]);
  for (int i = 1; i < validInputs.size(); i++)
  {
    itk::ImageRegionConstIteratorWithIndex<EspinaVolume> it(m_inputs[i], regions[i]);
    itk::ImageRegionIteratorWithIndex<EspinaVolume> ot(m_outputs[0], regions[i]);
    it.GoToBegin();
    ot.GetRegion();
    for (; !it.IsAtEnd(); ++it,++ot)
    {
      if (it.Value() == SEG_VOXEL_VALUE)
	ot.Set(0);
    }
  }
}
