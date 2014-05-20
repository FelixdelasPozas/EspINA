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

// EspINA
#include "ROI.h"
#include "Core/EspinaTypes.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ROI::ROI(const Bounds &bounds, const NmVector3 &spacing, const NmVector3 &origin)
  : SparseVolume{bounds, spacing, origin}
  , m_isRectangular{true}
  {
  }

  //-----------------------------------------------------------------------------
  ROI::ROI(const BinaryMaskSPtr<unsigned char> mask, unsigned char value)
  : SparseVolume<itkVolumeType>{mask->bounds().bounds(), mask->spacing(), mask->origin()}
  , m_isRectangular{false}
  {
    this->draw(mask, value);
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
  void ROI::draw(const vtkImplicitFunction* brush, const Bounds& bounds, const unsigned char value)
  {
    m_isRectangular = false;
    SparseVolume<itkVolumeType>::draw(brush, bounds, value);
  }

  //-----------------------------------------------------------------------------
  void ROI::draw(const BinaryMaskSPtr<unsigned char> mask, const unsigned char value)
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
  void ROI::draw(const typename itkVolumeType::IndexType index, const itkVolumeType::PixelType value)
  {
    m_isRectangular = false;
    SparseVolume::draw(index, value);
  }

  //-----------------------------------------------------------------------------
  ROI *ROI::clone() const
  {
    auto newROI = new ROI(bounds(), spacing(), origin());
    auto image = itkImage();

    newROI->draw(image);
    return newROI;
  }

}

