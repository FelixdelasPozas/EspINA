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

namespace ESPINA
{
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
  bool ROI::isRectangular() const
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
  void ROI::draw(const typename itkVolumeType::IndexType index, const itkVolumeType::ValueType value)
  {
    m_isRectangular = false;
    SparseVolume::draw(index, value);
  }

  //-----------------------------------------------------------------------------
  ROISPtr ROI::clone() const
  {
    ROISPtr newROI{new ROI(bounds(), spacing(), origin())};

    if (!isRectangular())
    {
      auto image = itkImage();

      newROI->draw(image);
    }

    return newROI;
  }

  //-----------------------------------------------------------------------------
  bool contains(ROISPtr roi, NmVector3 point, NmVector3 spacing)
  {
    bool result = contains(roi->bounds(), point, spacing);

    if(result && !roi->isRectangular())
    {
      auto roiPixel = roi->itkImage(Bounds(point));
      result = (SEG_VOXEL_VALUE == *(static_cast<unsigned char*>(roiPixel->GetBufferPointer())));
    }

    return result;
  }


}

