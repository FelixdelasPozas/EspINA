/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ROI.h"
#include "Core/EspinaTypes.h"

using namespace ESPINA;

enum ROI_TYPE { NON_ORTHOGONAL, ORTHOGONAL };

//-----------------------------------------------------------------------------
ROI::ROI(const Bounds &bounds, const NmVector3 &spacing, const NmVector3 &origin)
: SparseVolume{bounds, spacing, origin}
, m_isRectangular{true}
{
}

//-----------------------------------------------------------------------------
ROI::ROI(const BinaryMaskSPtr<unsigned char> mask)
: SparseVolume<itkVolumeType>{mask->bounds().bounds(), mask->spacing(), mask->origin()}
, m_isRectangular{false}
{
  this->draw(mask, mask->foregroundValue());
}

//-----------------------------------------------------------------------------
ROI::~ROI()
{
}

//-----------------------------------------------------------------------------
bool ROI::isOrthogonal() const
{
  return m_isRectangular;
}

//-----------------------------------------------------------------------------
void ROI::draw(const vtkImplicitFunction* brush, const Bounds& bounds, const itkVolumeType::ValueType value)
{
  m_isRectangular = false;
  SparseVolume<itkVolumeType>::draw(brush, bounds, value);
}

//-----------------------------------------------------------------------------
void ROI::draw(const BinaryMaskSPtr<unsigned char> mask, const itkVolumeType::ValueType value)
{
  m_isRectangular = false;
  SparseVolume<itkVolumeType>::draw(mask, value);
}

//-----------------------------------------------------------------------------
void ROI::draw(const itkVolumeType::Pointer volume)
{
  m_isRectangular = false;
  SparseVolume<itkVolumeType>::draw(volume);
}

//-----------------------------------------------------------------------------
void ROI::draw(const typename itkVolumeType::Pointer volume, const Bounds& bounds)
{
  m_isRectangular = false;
  SparseVolume<itkVolumeType>::draw(volume, bounds);
}

//-----------------------------------------------------------------------------
void ROI::draw(const typename itkVolumeType::IndexType &index,
               const itkVolumeType::ValueType           value)
{
  m_isRectangular = false;
  SparseVolume::draw(index, value);
}

//-----------------------------------------------------------------------------
ROISPtr ROI::clone() const
{
  ROISPtr newROI{new ROI(bounds(), spacing(), origin())};

  if (!isOrthogonal())
  {
    auto image = itkImage();

    newROI->draw(image);
  }

  return newROI;
}

//-----------------------------------------------------------------------------
bool ROI::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id)
{
  auto volumeBounds = temporalStorageBoundsId(m_path, m_id);

  m_isRectangular = m_storage->exists(volumeBounds);

  if (m_isRectangular)
  {
    m_bounds  = deserializeVolumeBounds(m_storage->snapshot(volumeBounds));
    m_origin  = m_bounds.origin();
    m_spacing = m_bounds.spacing();

    return true;
  }
  else
  {
    return SparseVolume<itkVolumeType>::fetchDataImplementation(storage, path, id);
  }
}

//-----------------------------------------------------------------------------
Snapshot ROI::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
{
  Snapshot snapshot;

  if (isOrthogonal())
  {
    snapshot << SnapshotData(temporalStorageBoundsId(path, id), serializeVolumeBounds(m_bounds));
  }
  else
  {
    snapshot << SparseVolume<itkVolumeType >::snapshot(storage, path, temporalStorageId(id));
  }

  return snapshot;
}

//-----------------------------------------------------------------------------
bool ESPINA::contains(ROIPtr roi, NmVector3 point, NmVector3 spacing)
{
  bool result = contains(roi->bounds(), point, spacing);

  if(result && !roi->isOrthogonal())
  {
    auto roiPixel = roi->itkImage(Bounds(point));
    result = (SEG_VOXEL_VALUE == *(static_cast<unsigned char*>(roiPixel->GetBufferPointer())));
  }

  return result;
}


  //-----------------------------------------------------------------------------
  void ESPINA::expandAndDraw(ROIPtr roi, const BinaryMaskSPtr<unsigned char> mask)
  {
    if(contains(roi->bounds(), mask->bounds().bounds(), roi->spacing()))
    {
      roi->draw(mask, mask->foregroundValue());
    }
    else
    {
      roi->resize(boundingBox(roi->bounds(), mask->bounds().bounds()));
      roi->draw(mask, mask->foregroundValue());
    }
  }
