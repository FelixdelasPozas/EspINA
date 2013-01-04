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
void FreeFormSource::draw(OutputId oId,
                          vtkImplicitFunction * brush,
                          const Nm bounds[6],
                          itkVolumeType::PixelType value)
{
  Q_ASSERT(0 == oId);
  if (m_outputs.isEmpty())
    createOutput(0, EspinaRegion(bounds), m_param.spacing());

  Filter::draw(oId, brush, bounds, value);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(OutputId oId,
                          itkVolumeType::IndexType index,
                          itkVolumeType::PixelType value)
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

    createOutput(0, volume);
  }
  Filter::draw(oId, index, value);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(OutputId oId,
                          Nm x, Nm y, Nm z,
                          itkVolumeType::PixelType value)
{
  if (m_outputs.isEmpty())
  {
    itkVolumeType::IndexType index;
    index[0] = x / m_param.spacing()[0] + 0.5;
    index[1] = y / m_param.spacing()[1] + 0.5;
    index[2] = z / m_param.spacing()[2] + 0.5;

    itkVolumeType::SizeType pixelSize;
    pixelSize.Fill(1);
    itkVolumeType::RegionType pixelRegion(index, pixelSize);
    itkVolumeType::Pointer volume = itkVolumeType::New();
    volume->SetRegions(pixelRegion);
    volume->SetSpacing(m_param.spacing());
    volume->Allocate();
    volume->FillBuffer(0);

    createOutput(0, volume);
  }
  Filter::draw(oId, x, y, z, value);
}

//-----------------------------------------------------------------------------
bool FreeFormSource::needUpdate() const
{
  return Filter::needUpdate();
}