/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#include "ROI.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ROI::ROI(SparseVolumeSPtr volume)
  {
  }

  //-----------------------------------------------------------------------------
  ROI::ROI(VolumeBounds bounds, NmVector3 spacing, NmVector3 origin)
  {
  }

  //-----------------------------------------------------------------------------
  ROI::~ROI()
  {
  }

  //-----------------------------------------------------------------------------
  size_t ROI::memoryUsage() const
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::setOrigin(const NmVector3& origin)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::setSpacing(const NmVector3& spacing)
  {
  }

  //-----------------------------------------------------------------------------
  const typename itkVolumeType::Pointer ROI::itkImage() const
  {
  }

  //-----------------------------------------------------------------------------
  const typename itkVolumeType::Pointer ROI::itkImage(const Bounds& bounds) const
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::draw(const vtkImplicitFunction* brush, const Bounds& bounds,
      const typename itkVolumeType::ValueType value)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::draw(const BinaryMaskSPtr<typename itkVolumeType::ValueType> mask,
      const typename itkVolumeType::ValueType value)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::draw(const typename itkVolumeType::Pointer volume)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::draw(const typename itkVolumeType::Pointer volume, const Bounds& bounds)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::draw(const typename itkVolumeType::IndexType index, const typename itkVolumeType::PixelType value)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::resize(const Bounds& bounds)
  {
  }

  //-----------------------------------------------------------------------------
  void ROI::undo()
  {
  }

  //-----------------------------------------------------------------------------
  bool ROI::isValid() const
  {
  }

  //-----------------------------------------------------------------------------
  bool ROI::fetchData(const TemporalStorageSPtr storage, const QString& prefix)
  {
  }

  //-----------------------------------------------------------------------------
  Snapshot ROI::snapshot(TemporalStorageSPtr storage, const QString& prefix) const
  {
  }
}
