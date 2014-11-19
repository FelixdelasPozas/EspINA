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

#ifndef ESPINA_BINARY_MASK_H
#define ESPINA_BINARY_MASK_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/SpatialUtils.hxx>

// ITK
#include <itkImage.h>
#include <itkSmartPointer.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>

// C++
#include <math.h>

// Qt
#include <QByteArray>

namespace ESPINA
{
  template<typename T > class EspinaCore_EXPORT BinaryMask
  {
    public:

      struct IndexType
      {
        IndexType(): x(0), y(0), z(0) {};
        IndexType(const int xv, const int yv, const int zv): x(xv), y(yv), z(zv) {};
        IndexType(const IndexType &i): x(i.x), y(i.y), z(i.z) {};

        int x; int y; int z;
      };

      using PixelType        = T;
      using itkImageType     = itk::Image<T,3>;
      using itkIndex         = typename itkImageType::IndexType;
      using itkRegion        = typename itkImageType::RegionType;
      using itkSpacing       = typename itkImageType::SpacingType;
      using itkSize          = typename itkImageType::SizeType;
      using itkConstIterator = itk::ImageRegionConstIterator<itkImageType>;

      struct Const_Violation_Exception{};
      struct Out_Of_Bounds_Exception{}; // TODO: fix this throw
      struct Overflow_Exception{};
      struct Underflow_Exception{};
      struct Region_Not_Contained_In_Mask_Exception{};
      struct Invalid_Bounds_Exception{};

      //- BINARY MASK CLASS  ----------------------------------------------------------------

      /** \brief Binary Mask class constructor.
       * \param[in] bounds, bounds of the volume.
       * \param[in] spacing, spacing of the volume.
       * \param[in] origin, origin of the volume.
       *
       *  Foreground and background will be set to default values.
       */
      explicit BinaryMask(const Bounds& bounds, const NmVector3& spacing = NmVector3{1,1,1}, const NmVector3& origin = NmVector3())
      throw(Invalid_Bounds_Exception);

      /** \brief Binary Mask class constructor.
       * \param[in] image, itkImage smart pointer.
       * \param[in] backgroundValue, value to be considered as background in the image passed as parameter.
       *
       *  Sets background value to suggested values. Foreground resorts to default value.
       */
      explicit BinaryMask(const typename itkImageType::Pointer image, const T &backgroundValue = SEG_BG_VALUE);
      virtual ~BinaryMask();

      /** \brief Returns mask bounds.
       *
       */
      VolumeBounds bounds() const
      { return m_bounds; }

      /** \brief Sets the origin of the volume.
       * \param[in] origin, new origin.
       *
       */
      void setOrigin(const NmVector3& origin);

      /** \brief Returns the origin of the volume.
       *
       */
      NmVector3 origin() const
      { return m_origin; }

      /** \brief Sets the spacing of the volume.
       * \param[in] spacing, new spacing.
       *
       */
      void setSpacing(const NmVector3& spacing);

      /** \brief Returns mask spacing.
       */
      NmVector3 spacing() const
      { return m_spacing; }

      /** \brief Returns background value of the mask.
       *   Unset bits in the image will be interpreted as having this value.
       */
      PixelType backgroundValue() const
      { return m_backgroundValue; }

      /** \brief Set mask's backgound value.
       * \param[in] backgroundValue, new background value.
       *
       *   Unset bits in the image will be interpreted as having this value.
       */
      void setBackgroundValue(const T backgroundValue)
      { m_backgroundValue = backgroundValue; }

      /** \brief Returns foreground value of the mask.
       *   Set bits in the image will be interpreted as having this value.
       */
      PixelType foregroundValue() const
      { return m_foregroundValue; }

      /** \brief Set mask's foreground value.
       * \param[in] foregroundValue, new foreground value.
       *
       *   Set bits in the image will be interpreted as having this value.
       */
      void setForegroundValue(const T foregroundValue)
      { m_foregroundValue = foregroundValue; }

      /** \brief Returns the number of voxels of the mask.
       *
       */
      unsigned long long numberOfVoxels()
      { return m_size[0] * m_size[1] * m_size[2]; }

      /** \brief Returns the size of the array used for internal storage.
       *
       */
      unsigned long long bufferSize()
      { return (m_size[0] * m_size[1] * m_size[2])/m_integerSize; }

      /** \brief Returns the number of bytes allocated by this mask.
       *
       */
      size_t memoryUsage() const
      { return m_bufferSize * sizeof(int); }

      /** \brief Returns the buffer as a QByteArray
       *
       */
      QByteArray buffer()
      { return QByteArray(reinterpret_cast<const char*>(m_image), static_cast<int>(m_size[0] * m_size[1] * m_size[2])); }

      /** \brief Set pixel value to foreground value.
       * \param[in] index, index to modify.
       *
       */
      void setPixel(const IndexType& index) throw(Out_Of_Bounds_Exception);

      /** \brief Set pixel value to background value
       * \param[in] index, index to modify.
       *
       */
      void unsetPixel(const IndexType& index) throw(Out_Of_Bounds_Exception);

      /** \brief Return the value of specified voxel in the mask.
       * \param[in] index, index to modify.
       *
       *  The returned value will be of type T.
       */
      PixelType pixel(const IndexType& index) const throw(Out_Of_Bounds_Exception);

      /** \brief Returns the itk::image<T> equivalent of a BinaryMask<T>
       *
       */
      typename itkImageType::Pointer itkImage() const;

      friend class iterator;//NOTE: No hace falta
      friend class const_iterator;
      friend class region_iterator;
      friend class const_region_iterator;

    private:
      VolumeBounds  m_bounds;
      PixelType     m_backgroundValue;
      PixelType     m_foregroundValue;
      int          *m_image;
      unsigned int  m_integerSize;
      NmVector3     m_origin;
      NmVector3     m_spacing;
      unsigned long m_size[3];
      IndexType     m_indexOrigin;
      unsigned long long m_bufferSize;

    public:
      //- ITERATOR CLASS --------------------------------------------------------------------
      class iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
        public:
          /** \brief iterator class constructor.
           * \param[in] mask, mask to iterate.
           */
          iterator(BinaryMask<T> *mask)
          : m_mask(mask)
          , m_pos(0)
          , m_bitPos(0)
          {}

          /** \brief iterator class constructor.
           * \param[in] it, iterator.
           * \param[in] pos, byte position.
           * \param[in] bitPos, bit position in byte.
           *
           * Constructor for begin() and end() iterators, and others.
           *
           */
          iterator(const iterator& it, const unsigned long long pos = 0, const int bitPos = 0)
          : m_mask(it.m_mask)
          , m_pos(pos)
          , m_bitPos(bitPos)
          {}

          /** \brief iterator class destructor.
           *
           */
          virtual ~iterator()
          {};

          /** \brief Returns an iterator positioned at the beginning of the mask.
           *
           *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
           *        is not the intended here.
           */
          iterator begin()
          { return iterator(*this); }

          /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
           *
           *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
           *        is not the intended here.
           *  NOTE: Trying to get or set a value of this iterator position will throw an
           *        exception.
           */
          iterator end()
          {
            unsigned long long pos    = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize);
            unsigned long long bitpos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize);

            return iterator(*this, pos, bitpos);
          }

          /** \brief Sets the position of the iterator to the beginning of the mask.
           *
           */
          void goToBegin()
          { m_pos = 0; m_bitPos = 0; }

          /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
           *
           */
          void goToEnd()
          {
            m_pos    = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize);
            m_bitPos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize);
          }

          /** \brief Returns true if the iterator is positioned at the end of the mask.
           *
           */
          bool isAtEnd() const
          {
            return ((m_pos    == ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize)) &&
                    (m_bitPos == ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize)));
          }

          /** \brief Equal operator between iterator and an iterator or a const_iterator of the mask.
           *
           */
          bool operator==(const iterator& other) const
          {

            return ((m_mask == other.m_mask) && (m_pos == other.m_pos) && (m_bitPos == other.m_bitPos));
          }

          /** \brief Non equal operator between iterator and an iterator or a const_iterator of the mask.
           *
           */
          bool operator!=(const iterator& other) const
          {
            return ((m_mask != other.m_mask) || (m_pos != other.m_pos) || (m_bitPos == other.m_bitPos));
          }

          /** \brief Returns the value at the position of the iterator.
           *
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the mask.
           */
          const T& Get() const throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
              throw Out_Of_Bounds_Exception();

            if ((m_mask->m_image[m_pos] & (1 << m_bitPos)) == (1 << m_bitPos))
              return m_mask->m_foregroundValue;

            return m_mask->m_backgroundValue;
          }

          /** \brief Convenience method when one doesn't need to know the foreground/background
           * values of the mask.
           *
           */
          bool isSet()
          {
            return (Get() == m_mask->m_foregroundValue);
          }

          /** \brief Sets the value of the pointed element of the mask to foreground value.
           *
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the mask.
           */
          void Set() throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
              throw Out_Of_Bounds_Exception();

            m_mask->m_image[m_pos] = m_mask->m_image[m_pos] | (1 << m_bitPos);
          }

          /** \brief Sets the value of the pointed element of the mask to background value.
           *
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the mask.
           */
          void Unset() throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
              throw Out_Of_Bounds_Exception();

            m_mask->m_image[m_pos] = m_mask->m_image[m_pos] & ~(1 << m_bitPos);
          }

          /** \brief Decrements the iterator position.
           *
           *  NOTE: Can throw an Underflow_Exception if the iterator is positioned at
           *  the beginning of the mask.
           */
          iterator& operator--() throw(Underflow_Exception)
          {
            if (m_pos == 0 && m_bitPos == 0)
              throw Underflow_Exception();

            if (m_bitPos == 0)
            {
              m_bitPos = m_mask->m_integerSize - 1;
              --m_pos;
            }
            else
              --m_bitPos;

            return *this;
          }

          /** \brief Increments the iterator position.
           *
           *  NOTE: Can throw an Overflow_Exception if the iterator is positioned at
           *  one-past-the-end element of the mask.
           */
          iterator &operator++() throw (Overflow_Exception)
          {
            if (isAtEnd())
              throw Overflow_Exception();

            if (m_bitPos == m_mask->m_integerSize-1)
            {
              m_bitPos = 0;
              ++m_pos;
            }
            else
              ++m_bitPos;

            return *this;
          }

        protected:
          BinaryMask<T>     *m_mask;
          unsigned long long m_pos;
          int                m_bitPos;
      };

      //- CONST ITERATOR CLASS --------------------------------------------------------------
      class const_iterator
      : public iterator
      {
        public:
          /** \brief const_iterator class constructor.
           * \param[in] mask, mask to iterate.
           */
          const_iterator(BinaryMask<T>* mask)
          : iterator(mask)
          {};

          /** \brief const_iterator class destructor.
           *
           */
          virtual ~const_iterator()
          {};

          /** \brief Forbidden Set() will throw a Const_Violation_Exception exception.
           *
           */
          void Set() throw(Const_Violation_Exception)
          { throw Const_Violation_Exception(); }

          /** \brief Forbidden Unset() will throw a Const_Violation_Exception exception.
           *
           */
          void Unset() throw(Const_Violation_Exception)
          { throw Const_Violation_Exception(); }
      };

      //- REGION ITERATOR CLASS -------------------------------------------------------------
      class region_iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
        public:
          /** \brief region_iterator class constructor.
           * \param[in] mask, mask to iterate.
           * \param[in] bounds, bounds of the region to iterate.
           *
           *  NOTE: Can throw a Region_Not_Contained_In_Mask if the given region is not
           *        inside the largest possible region of the mask.
           */
          region_iterator(BinaryMask<T> *mask, const Bounds &bounds)
          throw (Region_Not_Contained_In_Mask_Exception)
          : m_mask(mask)
          {
            m_bounds = VolumeBounds(bounds, mask->m_spacing, mask->m_origin);

            if (!contains(mask->bounds(), m_bounds))
              throw Region_Not_Contained_In_Mask_Exception();

            itkVolumeType::RegionType region = equivalentRegion<itkVolumeType>(mask->m_origin, mask->m_spacing, bounds);

            m_extent[0] = region.GetIndex(0);
            m_extent[1] = region.GetIndex(0) + region.GetSize(0) - 1;
            m_extent[2] = region.GetIndex(1);
            m_extent[3] = region.GetIndex(1) + region.GetSize(1) - 1;
            m_extent[4] = region.GetIndex(2);
            m_extent[5] = region.GetIndex(2) + region.GetSize(2) - 1;

            m_index.x = m_extent[0];
            m_index.y = m_extent[2];
            m_index.z = m_extent[4];
          }

          /** \brief region_iterator class constructor.
           * \param[in] it, region iterator.
           * \param[in] index, index object.
           *
           * Constructor for begin() and end() iterators, and others.
           *
           */
          region_iterator(const region_iterator &it, const IndexType &index)
          : m_mask(it.m_mask)
          , m_bounds(it.m_bounds)
          {
            memcpy(m_extent, it.m_extent, 6*sizeof(int));

            m_index.x = index.x;
            m_index.y = index.y;
            m_index.z = index.z;
          }

          /** \brief region_iterator class destructor.
           *
           */
          virtual ~region_iterator()
          {};

          /** \brief Returns the index of the current position of the iterator.
           *
           */
          const IndexType getIndex() const
          {
            return m_index;
          }

          /** \brief Returns the position of the current index in world coordinates.
           *
           */
          NmVector3 getCenter() const
          {
            return NmVector3{
              m_mask->m_origin[0] + m_index.x * m_mask->m_spacing[0],
              m_mask->m_origin[1] + m_index.y * m_mask->m_spacing[1],
              m_mask->m_origin[2] + m_index.z * m_mask->m_spacing[2]
            };
          }

          /** \brief Returns an iterator positioned at the beginning of the region of the mask.
           *
           *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
           *        is not the intended here.
           */
          region_iterator begin()
          {
            IndexType index;
            index.x = m_extent[0];
            index.y = m_extent[2];
            index.z = m_extent[4];

            return region_iterator(*this, index);
          }

          /** \brief Returns an iterator positioned at one-past-the-end element of the region of the mask.
           *
           *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
           *        is not the intended here.
           *  NOTE: Trying to get or set a value of this iterator position will throw an
           *        exception.
           */
          region_iterator end()
          {
            IndexType index;
            index.x = m_extent[1] + 1;
            index.y = m_extent[3] + 1;
            index.z = m_extent[5] + 1;

            return region_iterator(*this, index);
          }

          /** \brief Sets the position of the iterator to the beginning of the region of the mask.
           *
           */
          void goToBegin()
          {
            m_index.x = m_extent[0];
            m_index.y = m_extent[2];
            m_index.z = m_extent[4];
          }

          /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
           *
           */
          void goToEnd()
          {
            m_index.x = m_extent[1] + 1;
            m_index.y = m_extent[3] + 1;
            m_index.z = m_extent[5] + 1;
          }

          /** \brief Returns true if the region_iterator is positioned at one-past-the-end
           *  element of the region of the mask.
           *
           */
          bool isAtEnd() const
          {
            return ((m_index.x == m_extent[1] + 1) && (m_index.y == m_extent[3] + 1) && (m_index.z == m_extent[5] + 1));
          }

          /** \brief Equal operator between region_iterator and a region_iterator or a const_region_iterator of the mask.
           *
           */
          bool operator==(const region_iterator& other) const
          {
            bool retValue = true;

            retValue &= m_mask == other.m_mask;
            retValue &= m_bounds == other.m_bounds;
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(int)) == 0);
            retValue &= ((m_index.x == other.m_index.x) && (m_index.y == other.m_index.y) && (m_index.z == other.m_index.z));

            return retValue;
          }

          /** \brief Non equal operator between region_iterator and a region_iterator or a const_region_iterator of the mask.
           *
           */
          bool operator!=(const region_iterator& other) const
          {
            return !operator==(other);
          }

          /** \brief Returns the value at the position of the iterator.
           *
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the mask.
           */
          const T& Get() const throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
              throw Out_Of_Bounds_Exception();

            // NOTE: we need this to avoid returning a temporary value
            m_getReturnValue = m_mask->pixel(m_index);

            return m_getReturnValue;
          }

          /** \brief Convenience method when one doesn't need to know the foreground/background
           * values of the mask.
           *
           */
          bool isSet()
          {
            NmVector3 point = {
              m_mask->m_origin[0] + static_cast<Nm>(m_index.x)*m_mask->m_spacing[0],
              m_mask->m_origin[1] + static_cast<Nm>(m_index.y)*m_mask->m_spacing[1],
              m_mask->m_origin[2] + static_cast<Nm>(m_index.z)*m_mask->m_spacing[2]
            };

            if (!contains(m_bounds, point))
            	throw Out_Of_Bounds_Exception();

            // we must adjust the index
            IndexType newIndex;
            newIndex.x = m_index.x - m_mask->m_indexOrigin.x;
            newIndex.y = m_index.y - m_mask->m_indexOrigin.y;
            newIndex.z = m_index.z - m_mask->m_indexOrigin.z;

            unsigned long long valuePosition = newIndex.x + (newIndex.y * m_mask->m_size[0]) + (newIndex.z * m_mask->m_size[0]*m_mask->m_size[1]);
            unsigned long long imageOffset   = valuePosition / m_mask->m_integerSize;
            unsigned long long valueOffset   = valuePosition % m_mask->m_integerSize;

            Q_ASSERT(imageOffset < m_mask->m_bufferSize);
            return ((m_mask->m_image[imageOffset] & (1 << valueOffset)) == (1 << valueOffset));
          }

          /** \brief Sets the value of the pointed element of the mask to foreground value.
           *
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the region.
           *
           */
          void Set() throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
            	throw Out_Of_Bounds_Exception();

            m_mask->setPixel(m_index);
          }

          /** \brief Sets the value of the pointed element of the mask to background value.
           *
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the region.
           *
           */
          void Unset() throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
            	throw Out_Of_Bounds_Exception();

            m_mask->unsetPixel(m_index);
          }

          /** \brief Decrements the iterator position.
           *
           *  NOTE: Can throw an Underflow_Exception if the iterator is positioned at
           *  the beginning of the region.
           *
           */
          region_iterator& operator--() throw(Underflow_Exception)
          {
            if ((m_index.x == m_extent[0]) && (m_index.y == m_extent[2]) && (m_index.z == m_extent[4]))
              throw Underflow_Exception();

            if (m_index.x == m_extent[0])
            {
              m_index.x = m_extent[1];
              if (m_index.y == m_extent[2])
              {
                m_index.y = m_extent[3];
                --m_index.z;
              }
              else
                --m_index.y;
            }
            else
              --m_index.x;

            return *this;
          }

          /** \brief Increments the iterator position.
           *
           *  NOTE: Can throw an Overflow_Exception if the iterator is positioned at
           *  one-past-the-end element of the region.
           *
           */
          region_iterator &operator++() throw(Overflow_Exception)
          {
            if ((m_index.x == m_extent[1] + 1) && (m_index.y == m_extent[3] + 1) && (m_index.z == m_extent[5] + 1))
              throw Overflow_Exception();

            if ((m_index.x == m_extent[1]) && (m_index.y == m_extent[3]) && (m_index.z == m_extent[5]))
            {
              m_index.x = m_extent[1] + 1;
              m_index.y = m_extent[3] + 1;
              m_index.z = m_extent[5] + 1;
              return *this;
            }

            if (m_index.x == m_extent[1])
            {
              m_index.x = m_extent[0];
              if (m_index.y == m_extent[3])
              {
                m_index.y = m_extent[2];
                ++m_index.z;
              }
              else
                ++m_index.y;
            }
            else
              ++m_index.x;

            return *this;
          }

        protected:
          BinaryMask<T>     *m_mask;
          VolumeBounds       m_bounds;
          int                m_extent[6];
          IndexType          m_index;
          mutable PixelType  m_getReturnValue; // just to return the value of Get();
      };

      //- CONST REGION ITERATOR CLASS -------------------------------------------------------
      class const_region_iterator
      : public region_iterator
      {
        public:
          /** \brief const_region_iterator class constructor.
           * \param[in] mask, mask to iterate.
           * \param[in] bounds, bounds to iterate.
           *
           */
          const_region_iterator(BinaryMask<T> *mask, const Bounds &bounds)
          : region_iterator(mask, bounds)
          {}

          /** \brief const_region_iterator class destructor.
           *
           */
          virtual ~const_region_iterator()
          {};

          /** \brief Forbidden Set() will throw a Const_Violation_Exception exception.
           *
           */
          void Set() throw(Const_Violation_Exception)
          { throw Const_Violation_Exception(); }

          /** \brief Forbidden Unet() will throw a Const_Violation_Exception exception.
           *
           */
          void Unset() throw(Const_Violation_Exception)
          { throw Const_Violation_Exception(); }
      };

  };

  //-------------------------------------------------------------------------------------
  template<typename T> BinaryMask<T>::BinaryMask(const typename itkImageType::Pointer image, const T &backgroundValue)
  : m_backgroundValue(backgroundValue)
  , m_foregroundValue(SEG_VOXEL_VALUE)
  , m_integerSize(sizeof(int)*8)
  {
    itkRegion region = image->GetLargestPossibleRegion();
    itkIndex  index  = region.GetIndex();

    m_indexOrigin.x = index[0];
    m_indexOrigin.y = index[1];
    m_indexOrigin.z = index[2];

    for (int i = 0; i < 3; ++i)
    {
      m_origin[i]  = image->GetOrigin()[i];
      m_spacing[i] = image->GetSpacing()[i];
    }

    m_bounds = volumeBounds<itkVolumeType>(m_origin, m_spacing, region);

    itkSize size = region.GetSize();
    m_size[0] = size[0];
    m_size[1] = size[1];
    m_size[2] = size[2];

    m_bufferSize = region.GetNumberOfPixels() / m_integerSize;

    if (region.GetNumberOfPixels() % m_integerSize != 0)
      m_bufferSize++;

    m_image = new int[m_bufferSize];
    memset(m_image, 0, m_bufferSize*sizeof(int));

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
  template<typename T>
  BinaryMask<T>::BinaryMask(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin)
  throw(Invalid_Bounds_Exception)
  : m_backgroundValue(0)
  , m_foregroundValue(255)
  , m_integerSize(sizeof(int)*8)
  , m_origin(origin)
  , m_spacing(spacing)
  {
    if (!bounds.areValid())
      throw Invalid_Bounds_Exception();

    m_bounds = VolumeBounds(bounds, m_spacing, m_origin);
    itkVolumeType::RegionType region = equivalentRegion<itkVolumeType>(m_origin, m_spacing, bounds);

    Q_ASSERT(isEquivalent(m_bounds, volumeBounds<itkVolumeType>(m_origin, m_spacing, region)));
    Q_ASSERT(m_bounds == volumeBounds<itkVolumeType>(m_origin, m_spacing, region));

    m_size[0] = region.GetSize(0);
    m_size[1] = region.GetSize(1);
    m_size[2] = region.GetSize(2);

    m_indexOrigin.x = region.GetIndex(0);
    m_indexOrigin.y = region.GetIndex(1);
    m_indexOrigin.z = region.GetIndex(2);

    m_bufferSize = region.GetNumberOfPixels() / m_integerSize;

    if (region.GetNumberOfPixels() % m_integerSize != 0)
      m_bufferSize++;

    m_image = new int[m_bufferSize];

    memset(m_image, 0, m_bufferSize*sizeof(int));
  }

  //-------------------------------------------------------------------------------------
  template<typename T>
  BinaryMask<T>::~BinaryMask()
  {
    if (m_image) delete[] m_image;
  }

  //-------------------------------------------------------------------------------------
  template<typename T>
  void BinaryMask<T>::setOrigin(const NmVector3& spacing)
  {
    Q_ASSERT(false);
  }

  //-------------------------------------------------------------------------------------
  template<typename T>
  void BinaryMask<T>::setSpacing(const NmVector3& spacing)
  {
    Q_ASSERT(false);
//     if (m_spacing != spacing)
//     {
//       NmVector3 origin;//TODO: Update API to use mask with origin != (0,0,0)???
//       auto region = equivalentRegion<itkVolumeType>(origin, m_spacing, m_bounds);
//
//       m_spacing = spacing;
//
//       m_bounds = equivalentBounds<itkVolumeType>(origin, m_spacing, region);
//     }
  }

  //-------------------------------------------------------------------------------------
  template<typename T>
  void BinaryMask<T>::setPixel(const IndexType& index)
  throw(Out_Of_Bounds_Exception)
  {
    NmVector3 point = {
      m_origin[0] + static_cast<Nm>(index.x)*m_spacing[0],
      m_origin[1] + static_cast<Nm>(index.y)*m_spacing[1],
      m_origin[2] + static_cast<Nm>(index.z)*m_spacing[2]
    };

    if (!contains(m_bounds, point))
    	throw Out_Of_Bounds_Exception();

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_indexOrigin.x;
    newIndex.y = index.y - m_indexOrigin.y;
    newIndex.z = index.z - m_indexOrigin.z;

    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[0]) + (newIndex.z * m_size[0]*m_size[1]);
    unsigned long imageOffset   = valuePosition / m_integerSize;
    unsigned long valueOffset   = valuePosition % m_integerSize;

    Q_ASSERT(imageOffset < m_bufferSize);

    m_image[imageOffset] = m_image[imageOffset] | (1 << valueOffset);
  }

  //-------------------------------------------------------------------------------------
  template<typename T>
  void BinaryMask<T>::unsetPixel(const IndexType& index)
  throw(Out_Of_Bounds_Exception)
  {
    NmVector3 point = {
      m_origin[0] + static_cast<Nm>(index.x)*m_spacing[0],
      m_origin[1] + static_cast<Nm>(index.y)*m_spacing[1],
      m_origin[2] + static_cast<Nm>(index.z)*m_spacing[2]
    };

    if (!contains(m_bounds, point))
    	throw Out_Of_Bounds_Exception();

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_indexOrigin.x;
    newIndex.y = index.y - m_indexOrigin.y;
    newIndex.z = index.z - m_indexOrigin.z;

    unsigned long valuePosition = newIndex.x + (newIndex.y * m_size[0]) + (newIndex.z * m_size[0]*m_size[1]);
    unsigned long imageOffset   = valuePosition / m_integerSize;
    unsigned long valueOffset   = valuePosition % m_integerSize;

    Q_ASSERT(imageOffset < m_bufferSize);
    m_image[imageOffset] = m_image[imageOffset] & ~(1 << valueOffset);
  }

  //-------------------------------------------------------------------------------------
  template<typename T> typename
  BinaryMask<T>::PixelType BinaryMask<T>::pixel(const IndexType& index) const
  throw(Out_Of_Bounds_Exception)
  {
    NmVector3 point = {
      m_origin[0] + static_cast<Nm>(index.x)*m_spacing[0],
      m_origin[1] + static_cast<Nm>(index.y)*m_spacing[1],
      m_origin[2] + static_cast<Nm>(index.z)*m_spacing[2]
    };

    if (!contains(m_bounds, point))
    	throw Out_Of_Bounds_Exception();

    // we must adjust the index
    IndexType newIndex;
    newIndex.x = index.x - m_indexOrigin.x;
    newIndex.y = index.y - m_indexOrigin.y;
    newIndex.z = index.z - m_indexOrigin.z;

    unsigned long long valuePosition = newIndex.x + (newIndex.y * m_size[0]) + (newIndex.z * m_size[0]*m_size[1]);
    unsigned long long imageOffset   = valuePosition / m_integerSize;
    unsigned long long valueOffset   = valuePosition % m_integerSize;

    Q_ASSERT(imageOffset < m_bufferSize);
    bool isSet = ((m_image[imageOffset] & (1 << valueOffset)) == (1 << valueOffset));

    return isSet?m_foregroundValue:m_backgroundValue;
  }

  //-------------------------------------------------------------------------------------
  template<typename T>
  typename BinaryMask<T>::itkImageType::Pointer BinaryMask<T>::itkImage() const
  {
    using ImagePointer  = typename itkImageType::Pointer;
    using ImageRegion   = typename itkImageType::RegionType;
    using ImagePoint    = typename itkImageType::PointType;
    using ImageIndex    = typename itkImageType::RegionType::IndexType;
    using ImageSize     = typename itkImageType::RegionType::SizeType;
    using ImageSpacing  = typename itkImageType::SpacingType;
    using ImageIterator = typename itk::ImageRegionIteratorWithIndex<itkImageType>;
    using MaskIterator  = typename BinaryMask<T>::const_region_iterator;

    ImageIndex index;
    index[0] = m_indexOrigin.x;
    index[1] = m_indexOrigin.y;
    index[2] = m_indexOrigin.z;

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
    origin[0] = m_indexOrigin.x;
    origin[1] = m_indexOrigin.y;
    origin[2] = m_indexOrigin.z;

    ImagePointer image = itkImageType::New();
    image->SetOrigin(origin);
    image->SetRegions(region);
    image->SetSpacing(spacing);
    image->Allocate();
    image->FillBuffer(m_backgroundValue);

    ImageIterator iit(image, region);

    MaskIterator mit(const_cast<BinaryMask<T>*>(this), m_bounds.bounds());

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

  template<class T> using BinaryMaskPtr  = BinaryMask<T> *;
  template<class T> using BinaryMaskSPtr = std::shared_ptr<BinaryMask<T>>;

} // namespace ESPINA

#endif // ESPINA_BINARY_MASK_H
