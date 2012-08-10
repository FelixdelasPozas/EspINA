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
#include <model/EspinaFactory.h>
#include <EspinaCore.h>
#include <EspinaView.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <QDateTime>
#include <QDebug>

const QString FreeFormSource::TYPE = "EditorToolBar::FreeFormSource";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId FreeFormSource::SPACING = ArgumentId("SPACING", true);

//-----------------------------------------------------------------------------
bool FreeFormSource::drawPixel(int x, int y, int z,
			       int cx, int cy, int cz,
			       int r, int plane)
{
  bool onSlice = false;
  onSlice |= plane == AXIAL    &&  z == cz-Extent[4];
  onSlice |= plane == SAGITTAL &&  x == cx-Extent[0];
  onSlice |= plane == CORONAL  &&  y == cy-Extent[2];

  if (onSlice)
  {
    double p1 = SAGITTAL==plane?
                (y-cy+Extent[2])*m_param.spacing()[1]:
                (x-cx+Extent[0])*m_param.spacing()[0];
    double p2 = AXIAL==plane?
                (y-cy+Extent[2])*m_param.spacing()[1]:
                (z-cz+Extent[4])*m_param.spacing()[2];
    double dist2= pow(p1, 2) + pow(p2, 2);

    return dist2 <= r*r;
  }

  return false;
}

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(Filter::NamedInputs inputs,
                               ModelItem::Arguments args)
: Filter(inputs, args)
, m_param(m_args)
, m_hasPixels(false)
, m_init(false)
, m_volume(NULL)
{
  Q_ASSERT(inputs.isEmpty());
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(PlaneType plane,
                          QVector3D center, Nm radius)
{
  if (plane < 0 || 2 < plane || radius < 0)
    return;

  bool expandX = AXIAL    == plane
              || CORONAL  == plane;
  bool expandY = AXIAL    == plane
              || SAGITTAL == plane;
  bool expandZ = SAGITTAL == plane
              || CORONAL  == plane;

  int cx = center.x() / m_param.spacing()[0] + 0.5;
  int cy = center.y() / m_param.spacing()[1] + 0.5;
  int cz = center.z() / m_param.spacing()[2] + 0.5;

  if (!m_init)
  {
    Extent[0] = Extent[1] = cx;
    Extent[2] = Extent[3] = cy;
    Extent[4] = Extent[5] = cz;
    m_init = true;
  }

  EspinaVolume::SizeType dim;
  int prevExtent[6];
  memcpy(prevExtent, Extent, 6*sizeof(int));

  int rx = round(radius/m_param.spacing()[0]);
  int ry = round(radius/m_param.spacing()[1]);
  int rz = round(radius/m_param.spacing()[2]);

  Extent[0] = std::min(Extent[0], expandX?cx - rx:cx);
  Extent[1] = std::max(Extent[1], expandX?cx + rx:cx);
  Extent[2] = std::min(Extent[2], expandY?cy - ry:cy);
  Extent[3] = std::max(Extent[3], expandY?cy + ry:cy);
  Extent[4] = std::min(Extent[4], expandZ?cz - rz:cz);
  Extent[5] = std::max(Extent[5], expandZ?cz + rz:cz);

  if (m_volume.IsNotNull() && memcmp(prevExtent, Extent, 6*sizeof(int)) == 0)
  {
    int minX = (expandX?cx - rx:cx)-Extent[0];
    int maxX = (expandX?cx + rx:cx)-Extent[0];
    int minY = (expandY?cy - ry:cy)-Extent[2];
    int maxY = (expandY?cy + ry:cy)-Extent[2];
    int minZ = (expandZ?cz - rz:cz)-Extent[4];
    int maxZ = (expandZ?cz + rz:cz)-Extent[4];

    dim = m_volume->GetBufferedRegion().GetSize();
    unsigned char *outputPtr = static_cast<unsigned char *>(m_volume->GetBufferPointer());
    for (int z = minZ; z <= maxZ; z++)
    {
      unsigned long long zOffset = z * dim[0] * dim[1];
      for (int y = minY; y <= maxY; y++)
      {
        unsigned long long yOffset = y * dim[0];
        for (int x = minX; x <= maxX; x++)
        {
          unsigned long long offset = x + yOffset + zOffset;
          if (drawPixel(x,y,z,cx,cy,cz,radius,plane))
            outputPtr[offset] = 255;
        }
      }
    }
    m_volume->Modified();
  }else
  {
    EspinaVolume::Pointer img = EspinaVolume::New();
    EspinaVolume::RegionType buffer = ExtentToRegion(Extent);
    img->SetRegions(buffer);
    img->SetSpacing(m_param.spacing());
    img->Allocate();

    unsigned char *prevOutputPtr = NULL;
    if (!m_volume)
    {
      prevExtent[0] = prevExtent[2] = prevExtent[4] = 0;
      prevExtent[1] = prevExtent[3] = prevExtent[5] = -1;
    } else
    {
      prevOutputPtr = static_cast<unsigned char *>(m_volume->GetBufferPointer());
    }

    int minX = 0;
    int maxX = Extent[1]-Extent[0];
    int minY = 0;
    int maxY = Extent[3]-Extent[2];
    int minZ = 0;
    int maxZ = Extent[5]-Extent[4];

    unsigned char *outputPtr = static_cast<unsigned char *>(img->GetBufferPointer());
    dim = buffer.GetSize();

    for (int z = minZ; z <= maxZ; z++)
    {
      unsigned long long zOffset = z * dim[0] * dim[1];
      for (int y = minY; y <= maxY; y++)
      {
        unsigned long long yOffset = y * dim[0];
        for (int x = minX; x <= maxX; x++)
        {
          unsigned long long offset = x + yOffset + zOffset;
          double prevValue = 0;
          if (prevExtent[0] <= x + Extent[0] && x + Extent[0] <= prevExtent[1]
            && prevExtent[2] <= y + Extent[2] && y + Extent[2] <= prevExtent[3]
            && prevExtent[4] <= z + Extent[4] && z + Extent[4] <= prevExtent[5])
          {
            prevValue = *prevOutputPtr;
            prevOutputPtr++;
          }
          outputPtr[offset] = drawPixel(x,y,z,cx,cy,cz,radius,plane)?255:prevValue;
        }
      }
    }
    m_volume = img;
  }

  emit modified(this);

  m_hasPixels = true;
}

//-----------------------------------------------------------------------------
void FreeFormSource::erase(PlaneType plane,
                           QVector3D center, Nm radius)
{
  if (!m_hasPixels)
    return;

  int cx = center.x() / m_param.spacing()[0] + 0.5;
  int cy = center.y() / m_param.spacing()[1] + 0.5;
  int cz = center.z() / m_param.spacing()[2] + 0.5;

  int rx = round(radius/m_param.spacing()[0]);
  int ry = round(radius/m_param.spacing()[1]);
  int rz = round(radius/m_param.spacing()[2]);

  if (0 <= plane && plane <=2 && radius > 0
    && m_volume.IsNotNull()
    && Extent[0] <= cx + rx && cx - rx <= Extent[1]
    && Extent[2] <= cy + ry && cy - ry <= Extent[3]
    && Extent[4] <= cz + rz && cz - rz <= Extent[5]
  )
  {
    DrawExtent[0] = cx - rx;
    DrawExtent[1] = cx + rx;
    DrawExtent[2] = cy - ry;
    DrawExtent[3] = cy + ry;
    DrawExtent[4] = cz - rz;
    DrawExtent[5] = cz + rz;

    bool expandX = AXIAL    == plane
                || CORONAL  == plane;
    bool expandY = AXIAL    == plane
                || SAGITTAL == plane;
    bool expandZ = SAGITTAL == plane
                || CORONAL  == plane;
    int minX = std::max(Extent[0],(expandX?cx - rx:cx))-Extent[0];
    int maxX = std::min(Extent[1],(expandX?cx + rx:cx))-Extent[0];
    int minY = std::max(Extent[2],(expandY?cy - ry:cy))-Extent[2];
    int maxY = std::min(Extent[3],(expandY?cy + ry:cy))-Extent[2];
    int minZ = std::max(Extent[4],(expandZ?cz - rz:cz))-Extent[4];
    int maxZ = std::min(Extent[5],(expandZ?cz + rz:cz))-Extent[4];

    EspinaVolume::SizeType dim;
    dim = m_volume->GetBufferedRegion().GetSize();

    //NOTE: Try using vtkImageIterator
    unsigned char *outputPtr = static_cast<unsigned char *>(m_volume->GetBufferPointer());

    for (int z = minZ; z <= maxZ; z++)
    {
      unsigned long long zOffset = z * dim[0] * dim[1];
      for (int y = minY; y <= maxY; y++)
      {
    unsigned long long yOffset = y * dim[0];
    for (int x = minX; x <= maxX; x++)
    {
      unsigned long long offset = x + yOffset + zOffset;
      if (drawPixel(x,y,z,cx,cy,cz,radius,plane))
        outputPtr[offset] = 0;
    }
      }
    }
    m_volume->Modified();
  }

  emit modified(this);
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
  return m_volume.IsNull();
}


//-----------------------------------------------------------------------------
int FreeFormSource::numberOutputs() const
{
  return m_hasPixels?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* FreeFormSource::output(OutputNumber i) const
{
  if (m_volume.IsNotNull() && i == 0)
    return m_volume.GetPointer();

  return NULL;
}

//-----------------------------------------------------------------------------
bool FreeFormSource::prefetchFilter()
{
  QString tmpFile = id() + "_0.mhd";
  m_cachedFilter = tmpFileReader(tmpFile);

  if (m_cachedFilter.IsNotNull())
  {
    m_volume = m_cachedFilter->GetOutput();
    m_hasPixels = true;
    emit modified(this);
    return true;
  }

  return false;
}