/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// EspINA
#include "BinaryMask.h"
#include <Core/EspinaTypes.h>

// C++
#include <memory>
#include <math.h>
#include <string.h>
#include <cmath>

// itk
#include <itkImageRegion.h>
#include <itkImageRegionConstIterator.h>

namespace EspINA
{

  //-------------------------------------------------------------------------------------
  template<typename T> BinaryMask<T>::BinaryMask(const Bounds& bounds, Spacing spacing)
  : m_bounds{bounds}
  , m_backgroundValue(0)
  , m_foregroundValue(255)
  , m_integerSize(sizeof(int)*8)
  , m_spacing(spacing)
  {
    if (!bounds.areValid())
      throw;

    m_size[0] = (std::floor(bounds[1]/m_spacing.x) - std::ceil(bounds[0]/m_spacing.x)) + 1;
    m_size[1] = (std::floor(bounds[3]/m_spacing.y) - std::ceil(bounds[2]/m_spacing.y)) + 1;
    m_size[2] = (std::floor(bounds[5]/m_spacing.z) - std::ceil(bounds[4]/m_spacing.z)) + 1;

    m_origin.x = static_cast<int>(std::floor(m_bounds[0]/m_spacing.x));
    m_origin.y = static_cast<int>(std::floor(m_bounds[2]/m_spacing.y));
    m_origin.z = static_cast<int>(std::floor(m_bounds[4]/m_spacing.z));

    long int bufferSize = m_size[0] * m_size[1] * m_size[2];
    m_image = new int[bufferSize];
    memset(m_image, 0, bufferSize*sizeof(int));
  }

  //-------------------------------------------------------------------------------------
  template<typename T> void BinaryMask<T>::setPixel(IndexType& index)
  {
    Nm point[3] = { index.x*m_spacing.x, index.y*m_spacing.y, index.z*m_spacing.z };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw;
    }

    // we must adjust the index
    index.x = index.x - m_origin.x;
    index.y = index.y - m_origin.y;
    index.z = index.z - m_origin.z;
    unsigned long valuePosition = index.x + (index.y * m_size[1]) + (index.z * m_size[2]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    m_image[imageOffset] = m_image[imageOffset] & (1 << valueOffset);
  }

  //-------------------------------------------------------------------------------------
  template<typename T> void BinaryMask<T>::unsetPixel(IndexType& index)
  {
    Nm point[3] = { index.x*m_spacing.x, index.y*m_spacing.y, index.z*m_spacing.z };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw;
    }

    // we must adjust the index
    index.x = index.x - m_origin.x;
    index.y = index.y - m_origin.y;
    index.z = index.z - m_origin.z;
    unsigned long valuePosition = index.x + (index.y * m_size[1]) + (index.z * m_size[2]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    m_image[imageOffset] = m_image[imageOffset] &= ~(1 << valueOffset);;
  }

  //-------------------------------------------------------------------------------------
  template<typename T> typename BinaryMask<T>::PixelType BinaryMask<T>::pixel(IndexType& index) const
  {
    Nm point[3] = { index.x*m_spacing.x, index.y*m_spacing.y, index.z*m_spacing.z };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw;
    }

    // we must adjust the index
    index.x = index.x - m_origin.x;
    index.y = index.y - m_origin.y;
    index.z = index.z - m_origin.z;
    unsigned long valuePosition = index.x + (index.y * m_size[1]) + (index.z * m_size[2]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    bool set = ((m_image[imageOffset] & (1 << valueOffset)) == (1 << valueOffset));
    if (set)
      return m_foregroundValue;

    return m_backgroundValue;
  }

  //-------------------------------------------------------------------------------------
  template<typename T> typename BinaryMask<T>::itkImageType::Pointer BinaryMask<T>::itkImage() const
  {
    using ImagePointer = typename itkImageType::Pointer;
    using ImageRegion = typename itkImageType::RegionType;
    using ImageIndex = typename itkImageType::RegionType::IndexType;
    using ImageSize = typename itkImageType::RegionType::SizeType;
    using ImageSpacing = typename itkImageType::SpacingType;
    using ImageIterator = typename itk::ImageRegionConstIterator<itkImageType>;
    using MaskIterator = typename BinaryMask<T>::const_iterator;

    ImageIndex index;
    index[0] = m_origin[0];
    index[1] = m_origin[1];
    index[2] = m_origin[2];
    ImageSize size;
    size[0] = m_size[0];
    size[1] = m_size[1];
    size[2] = m_size[2];
    ImageRegion region;
    region.SetIndex(index);
    region.SetSize(size);
    ImageSpacing spacing;
    spacing[0] = m_spacing.x;
    spacing[1] = m_spacing.y;
    spacing[2] = m_spacing.z;

    ImagePointer image = itkImageType::New();
    image->SetOrigin(index);
    image->SetRegions(region);
    image->SetSpacing(spacing);
    image->Allocate();
    image->FillBuffer(m_backgroundValue);

    ImageIterator iit(image, region);
    MaskIterator mit(&this, m_bounds);

    // TODO: use itk & mask raw buffers to make a fast copy
    for (iit.GoToBegin(); iit != iit.End(); ++iit, ++mit)
      *iit = *mit;

    return image;
  }

};


