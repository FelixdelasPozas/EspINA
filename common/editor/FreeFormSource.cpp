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
#include "common/EspinaRegions.h"
#include "common/model/EspinaFactory.h"

// ITK
#include <itkImageRegionIteratorWithIndex.h>

// Qt
#include <QDebug>


const QString FreeFormSource::TYPE = "EditorToolBar::FreeFormSource";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId FreeFormSource::SPACING = "SPACING";

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(Filter::NamedInputs inputs,
                               ModelItem::Arguments args)
: Filter(inputs, args)
, m_param(m_args)
{
  Q_ASSERT(inputs.isEmpty());
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(OutputNumber i,
			  vtkImplicitFunction* brush,
			  double bounds[6],
			  EspinaVolume::PixelType value)
{
  Q_ASSERT(0 == i);
  if (m_outputs[i].IsNull())
  {
    EspinaVolume::Pointer img = EspinaVolume::New();
    EspinaVolume::RegionType buffer = BoundsToRegion(bounds, m_param.spacing());
    img->SetRegions(buffer);
    img->SetSpacing(m_param.spacing());
    img->Allocate();
    img->FillBuffer(0);
    m_outputs[i] = img;
  }
  Filter::draw(i, brush, bounds, value);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(OutputNumber i,
			  EspinaVolume::IndexType index,
			  EspinaVolume::PixelType value)
{
  if (m_outputs[i].IsNull())
  {
    EspinaVolume::SizeType pixelSize;
    pixelSize.Fill(1);
    EspinaVolume::RegionType pixelRegion(index, pixelSize);
    EspinaVolume::Pointer img = EspinaVolume::New();
    img->SetRegions(pixelRegion);
    img->SetSpacing(m_param.spacing());
    img->Allocate();
    img->FillBuffer(0);
    m_outputs[0] = img;
  }
  Filter::draw(i, index, value);
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(OutputNumber i,
			  Nm x, Nm y, Nm z,
			  EspinaVolume::PixelType value)
{
  EspinaVolume::IndexType index;
  index[0] = x / m_param.spacing()[0] + 0.5;
  index[1] = y / m_param.spacing()[1] + 0.5;
  index[2] = z / m_param.spacing()[2] + 0.5;
  FreeFormSource::draw(i, index, value);
}

//-----------------------------------------------------------------------------
QVariant FreeFormSource::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
bool FreeFormSource::needUpdate() const
{
  return m_outputs[0].IsNull();
}