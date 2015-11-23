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
#include "Core/Types.h"

using namespace ESPINA;

enum ROI_TYPE { NON_ORTHOGONAL, ORTHOGONAL };

//-----------------------------------------------------------------------------
ROI::ROI(const Bounds &bounds, const NmVector3 &spacing, const NmVector3 &origin)
: SparseVolume{bounds, spacing, origin}
, m_isOrthogonal{true}
{
}

//-----------------------------------------------------------------------------
ROI::ROI(const VolumeBounds &bounds)
: SparseVolume{bounds}
, m_isOrthogonal{true}
{
}

//-----------------------------------------------------------------------------
ROI::ROI(const BinaryMaskSPtr<unsigned char> mask)
: SparseVolume<itkVolumeType>{mask->bounds().bounds(), mask->spacing(), mask->origin()}
, m_isOrthogonal{false}
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
  return m_isOrthogonal;
}

//-----------------------------------------------------------------------------
void ROI::draw(vtkImplicitFunction *brush, const Bounds& bounds, const itkVolumeType::ValueType value)
{
  m_isOrthogonal = false;
  SparseVolume<itkVolumeType>::draw(brush, bounds, value);
}

//-----------------------------------------------------------------------------
void ROI::draw(const BinaryMaskSPtr<unsigned char> mask, const itkVolumeType::ValueType value)
{
  m_isOrthogonal = false;
  SparseVolume<itkVolumeType>::draw(mask, value);
}

//-----------------------------------------------------------------------------
void ROI::draw(const itkVolumeType::Pointer volume)
{
  m_isOrthogonal = false;
  SparseVolume<itkVolumeType>::draw(volume);
}

//-----------------------------------------------------------------------------
void ROI::draw(const typename itkVolumeType::Pointer volume, const Bounds& bounds)
{
  m_isOrthogonal = false;
  SparseVolume<itkVolumeType>::draw(volume, bounds);
}

//-----------------------------------------------------------------------------
void ROI::draw(const Bounds                   &bounds,
               const itkVolumeType::ValueType  value)
{
  m_isOrthogonal = false;
  SparseVolume::draw(bounds, value);
}

//-----------------------------------------------------------------------------
void ROI::draw(const typename itkVolumeType::IndexType &index,
               const itkVolumeType::ValueType           value)
{
  m_isOrthogonal = false;
  SparseVolume::draw(index, value);
}

//-----------------------------------------------------------------------------
ROISPtr ROI::clone() const
{
  auto newROI = std::make_shared<ROI>(bounds());

  if (!isOrthogonal())
  {
    auto image = itkImage();

    newROI->draw(image);
  }

  return newROI;
}

//-----------------------------------------------------------------------------
bool ROI::fetchDataImplementation(TemporalStorageSPtr storage, const QString &path, const QString &id, const VolumeBounds &bounds)
{
  auto volumeBounds = temporalStorageBoundsId(m_path, m_id);

  if (m_storage->exists(volumeBounds))
  {
    m_bounds = deserializeVolumeBounds(m_storage->snapshot(volumeBounds));

    m_isOrthogonal = !SparseVolume<itkVolumeType>::fetchDataImplementation(storage, path, temporalStorageId(id), m_bounds);

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
Snapshot ROI::snapshot(TemporalStorageSPtr storage, const QString &path, const QString &id) const
{
  Snapshot snapshot;

  snapshot << SnapshotData(temporalStorageBoundsId(path, id), serializeVolumeBounds(m_bounds));

  if (!isOrthogonal())
  {
    snapshot << SparseVolume<itkVolumeType>::snapshot(storage, path, temporalStorageId(id));
  }

  return snapshot;
}

//-----------------------------------------------------------------------------
void ESPINA::ROI::applyFixes()
{
  fixVersion2_0_1();
}

//-----------------------------------------------------------------------------
void ESPINA::ROI::fixVersion2_0_1()
{
  // 2.0.1 seg files had "roi" as the id, would not load when part of a sgs filter nowadays.
  if(m_id == "sgs")
  {
    auto volumeBounds = temporalStorageBoundsId(m_path, "roi");
    if(m_storage->exists(volumeBounds))
    {
      auto newVolumeBounds = temporalStorageBoundsId(m_path, m_id);
      m_storage->rename(volumeBounds, newVolumeBounds);

      auto oldNameGenerator = [this](int i) { return QString("roi_%2_%3.mhd").arg(this->type()).arg(i); };
      auto newNameGenerator = [this](int i) { return QString("%1_%2_%3.mhd").arg(m_id).arg(this->type()).arg(i); };
      int i = 0;
      auto name = oldNameGenerator(i);

      while(m_storage->exists(name))
      {
        m_storage->rename(name, newNameGenerator(i));
        name = oldNameGenerator(++i);
      }
    }
  }
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
  if (contains(roi->bounds(), mask->bounds()))
  {
    roi->draw(mask, mask->foregroundValue());
  }
  else
  {
    roi->resize(boundingBox(roi->bounds(), mask->bounds()));
    roi->draw(mask, mask->foregroundValue());
  }
}
