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


#include "FreeFormSource.h"

// EspINA
#include "Core/Model/EspinaFactory.h"

// ITK
#include <itkImageRegionIteratorWithIndex.h>
#include <vtkMath.h>

// Qt
#include <QDebug>

using namespace EspINA;

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId FreeFormSource::SPACING = "SPACING";

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(NamedInputs inputs,
                               Arguments   args,
                               FilterType  type)
: SegmentationFilter(inputs, args, type)
, m_param(m_args)
{
  Q_ASSERT(inputs.isEmpty());
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(FilterOutputId oId,
                          vtkImplicitFunction * brush,
                          const Nm bounds[6],
                          itkVolumeType::PixelType value,
                          bool emitSignal)
{
  Q_ASSERT(0 == oId);

  if (m_outputs.isEmpty())
  {
    FilterOutput::OutputTypeList dataList;
    dataList << SegmentationVolumeTypeSPtr(new SegmentationVolumeType(EspinaRegion(bounds), m_param.spacing()));

    createOutput(0, dataList);
  }

  Filter::draw(oId, brush, bounds, value, emitSignal);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(FilterOutputId oId,
                          itkVolumeType::IndexType index,
                          itkVolumeType::PixelType value,
                          bool emitSignal)
{
  if (m_outputs.isEmpty())
  {
    itkVolumeType::SizeType pixelSize;
    pixelSize.Fill(1);
    itkVolumeType::RegionType pixelRegion(index, pixelSize);
    itkVolumeType::Pointer volume = itkVolumeType::New();
    volume->SetRegions(pixelRegion);
    volume->SetSpacing(m_param.spacing());
    volume->Allocate();
    volume->FillBuffer(0);

    FilterOutput::OutputTypeList dataList;
    dataList << SegmentationVolumeTypeSPtr(new SegmentationVolumeType(volume));

    createOutput(0, dataList);
  }

  Filter::draw(oId, index, value, emitSignal);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(FilterOutputId oId,
                          Nm x, Nm y, Nm z,
                          itkVolumeType::PixelType value,
                          bool emitSignal)
{
  if (m_outputs.isEmpty())
  {
    itkVolumeType::IndexType index;
    index[0] = vtkMath::Round(x / m_param.spacing()[0]);
    index[1] = vtkMath::Round(y / m_param.spacing()[1]);
    index[2] = vtkMath::Round(z / m_param.spacing()[2]);

    itkVolumeType::SizeType pixelSize;
    pixelSize.Fill(1);
    itkVolumeType::RegionType pixelRegion(index, pixelSize);
    itkVolumeType::Pointer volume = itkVolumeType::New();
    volume->SetRegions(pixelRegion);
    volume->SetSpacing(m_param.spacing());
    volume->Allocate();
    volume->FillBuffer(0);

    FilterOutput::OutputTypeList dataList;
    dataList << SegmentationVolumeTypeSPtr(new SegmentationVolumeType(volume));

    createOutput(0, dataList);
  }

  Filter::draw(oId, x, y, z, value, emitSignal);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(FilterOutputId oId,
                          itkVolumeType::Pointer volume,
                          bool emitSignal)
{
  if (m_outputs.isEmpty())
  {
    FilterOutput::OutputTypeList dataList;
    dataList << SegmentationVolumeTypeSPtr(new SegmentationVolumeType(volume));

    createOutput(0, dataList);
  }
  else
    EspINA::Filter::draw(oId, volume, emitSignal);
}


//-----------------------------------------------------------------------------
bool FreeFormSource::needUpdate(FilterOutputId oId) const
{
  return Filter::needUpdate(oId);
}
