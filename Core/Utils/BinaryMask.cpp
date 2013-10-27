/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include <itkImageRegionIterator.h>

// qt
#include <QDebug>

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

    qDebug() << "x size:" << m_size[0] << "y size:" << m_size[1] << "z_size:" << m_size[2];
    qDebug() << "integer size:" << m_integerSize << "bits";

    m_origin.x = static_cast<int>(std::floor(m_bounds[0]/m_spacing.x));
    m_origin.y = static_cast<int>(std::floor(m_bounds[2]/m_spacing.y));
    m_origin.z = static_cast<int>(std::floor(m_bounds[4]/m_spacing.z));

    qDebug() << "origin:" << m_origin.x << m_origin.y << m_origin.z;

    long int bufferSize = (m_size[0] * m_size[1] * m_size[2]) / m_integerSize;
    int remainder = (m_size[0] * m_size[1] * m_size[2]) % m_integerSize;
    if (remainder != 0)
      bufferSize++;

    qDebug() << "buffer size" << bufferSize << "bytes - nº virtual voxels:" << bufferSize*m_integerSize << "nº real voxels:" << m_size[0] * m_size[1] * m_size[2];
    qDebug() << "sizeof int" << sizeof(int) << "buffersize*sizeof(int)" << bufferSize*sizeof(int);

    m_image = new int[bufferSize];
    memset(m_image, 0, bufferSize*sizeof(int));
  }

  //-------------------------------------------------------------------------------------
  template<typename T> void BinaryMask<T>::setPixel(const IndexType& index)
  {
    Nm point[3] = { static_cast<Nm>(index.x*m_spacing.x),
                    static_cast<Nm>(index.y*m_spacing.y),
                    static_cast<Nm>(index.z*m_spacing.z) };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw;
    }

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_origin.x;
    newIndex.y = index.y - m_origin.y;
    newIndex.z = index.z - m_origin.z;
    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[1]) + (newIndex.z * m_size[2]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    qDebug() << "SET offset" << m_image[imageOffset] << "bit offset" << valueOffset << "mask" << (1 << valueOffset);
    m_image[imageOffset] = m_image[imageOffset] | (1 << valueOffset);

    qDebug() << "SET buffer pos" << imageOffset << "bit position" << valueOffset;
  }

  //-------------------------------------------------------------------------------------
  template<typename T> void BinaryMask<T>::unsetPixel(const IndexType& index)
  {
    Nm point[3] = { static_cast<Nm>(index.x*m_spacing.x),
                    static_cast<Nm>(index.y*m_spacing.y),
                    static_cast<Nm>(index.z*m_spacing.z) };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw;
    }

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_origin.x;
    newIndex.y = index.y - m_origin.y;
    newIndex.z = index.z - m_origin.z;
    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[1]) + (newIndex.z * m_size[2]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    m_image[imageOffset] = m_image[imageOffset] & ~(1 << valueOffset);

    qDebug() << "UNSET buffer pos" << imageOffset << "bit position" << valueOffset;
  }

  //-------------------------------------------------------------------------------------
  template<typename T> typename BinaryMask<T>::PixelType BinaryMask<T>::pixel(const IndexType& index) const
  {
    Nm point[3] = { static_cast<Nm>(index.x*m_spacing.x),
                    static_cast<Nm>(index.y*m_spacing.y),
                    static_cast<Nm>(index.z*m_spacing.z) };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw;
    }

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_origin.x;
    newIndex.y = index.y - m_origin.y;
    newIndex.z = index.z - m_origin.z;
    unsigned long valuePosition = index.x + (index.y * m_size[1]) + (index.z * m_size[2]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    qDebug() << "PIXEL buffer pos" << imageOffset << "bit position" << valueOffset;
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
    using ImagePoint = typename itkImageType::PointType;
    using ImageIndex = typename itkImageType::RegionType::IndexType;
    using ImageSize = typename itkImageType::RegionType::SizeType;
    using ImageSpacing = typename itkImageType::SpacingType;
    using ImageIterator = typename itk::ImageRegionIterator<itkImageType>;
    using MaskIterator = typename BinaryMask<T>::const_iterator;

    ImageIndex index;
    index[0] = m_origin.x;
    index[1] = m_origin.y;
    index[2] = m_origin.z;
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

    ImagePoint origin;
    origin[0] = m_origin.x;
    origin[1] = m_origin.y;
    origin[2] = m_origin.z;

    ImagePointer image = itkImageType::New();
    image->SetOrigin(origin);
    image->SetRegions(region);
    image->SetSpacing(spacing);
    image->Allocate();
    image->FillBuffer(m_backgroundValue);

    ImageIterator iit(image, region);

    MaskIterator mit(this);

    // TODO: use itk & mask raw buffers to make a fast copy
    for (auto i = 0; i < region.GetNumberOfPixels(); ++i, ++iit, ++mit)
      iit.Set(mit.Get());

    return image;
  }

  template class BinaryMask<unsigned char>;

};


