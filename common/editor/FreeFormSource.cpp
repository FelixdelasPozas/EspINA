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
#include <vtkSliceView.h>
#include <EspinaCore.h>
#include <EspinaView.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <QDateTime>
#include <QDebug>

const QString FreeFormSource::TYPE = "EditorToolBar::FreeFormSource";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId FreeFormSource::SPACING = ArgumentId("SPACING", true);

//-----------------------------------------------------------------------------
bool drawPixel(int x, int y, int z,
               int cx, int cy, int cz,
               int r, int plane,
               int extent[6])
{
  if (plane == 2)
  {
    double r2 = pow(x-cx+extent[0], 2) + pow(y-cy+extent[2], 2);
    return r2 <= r*r && z == cz-extent[4];
  }else if (plane == 1)
  {
    double r2 = pow(x-cx+extent[0], 2) + pow(z-cz+extent[4], 2);
    return r2 <= r*r && y == cy-extent[2];
  }else if (plane == 0)
  {
    double r2 = pow(y-cy+extent[2], 2) + pow(z-cz+extent[4], 2);
    return r2 <= r*r && x == cx-extent[0];
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
, m_filter(FilterType::New())
{
  Q_ASSERT(inputs.isEmpty());
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{
}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(vtkSliceView::VIEW_PLANE plane,
                          QVector3D center, int r)
{
  bool expandX = vtkSliceView::AXIAL    == plane
              || vtkSliceView::CORONAL  == plane;
  bool expandY = vtkSliceView::AXIAL    == plane
              || vtkSliceView::SAGITTAL == plane;
  bool expandZ = vtkSliceView::SAGITTAL == plane
              || vtkSliceView::CORONAL  == plane;

  int cx = center.x();
  int cy = center.y();
  int cz = center.z();

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

  Extent[0] = std::min(Extent[0], expandX?cx - r:cx);
  Extent[1] = std::max(Extent[1], expandX?cx + r:cx);
  Extent[2] = std::min(Extent[2], expandY?cy - r:cy);
  Extent[3] = std::max(Extent[3], expandY?cy + r:cy);
  Extent[4] = std::min(Extent[4], expandZ?cz - r:cz);
  Extent[5] = std::max(Extent[5], expandZ?cz + r:cz);

  if (memcmp(prevExtent, Extent, 6*sizeof(int)) == 0)
  {
    int minX = (expandX?cx - r:cx)-Extent[0];
    int maxX = (expandX?cx + r:cx)-Extent[0];
    int minY = (expandY?cy - r:cy)-Extent[2];
    int maxY = (expandY?cy + r:cy)-Extent[2];
    int minZ = (expandZ?cz - r:cz)-Extent[4];
    int maxZ = (expandZ?cz + r:cz)-Extent[4];

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
          if (drawPixel(x,y,z,cx,cy,cz,r,plane,Extent))
            outputPtr[offset] = 255;
        }
      }
    }
    m_volume->Modified();
  }else
  {
    EspinaVolume::Pointer img = EspinaVolume::New();
    EspinaVolume::RegionType buffer = region(Extent);
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
          outputPtr[offset] = drawPixel(x,y,z,cx,cy,cz,r,plane,Extent)?255:prevValue;
        }
      }
    }
    m_volume = img;
    m_filter->SetInput(m_volume);
  }
  m_filter->Update();

  m_hasPixels = true;
}

//-----------------------------------------------------------------------------
void FreeFormSource::erase(vtkSliceView::VIEW_PLANE plane,
                           QVector3D center, int r)
{
  if (!m_hasPixels)
    return;

  int cx = center.x();
  int cy = center.y();
  int cz = center.z();

  if (0 <= plane && plane <=2 && r > 0
    && m_volume.IsNotNull()
    && Extent[0] <= cx + r && cx - r <= Extent[1]
    && Extent[2] <= cy + r && cy - r <= Extent[3]
    && Extent[4] <= cz + r && cz - r <= Extent[5]
  )
  {
    DrawExtent[0] = cx - r;
    DrawExtent[1] = cx + r;
    DrawExtent[2] = cy - r;
    DrawExtent[3] = cy + r;
    DrawExtent[4] = cz - r;
    DrawExtent[5] = cz + r;

    bool expandX = vtkSliceView::AXIAL    == plane
                || vtkSliceView::CORONAL  == plane;
    bool expandY = vtkSliceView::AXIAL    == plane
                || vtkSliceView::SAGITTAL == plane;
    bool expandZ = vtkSliceView::SAGITTAL == plane
                || vtkSliceView::CORONAL  == plane;

    int minX = std::max(Extent[0],(expandX?cx - r:cx))-Extent[0];
    int maxX = std::min(Extent[1],(expandX?cx + r:cx))-Extent[0];
    int minY = std::max(Extent[2],(expandY?cy - r:cy))-Extent[2];
    int maxY = std::min(Extent[3],(expandY?cy + r:cy))-Extent[2];
    int minZ = std::max(Extent[4],(expandZ?cz - r:cz))-Extent[4];
    int maxZ = std::min(Extent[5],(expandZ?cz + r:cz))-Extent[4];

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
      if (drawPixel(x,y,z,cx,cy,cz,r,plane,Extent))
        outputPtr[offset] = 0;
    }
      }
    }

    m_volume->Modified();
  }
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
int FreeFormSource::numberOutputs() const
{
  return m_hasPixels?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* FreeFormSource::output(OutputNumber i) const
{
  if (m_volume.IsNotNull() && i == 0)
    return m_filter->GetOutput();

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
    m_filter->SetInput(m_volume);
    emit modified(this);
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
EspinaVolume::RegionType FreeFormSource::region(int extent[6]) const
{
  EspinaVolume::RegionType res;
  for(int i=0; i<3; i++)
  {
    res.SetIndex(i, extent[2*i]);
    res.SetSize (i, extent[2*i+1] - extent[2*i]+1);
  }
  return res;
}
