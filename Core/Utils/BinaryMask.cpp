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
#include <Core/Utils/Bounds.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>

// C++
#include <memory>
#include <math.h>
#include <string.h>
#include <cmath>

// itk
#include <itkImageRegion.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.hxx>

#include <QDebug>

namespace EspINA
{
  //-------------------------------------------------------------------------------------
  template<typename T> BinaryMask<T>::BinaryMask(const typename itkImageType::Pointer image, const T &backgroundValue)
  : m_backgroundValue(backgroundValue)
  , m_foregroundValue(SEG_VOXEL_VALUE)
  , m_integerSize(sizeof(int)*8)
  {
    itkRegion region = image->GetLargestPossibleRegion();
    itkIndex index = region.GetIndex();
    m_origin.x = index[0];
    m_origin.y = index[1];
    m_origin.z = index[2];

    itkSpacing spacing = image->GetSpacing();
    m_spacing = NmVector3{ spacing[0], spacing[1], spacing[2]};

    itkSize size = region.GetSize();
    m_size[0] = size[0];
    m_size[1] = size[1];
    m_size[2] = size[2];

    itkVolumeType::Pointer fakeImage = itkVolumeType::New();
    itkVolumeType::SpacingType fakeImageSpacing = image->GetSpacing();
    itkVolumeType::RegionType fakeRegion = region;

    fakeImage->SetRegions(region);
    fakeImage->SetSpacing(fakeImageSpacing);

    Bounds bounds = equivalentBounds<itkVolumeType>(fakeImage, fakeRegion);
    m_bounds = bounds;

    unsigned long long bufferSize = size[0] * size[1] * size[2] / m_integerSize;
    if (0 != (size[0] * size[1] * size[2] % m_integerSize))
      bufferSize++;

    m_image = new int[bufferSize];
    memset(m_image, 0, bufferSize*sizeof(int));

    itkConstIterator iit(image, image->GetLargestPossibleRegion());
    iterator mit(this);

    iit.GoToBegin();
    mit.goToBegin();
    while(!mit.isAtEnd())
    {
      if (iit.Value() != m_backgroundValue)
        mit.Set();

      ++mit;
      ++iit;
    }
  }

  //-------------------------------------------------------------------------------------
  template<typename T> BinaryMask<T>::BinaryMask(const Bounds& bounds, NmVector3 spacing) throw(Invalid_Bounds_Exception)
  : m_bounds{bounds}
  , m_backgroundValue(0)
  , m_foregroundValue(255)
  , m_integerSize(sizeof(int)*8)
  , m_spacing(spacing)
  {
    if (!bounds.areValid())
      throw Invalid_Bounds_Exception();

    double dSpacing[3]{ spacing[0], spacing[1], spacing[2]};
    itkVolumeType::Pointer image = itkVolumeType::New();
    image->SetSpacing(dSpacing);

    itkVolumeType::RegionType region = equivalentRegion<itkVolumeType>(image, bounds);

    m_size[0] = region.GetSize(0);
    m_size[1] = region.GetSize(1);
    m_size[2] = region.GetSize(2);

    m_origin.x = region.GetIndex(0);
    m_origin.y = region.GetIndex(1);
    m_origin.z = region.GetIndex(2);

    long int bufferSize = (m_size[0] * m_size[1] * m_size[2]) / m_integerSize;
    int remainder = (m_size[0] * m_size[1] * m_size[2]) % m_integerSize;
    if (remainder != 0)
      bufferSize++;

    m_image = new int[bufferSize];
    memset(m_image, 0, bufferSize*sizeof(int));
  }

  //-------------------------------------------------------------------------------------
  template<typename T> void BinaryMask<T>::setPixel(const IndexType& index) throw(Out_Of_Bounds_Exception)
  {
    Nm point[3] = { static_cast<Nm>(index.x*m_spacing[0]),
                    static_cast<Nm>(index.y*m_spacing[1]),
                    static_cast<Nm>(index.z*m_spacing[2]) };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw Out_Of_Bounds_Exception();
    }

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_origin.x;
    newIndex.y = index.y - m_origin.y;
    newIndex.z = index.z - m_origin.z;
    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[0]) + (newIndex.z * m_size[0]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    m_image[imageOffset] = m_image[imageOffset] | (1 << valueOffset);
  }

  //-------------------------------------------------------------------------------------
  template<typename T> void BinaryMask<T>::unsetPixel(const IndexType& index) throw(Out_Of_Bounds_Exception)
  {
    Nm point[3] = { static_cast<Nm>(index.x*m_spacing[0]),
                    static_cast<Nm>(index.y*m_spacing[1]),
                    static_cast<Nm>(index.z*m_spacing[2]) };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw Out_Of_Bounds_Exception();
    }

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_origin.x;
    newIndex.y = index.y - m_origin.y;
    newIndex.z = index.z - m_origin.z;
    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[0]) + (newIndex.z * m_size[0]*m_size[1]);
    unsigned long imageOffset = valuePosition / m_integerSize;
    unsigned long valueOffset = valuePosition % m_integerSize;

    m_image[imageOffset] = m_image[imageOffset] & ~(1 << valueOffset);
  }

  //-------------------------------------------------------------------------------------
  template<typename T> typename BinaryMask<T>::PixelType BinaryMask<T>::pixel(const IndexType& index) const throw(Out_Of_Bounds_Exception)
  {
    Nm point[3] = { static_cast<Nm>(index.x*m_spacing[0]),
                    static_cast<Nm>(index.y*m_spacing[1]),
                    static_cast<Nm>(index.z*m_spacing[2]) };
    if (point[0] < m_bounds[0] || point[0] > m_bounds[1] ||
        point[1] < m_bounds[2] || point[1] > m_bounds[3] ||
        point[2] < m_bounds[4] || point[2] > m_bounds[5])
    {
      throw Out_Of_Bounds_Exception();
    }

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_origin.x;
    newIndex.y = index.y - m_origin.y;
    newIndex.z = index.z - m_origin.z;
    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[0]) + (newIndex.z * m_size[0]*m_size[1]);
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
    using ImagePoint = typename itkImageType::PointType;
    using ImageIndex = typename itkImageType::RegionType::IndexType;
    using ImageSize = typename itkImageType::RegionType::SizeType;
    using ImageSpacing = typename itkImageType::SpacingType;
    using ImageIterator = typename itk::ImageRegionIteratorWithIndex<itkImageType>;
    using MaskIterator = typename BinaryMask<T>::const_region_iterator;

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
    spacing[0] = m_spacing[0];
    spacing[1] = m_spacing[1];
    spacing[2] = m_spacing[2];

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

    MaskIterator mit(this, m_bounds);

    ImageIndex imageIndex;
    IndexType maskIndex;


    // TODO: use itk & mask raw buffers to make a fast copy
    for (auto i = 0; i < region.GetNumberOfPixels(); ++i, ++mit)
    {
      maskIndex = mit.getIndex();
      imageIndex[0] = maskIndex.x;
      imageIndex[1] = maskIndex.y;
      imageIndex[2] = maskIndex.z;
      image->SetPixel(imageIndex, mit.Get());
    }

    return image;
  }

  template class BinaryMask<unsigned char>;

};


