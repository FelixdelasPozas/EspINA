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


#ifndef ESPINA_BINARY_MASK_H
#define ESPINA_BINARY_MASK_H

#include "Core/Utils/Bounds.h"
#include <Core/Utils/Spatial.h>

#include <itkImage.h>
#include <itkSmartPointer.h>
#include <itkImageRegionConstIterator.h>

#include <math.h>

namespace EspINA
{
  template<typename T > class BinaryMask
  {
    public:
      using PixelType        = T;
      using IndexType        = struct index { int x; int y; int z;
                                              index(): x(0), y(0), z(0) {};
                                              index(const int xv, const int yv, const int zv): x(xv), y(yv), z(zv) {};
                                              index(const index &i): x(i.x), y(i.y), z(i.z) {}; };
      using Spacing          = struct spacing { Nm x; Nm y; Nm z;
                                              spacing(): x(1), y(1), z(1) {};
                                              spacing(const Nm xv, const Nm yv, const Nm zv): x(xv), y(yv), z(zv) {};
                                              spacing(const spacing &s): x(s.x), y(s.y), z(s.z) {}; };
      using itkImageType     = itk::Image<T,3>;
      using itkPointer       = typename itkImageType::Pointer;
      using itkIndex         = typename itkImageType::IndexType;
      using itkRegion        = typename itkImageType::RegionType;
      using itkSpacing       = typename itkImageType::SpacingType;
      using itkSize          = typename itkImageType::SizeType;
      using itkConstIterator = itk::ImageRegionConstIterator<itkImageType>;

      struct Const_Violation_Exception{};
      struct Out_Of_Bounds_Exception{};
      struct Overflow_Exception{};
      struct Underflow_Exception{};
      struct Region_Not_Contained_In_Mask_Exception{};
      struct Invalid_Bounds_Exception{};

      //- BINARY MASK CLASS  ----------------------------------------------------------------

      /** \brief Binary Mask constructor
       *
       *  Foreground and background will be set to default values.
       */
      explicit BinaryMask(const Bounds& bounds, const Spacing spacing = Spacing()) throw(Invalid_Bounds_Exception);

      /** \brief Binary Mask constructor from an image and a background value. Every other
       *   value in the image will be considered as foreground.
       *
       *  Sets background value to suggested values. Foreground resorts to default value.
       */
      explicit BinaryMask(const itkPointer image, const T &backgroundValue);
      virtual ~BinaryMask() {};

      /** \brief Returns mask bounds.
       */
      Bounds bounds() const                            { return m_bounds; }

      /** \brief Returns mask spacing.
       */
      Spacing spacing() const                          { return m_spacing; }

      /** \brief Returns background value of the mask.
       *   Unset bits in the image will be interpreted as having this value.
       */
      PixelType backgroundValue() const                { return m_backgroundValue; }

      /** \brief Set mask's backgound value.
       *   Unset bits in the image will be interpreted as having this value.
       */
      void setBackgroundValue(const T backgroundValue) { m_backgroundValue = backgroundValue; }

      /** \brief Returns foreground value of the mask.
       *   Set bits in the image will be interpreted as having this value.
       */
      PixelType foregroundValue() const                { return m_foregroundValue; }

      /** \brief Set mask's foreground value.
       *   Set bits in the image will be interpreted as having this value.
       */
      void setForegroundValue(const T foregroundValue) { m_foregroundValue = foregroundValue; }

      /** \brief Returns the number of voxels of the mask.
       */
      unsigned long long numberOfVoxels()              { return m_size[0] * m_size[1] * m_size[2]; }

      /** \brief Set pixel value to foreground value
       */
      void setPixel(const IndexType& index) throw(Out_Of_Bounds_Exception);

      /** \brief Set pixel value to background value
       */
      void unsetPixel(const IndexType& index) throw(Out_Of_Bounds_Exception);

      /** \brief Return the value of specified voxel in the mask.
       *  The returned value will be of type T.
       */
      PixelType pixel(const IndexType& index) const throw(Out_Of_Bounds_Exception);

      /** \brief Returns the itk::image<T> equivalent of a BinaryMask<T>
       */
      typename itkImageType::Pointer itkImage() const;


      friend class iterator;
      friend class const_iterator;
      friend class region_iterator;
      friend class const_region_iterator;

    private:
      Bounds        m_bounds;
      PixelType     m_backgroundValue;
      PixelType     m_foregroundValue;
      int          *m_image;
      int           m_integerSize;
      Spacing       m_spacing;
      unsigned long m_size[3];
      IndexType     m_origin;

    public:
      //- ITERATOR CLASS --------------------------------------------------------------------
      class iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
        public:
          /** \brief iterator for a given mask constructor
           */
          iterator(BinaryMask<T> *mask)
          : m_mask(mask), m_pos(0), m_bitPos(0)
          {
          }

          virtual ~iterator() {};

          /** \brief Returns an iterator positioned at the beginning of the mask
           *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
           *        is not the intended here.
           */
          iterator begin()
          {
            return iterator(*this);
          }

          /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
           *  NOTE: In the STL if a container is empty it.begin() == it.end(). That
           *        is not the intended here.
           *  NOTE: Trying to get or set a value of this iterator position will throw an
           *        exception.
           */
          iterator end()
          {
            unsigned long long pos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize);
            unsigned long long bitpos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize);

            return iterator(*this, pos, bitpos);
          }

          /** \brief Sets the position of the iterator to the beginning of the mask.
           */
          void goToBegin()
          {
            m_pos = 0;
            m_bitPos = 0;
          }

          /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
           */
          void goToEnd()
          {
            m_pos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize);
            m_bitPos = ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize);
          }

          /** \brief Returns true if the iterator is positioned at the end of the mask.
           */
          bool isAtEnd() const
          {
            return ((m_pos == ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) / m_mask->m_integerSize)) &&
                    (m_bitPos == ((m_mask->m_size[0] * m_mask->m_size[1] * m_mask->m_size[2]) % m_mask->m_integerSize)));
          }

          /** \brief Equal operator between iterator and an iterator or a const_iterator of the mask.
           */
          bool operator==(const iterator& other) const
          {

            return ((m_mask == other.m_mask) && (m_pos == other.m_pos) && (m_bitPos == other.m_bitPos));
          }

          /** \brief Non equal operator between iterator and an iterator or a const_iterator of the mask.
           */
          bool operator!=(const iterator& other) const
          {
            return ((m_mask != other.m_mask) || (m_pos != other.m_pos) || (m_bitPos == other.m_bitPos));
          }

          /** \brief Returns the value at the position of the iterator.
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

          /** \brief Sets the value of the pointed element of the mask to foreground value.
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
          /** \brief Private constructor for begin() and end() iterators
           */
          iterator(const iterator& it, const unsigned long long pos = 0, const int bitPos = 0)
          : m_mask(it.m_mask)
          , m_pos(pos)
          , m_bitPos(bitPos)
          {
          }

          BinaryMask<T>     *m_mask;
          unsigned long long m_pos;
          int                m_bitPos;
      };

      //- CONST ITERATOR CLASS --------------------------------------------------------------
      class const_iterator
      : public iterator
      {
        public:
          /** \brief const_iterator for a given mask constructor
           */
          const_iterator(BinaryMask<T>* mask)
          : iterator(mask)
          {
          };

          virtual ~const_iterator() {};

          /** \brief Forbidden Set() will throw a Const_Violation_Exception exception.
           */
          void Set() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }

          /** \brief Forbidden Unset() will throw a Const_Violation_Exception exception.
           */
          void Unset() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }
      };

      //- REGION ITERATOR CLASS -------------------------------------------------------------
      class region_iterator
      : public std::iterator<std::bidirectional_iterator_tag, T>
      {
        public:
          /** \brief region_iterator for a given region of a given mask constructor.
           *  NOTE: Can throw a Region_Not_Contained_In_Mask if the given region is not
           *        inside the largest possible region of the mask.
           */
          region_iterator(BinaryMask<T> *mask, const Bounds &bounds) throw (Region_Not_Contained_In_Mask_Exception)
          : m_mask(mask), m_bounds(bounds)
          {
            if (intersection(bounds, mask->bounds()) != bounds)
              throw Region_Not_Contained_In_Mask_Exception();

            m_extent[0] = static_cast<int>(std::round(m_bounds[0]/m_mask->m_spacing.x));
            m_extent[1] = static_cast<int>(std::round(m_bounds[1]/m_mask->m_spacing.x));
            m_extent[2] = static_cast<int>(std::round(m_bounds[2]/m_mask->m_spacing.y));
            m_extent[3] = static_cast<int>(std::round(m_bounds[3]/m_mask->m_spacing.y));
            m_extent[4] = static_cast<int>(std::round(m_bounds[4]/m_mask->m_spacing.z));
            m_extent[5] = static_cast<int>(std::round(m_bounds[5]/m_mask->m_spacing.z));

            m_index.x = m_extent[0];
            m_index.y = m_extent[2];
            m_index.z = m_extent[4];
          }

          virtual ~region_iterator() {};

          /** \brief Returns the index of the current position of the iterator.
           */
          const IndexType getIndex() const
          {
            return m_index;
          }

          /** \brief Returns an iterator positioned at the beginning of the region of the mask
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
           */
          void goToBegin()
          {
            m_index.x = m_extent[0];
            m_index.y = m_extent[2];
            m_index.z = m_extent[4];
          }

          /** \brief Returns an iterator positioned at one-past-the-end element of the mask.
           */
          void goToEnd()
          {
            m_index.x = m_extent[1] + 1;
            m_index.y = m_extent[3] + 1;
            m_index.z = m_extent[5] + 1;
          }

          /** \brief Returns true if the region_iterator is positioned at one-past-the-end
           *  element of the region of the mask.
           */
          bool isAtEnd() const
          {
            return ((m_index.x == m_extent[1] + 1) && (m_index.y == m_extent[3] + 1) && (m_index.z == m_extent[5] + 1));
          }

          /** \brief Equal operator between region_iterator and a region_iterator or a const_region_iterator of the mask.
           */
          bool operator==(const region_iterator& other) const
          {
            bool retValue = true;
            retValue &= (m_mask == other.m_mask);
            retValue &= ((m_bounds[0] == other.m_bounds[0]) && (m_bounds[1] == other.m_bounds[1]) && (m_bounds[2] == other.m_bounds[2]) &&
                         (m_bounds[3] == other.m_bounds[3]) && (m_bounds[4] == other.m_bounds[4]) && (m_bounds[5] == other.m_bounds[5]));
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(unsigned long long)) == 0);
            retValue &= ((m_index.x == other.m_index.x) && (m_index.y == other.m_index.y) && (m_index.z == other.m_index.z));

            return retValue;
          }

          /** \brief Non equal operator between region_iterator and a region_iterator or a const_region_iterator of the mask.
           */
          bool operator!=(const region_iterator& other) const
          {
            bool retValue = true;
            retValue &= (m_mask != other.m_mask);
            retValue &= ((m_bounds[0] != other.m_bounds[0]) && (m_bounds[1] != other.m_bounds[1]) && (m_bounds[2] != other.m_bounds[2]) &&
                         (m_bounds[3] != other.m_bounds[3]) && (m_bounds[4] != other.m_bounds[4]) && (m_bounds[5] != other.m_bounds[5]));
            retValue &= (memcmp(m_extent, other.m_extent, 6*sizeof(unsigned long long)) != 0);
            retValue &= ((m_index.x != other.m_index.x) && (m_index.y != other.m_index.y) && (m_index.z != other.m_index.z));

            return retValue;
          }

          /** \brief Returns the value at the position of the iterator.
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

          /** \brief Sets the value of the pointed element of the mask to foreground value.
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the region.
           */
          void Set() throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
              throw Out_Of_Bounds_Exception();

            m_mask->setPixel(m_index);
          }

          /** \brief Sets the value of the pointed element of the mask to background value.
           *  NOTE: Can throw an Out_Of_Bounds_Exception if the iterator is positioned at
           *  one-past-the-end element of the region.
           */
          void Unset() throw(Out_Of_Bounds_Exception)
          {
            if (isAtEnd())
              throw Out_Of_Bounds_Exception();

            m_mask->unsetPixel(m_index);
          }

          /** \brief Decrements the iterator position.
           *  NOTE: Can throw an Underflow_Exception if the iterator is positioned at
           *  the beginning of the region.
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
           *  NOTE: Can throw an Overflow_Exception if the iterator is positioned at
           *  one-past-the-end element of the region.
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
          /** \brief Private constructor for begin() and end() iterators
           */
          region_iterator(const region_iterator &it, const IndexType &index)
          : m_mask(it.m_mask)
          , m_bounds(it.m_bounds)
          {
            memcpy(m_extent, it.m_extent, 6*sizeof(unsigned long long));

            m_index.x = index.x;
            m_index.y = index.y;
            m_index.z = index.z;
          }

          BinaryMask<T>     *m_mask;
          Bounds             m_bounds;
          unsigned long long m_extent[6];
          IndexType          m_index;
          mutable PixelType  m_getReturnValue; // just to return the value of Get();
      };

      //- CONST REGION ITERATOR CLASS -------------------------------------------------------
      class const_region_iterator
      : public region_iterator
      {
        public:
          /** \brief const_region_iterator for a given region of a given mask constructor
           */
          const_region_iterator(BinaryMask<T> *mask, const Bounds &bounds)
          : region_iterator(mask, bounds)
          {
          }

          virtual ~const_region_iterator() {};

          /** \brief Forbidden Set() will throw a Const_Violation_Exception exception.
           */
          void Set() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }

          /** \brief Forbidden Unet() will throw a Const_Violation_Exception exception.
           */
          void Unset() throw(Const_Violation_Exception) { throw Const_Violation_Exception(); }
      };

  };

} // namespace EspINA

#endif // ESPINA_BINARY_MASK_H
